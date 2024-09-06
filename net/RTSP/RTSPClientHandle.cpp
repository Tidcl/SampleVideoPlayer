#include "RTSPClientHandle.h"
#include "SocketUtil.h"
#include "RTSPServer.h"

#include "RTSPSession.h"

RTSPClientHandle::RTSPClientHandle(std::shared_ptr<RTSPServer> server)
{
	m_server = server;
	m_rtspReq = std::make_shared<RtspRequest>();
	m_rtspRes = std::make_shared<RtspResponse>();;
}

RTSPClientHandle::RTSPClientHandle()
{
	RTSPClientHandle(nullptr);
}

void RTSPClientHandle::readHandle()
{
	HandleRtspRequest();
}

void RTSPClientHandle::closeHandle()
{
	m_Session;
	m_server->LookMediaSession(m_rtspReq->GetRtspUrlSuffix())->rmPullClient(m_rtpClient);
}

bool RTSPClientHandle::HandleRtspRequest()
{
	if (m_rtspReq->ParseRequest(m_channel->readBuffer())) {
		RtspRequest::Method method = m_rtspReq->GetMethod();
		if (method == RtspRequest::RTCP) {
			//HandleRtcp(buffer);
			return true;
		}
		else if (!m_rtspReq->GotAll()) {
			return true;
		}

		switch (method)
		{
		case RtspRequest::OPTIONS:
			HandleCmdOption();
			break;
		case RtspRequest::DESCRIBE:
			HandleCmdDescribe();
			break;
		case RtspRequest::SETUP:
			HandleCmdSetup();
			break;
		case RtspRequest::PLAY:
			HandleCmdPlay();
			break;
		case RtspRequest::TEARDOWN:
			HandleCmdTeardown();
			break;
		case RtspRequest::GET_PARAMETER:
			HandleCmdGetParamter();
			break;
		default:
			break;
		}

		if (m_rtspReq->GotAll()) {
			m_rtspReq->Reset();
		}
	}
	else {
		return false;
	}

	return true;
}

void RTSPClientHandle::HandleCmdOption()
{
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	int size = m_rtspReq->BuildOptionRes(res.get(), 2048);
	//this->SendRtspMessage(res, size);
	m_channel->writeByteBuffer().append(res.get(), size);
}

void RTSPClientHandle::HandleCmdDescribe()
{
	//if (m_authInfo != nullptr && !HandleAuthentication()) {
	//	return;
	//}

	//Ԥ������rtp����
	if (m_rtpClient == nullptr) {
		//m_rtpClient.reset(new RtpConnection(shared_from_this()));
		m_rtpClient.reset(new RtpConnection(m_channel));
	}

	//��������
	int size = 0;
	std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
	std::shared_ptr<RTSPSession> media_session = nullptr;

	//ͨ����׺��rtsp���������õ�session
	auto rtsp = m_server;
	if (rtsp) {
		media_session = rtsp->LookMediaSession(m_rtspReq->GetRtspUrlSuffix());
		//media_session = rtsp->LookMediaSession(m_rtspReq->GetRtspUrlSuffix());
		
	}

	//û���ҵ�rtsp����session�ͷ���notfound
	if (!rtsp || !media_session) {
		size = m_rtspReq->BuildNotFoundRes(res.get(), 4096);
	}
	else {
		//����ǰsocket��rtp���Ӽ��뵽session��
		//m_sessionId = media_session->GetMediaSessionId();
		//media_session->AddClient(this->GetSocket(), m_rtpClient);
		session_id_ = media_session->GetMediaSessionId();
		media_session->addPullClient(m_rtpClient);

		//����rtp
		for (int chn = 0; chn < MAX_MEDIA_CHANNEL; chn++) {
			MediaSource* source = media_session->GetMediaSource((MediaChannelId)chn);
			if (source != nullptr) {
				m_rtpClient->SetClockRate((MediaChannelId)chn, source->GetClockRate());
				m_rtpClient->SetPayloadType((MediaChannelId)chn, source->GetPayloadType());
			}
		}

		//��ȡsdp�����session�Ѿ������˾ͻ�ֱ�ӷ���
		//sdp����������������
		std::string sdp = media_session->GetSdpMessage(SocketUtil::GetSocketIp(m_channel->fd()), "");
		if (sdp == "") {
			size = m_rtspReq->BuildServerErrorRes(res.get(), 4096);
		}
		else {
			size = m_rtspReq->BuildDescribeRes(res.get(), 4096, sdp.c_str());
		}
	}

	//SendRtspMessage(res, size);
	m_channel->writeByteBuffer().append(res.get(), size);
	return;
}

