#include "RTSPClientSession.h"
#include "SocketUtil.h"
#include "RTSPServer.h"

#include "RTSPSession.h"

RTSPClientSession::RTSPClientSession(std::shared_ptr<RTSPServer> server)
{
	m_server = server;
	m_rtspReq = std::make_shared<RtspRequest>();
	m_rtspRes = std::make_shared<RtspResponse>();;
}

RTSPClientSession::RTSPClientSession()
{
	RTSPClientSession(nullptr);
}

void RTSPClientSession::readHandle()
{
	HandleRtspRequest();
}

void RTSPClientSession::closeHandle()
{
	m_Session;
	m_server->LookMediaSession(m_rtspReq->GetRtspUrlSuffix())->rmPullClient(m_rtpClient);
}

bool RTSPClientSession::HandleRtspRequest()
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

void RTSPClientSession::HandleCmdOption()
{
	std::shared_ptr<char> res(new char[2048], std::default_delete<char[]>());
	int size = m_rtspReq->BuildOptionRes(res.get(), 2048);
	m_channel->writeByteBuffer().append(res.get(), size);
}

void RTSPClientSession::HandleCmdDescribe()
{
	//Ԥ������rtp����
	if (m_rtpClient == nullptr) {
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
	}

	//û���ҵ�rtsp����session�ͷ���notfound
	if (!rtsp || !media_session) {
		size = m_rtspReq->BuildNotFoundRes(res.get(), 4096);
	}
	else {
		//����ǰsocket��rtp���Ӽ��뵽session��
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

	m_channel->writeByteBuffer().append(res.get(), size);
	return;
}

void RTSPClientSession::HandleCmdSetup()
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

	if (m_rtspReq->GetTransportMode() == RTP_OVER_TCP) {
		uint16_t rtp_channel = m_rtspReq->GetRtpChannel();
		uint16_t rtcp_channel = m_rtspReq->GetRtcpChannel();
		uint16_t session_id = m_rtpClient->GetRtpSessionId();

		//����tcp��Ӧ����
		m_rtpClient->SetupRtpOverTcp(channel_id, rtp_channel, rtcp_channel);
		size = m_rtspReq->BuildSetupTcpRes(res.get(), 4096, rtp_channel, rtcp_channel, session_id);
	}
	else {
		goto transport_unsupport;
	}

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

void RTSPClientSession::HandleCmdPlay()
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

void RTSPClientSession::HandleCmdTeardown()
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

void RTSPClientSession::HandleCmdGetParamter()
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

bool RTSPClientSession::HandleAuthentication()
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

void RTSPClientSession::SendRtspMessage(char* data, int size)
{
	m_channel->writeByteBuffer().append(data, size);
}