void RTSPClientHandle::HandleCmdSetup()
{
	int size = 0;
	//��������Ự
	//�������صĻ���
	std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
	MediaChannelId channel_id = m_rtspReq->GetChannelId();
	std::shared_ptr<RTSPSession> media_session = nullptr;
	
	//��ȡ��session
	auto rtsp = m_server;
	if (rtsp) {
		media_session = rtsp->LookMediaSession(session_id_);
	}

	//�Ƿ���ȷ��ȡserver��session
	if (!rtsp || !media_session) {
		goto server_error;
	}

	//session�Ƿ�㲥
	//if (media_session->IsMulticast()) {
	//	std::string multicast_ip = media_session->GetMulticastIp();
	//	if (m_rtspReq->GetTransportMode() == RTP_OVER_MULTICAST) {
	//		uint16_t port = media_session->GetMulticastPort(channel_id);
	//		uint16_t session_id = m_rtpClient->GetRtpSessionId();
	//		if (!m_rtpClient->SetupRtpOverMulticast(channel_id, multicast_ip.c_str(), port)) {
	//			goto server_error;
	//		}

	//		size = m_rtspReq->BuildSetupMulticastRes(res.get(), 4096, multicast_ip.c_str(), port, session_id);
	//	}
	//	else {
	//		goto transport_unsupport;
	//	}
	//}
	//else
	//{
		//����
		//��TCP����UDP����
		if (m_rtspReq->GetTransportMode() == RTP_OVER_TCP) {
			uint16_t rtp_channel = m_rtspReq->GetRtpChannel();
			uint16_t rtcp_channel = m_rtspReq->GetRtcpChannel();
			uint16_t session_id = m_rtpClient->GetRtpSessionId();

			//����tcp��Ӧ����
			m_rtpClient->SetupRtpOverTcp(channel_id, rtp_channel, rtcp_channel);
			size = m_rtspReq->BuildSetupTcpRes(res.get(), 4096, rtp_channel, rtcp_channel, session_id);
		}
		//else if (m_rtspReq->GetTransportMode() == RTP_OVER_UDP) {
		//	uint16_t peer_rtp_port = m_rtspReq->GetRtpPort();
		//	uint16_t peer_rtcp_port = m_rtspReq->GetRtcpPort();
		//	uint16_t session_id = m_rtpClient->GetRtpSessionId();

		//	if (m_rtpClient->SetupRtpOverUdp(channel_id, peer_rtp_port, peer_rtcp_port)) {
		//		SOCKET rtcp_fd = m_rtpClient->GetRtcpSocket(channel_id);
		//		rtcp_channels_[channel_id].reset(new Channel(rtcp_fd));
		//		rtcp_channels_[channel_id]->SetReadCallback([rtcp_fd, this]() { this->HandleRtcp(rtcp_fd); });
		//		rtcp_channels_[channel_id]->EnableReading();
		//		task_scheduler_->UpdateChannel(rtcp_channels_[channel_id]);
		//	}
		//	else {
		//		goto server_error;
		//	}

		//	uint16_t serRtpPort = m_rtpClient->GetRtpPort(channel_id);
		//	uint16_t serRtcpPort = m_rtpClient->GetRtcpPort(channel_id);
		//	//����udpӦ����
		//	size = m_rtspReq->BuildSetupUdpRes(res.get(), 4096, serRtpPort, serRtcpPort, session_id);
		//}
		else {
			goto transport_unsupport;
		}
	//}

	SendRtspMessage(res.get(), size);
	return;

transport_unsupport:
	size = m_rtspReq->BuildUnsupportedRes(res.get(), 4096);
	SendRtspMessage(res.get(), size);
	return;

server_error:
	size = m_rtspReq->BuildServerErrorRes(res.get(), 4096);
	SendRtspMessage(res.get(), size);
	return;
}

void RTSPClientHandle::HandleCmdPlay()
{
	//��֤
	if (m_authInfo != nullptr) {
		if (!HandleAuthentication()) {
			return;
		}
	}

	//�Ƿ���rtp����
	if (m_rtpClient == nullptr) {
		return;
	}

	//�޸�״̬���޸�rtp���ӵĲ���
	conn_state_ = START_PLAY;
	m_rtpClient->Play();

	uint16_t session_id = m_rtpClient->GetRtpSessionId();
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());

	//����playӦ����
	int size = m_rtspReq->BuildPlayRes(res.get(), 2048, nullptr, session_id);
	SendRtspMessage(res.get(), size);
}

void RTSPClientHandle::HandleCmdTeardown()
{
	if (m_rtpClient == nullptr) {
		return;
	}

	//�޸�rtp���ӵĲ���
	m_rtpClient->Teardown();

	uint16_t session_id = m_rtpClient->GetRtpSessionId();
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	//����teardownӦ���ģ���ֹ��
	int size = m_rtspReq->BuildTeardownRes(res.get(), 2048, session_id);
	SendRtspMessage(res.get(), size);
}

void RTSPClientHandle::HandleCmdGetParamter()
{
	if (m_rtpClient == nullptr) {
		return;
	}

	uint16_t session_id = m_rtpClient->GetRtpSessionId();
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	//����Ӧ����
	int size = m_rtspReq->BuildGetParamterRes(res.get(), 2048, session_id);
	SendRtspMessage(res.get(), size);
}

bool RTSPClientHandle::HandleAuthentication()
{
	if (m_authInfo != nullptr && !m_hasAuth) {
		std::string cmd = m_rtspReq->MethodToString[m_rtspReq->GetMethod()];
		std::string url = m_rtspReq->GetRtspUrl();

		if (m_nonce.size() > 0 && (m_authInfo->GetResponse(m_nonce, cmd, url) == m_rtspReq->GetAuthResponse())) {
			m_nonce.clear();
			m_hasAuth = true;
		}
		else {
			std::shared_ptr<char> res(new char[4096], std::default_delete<char[]>());
			m_nonce = m_authInfo->GetNonce();
			int size = m_rtspReq->BuildUnauthorizedRes(res.get(), 4096, m_authInfo->GetRealm().c_str(), m_nonce.c_str());
			SendRtspMessage(res.get(), size);
			return false;
		}
	}

	return true;
}

void RTSPClientHandle::SendRtspMessage(char* data, int size)
{
	m_channel->writeByteBuffer().append(data, size);
}
