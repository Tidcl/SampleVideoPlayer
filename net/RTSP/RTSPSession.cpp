#include "RTSPSession.h"

std::atomic_uint RTSPSession::last_session_id_(1);

RTSPSession::RTSPSession(std::string url_suffix /*= "live"*/)
{
	suffix_ = url_suffix;
	session_id_ = ++last_session_id_;
	m_source.resize(2);
}

void RTSPSession::addSource(MediaChannelId channel_id, MediaSource* source)
{
	source->SetSendFrameCallback([this](MediaChannelId channel_id, RtpPacket pkt) {
		for (auto iter = m_pullClient.begin(); iter != m_pullClient.end();) {
			std::shared_ptr<RtpConnection> client = *iter;
			iter++;
			if (client)
			{
				client->SendRtpPacket(channel_id, pkt);
			}
		}
		//{
		//	//std::lock_guard<std::mutex> lock(map_mutex_);
		//	//for (auto iter = m_pullClient.begin(); iter != m_pullClient.end();) {
		//	//	auto conn = iter->lock();
		//	//	if (conn == nullptr) {
		//	//		m_pullClient.erase(iter++);
		//	//	}
		//	//	else {
		//	//		//int id = conn->GetId();
		//	//		//if (id >= 0) {
		//	//			if (packets.find(id) == packets.end()) {
		//	//				RtpPacket tmp_pkt;
		//	//				memcpy(tmp_pkt.data.get(), pkt.data.get(), pkt.size);
		//	//				tmp_pkt.size = pkt.size;
		//	//				tmp_pkt.last = pkt.last;
		//	//				tmp_pkt.timestamp = pkt.timestamp;
		//	//				tmp_pkt.type = pkt.type;
		//	//				packets.emplace(id, tmp_pkt);
		//	//			}
		//	//			clients.emplace_front(conn);
		//	//		//}
		//	//		iter++;
		//	//	}
		//	//}
		//}

		//int count = 0;
		//for (auto iter : clients) {
		//	int ret = 0;
		//	int id = iter->GetId();
		//	if (id >= 0) {
		//		auto iter2 = packets.find(id);
		//		if (iter2 != packets.end()) {
		//			count++;
		//			ret = iter->SendRtpPacket(channel_id, iter2->second);
		//			if (is_multicast_ && ret == 0) {
		//				break;
		//			}
		//		}
		//	}
		//}
		return true;
		});

	m_source[channel_id].reset(source);
}

void RTSPSession::addPullClient(std::shared_ptr<RtpConnection> rtpClientHandle)
{
	m_pullClient.emplace_back(rtpClientHandle);
}

void RTSPSession::rmPullClient(std::shared_ptr<RtpConnection> rtpClientHandle)
{
	auto new_end = std::remove(m_pullClient.begin(), m_pullClient.end(), rtpClientHandle);
	m_pullClient.erase(new_end, m_pullClient.end());
}

std::string RTSPSession::GetSdpMessage(std::string ip, std::string session_name /*= ""*/)
{
	uint16_t multicast_port_[MAX_MEDIA_CHANNEL];

	if (sdp_ != "") {
		return sdp_;
	}

	if (m_source.empty()) {
		return "";
	}

	char buf[2048] = { 0 };

	snprintf(buf, sizeof(buf),
		"v=0\r\n"
		"o=- 9%ld 1 IN IP4 %s\r\n"
		"t=0 0\r\n"
		"a=control:*\r\n",
		(long)std::time(NULL), ip.c_str());

	if (session_name != "") {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"s=%s\r\n",
			session_name.c_str());
	}

	if (is_multicast_) {
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
			"a=type:broadcast\r\n"
			"a=rtcp-unicast: reflection\r\n");
	}

	for (uint32_t chn = 0; chn < m_source.size(); chn++) {
		if (m_source[chn]) {
			if (is_multicast_) {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
					"%s\r\n",
					m_source[chn]->GetMediaDescription(multicast_port_[chn]).c_str());

				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
					"c=IN IP4 %s/255\r\n",
					multicast_ip_.c_str());
			}
			else {
				snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
					"%s\r\n",
					m_source[chn]->GetMediaDescription(0).c_str());
			}

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"%s\r\n",
				m_source[chn]->GetAttribute().c_str());

			snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
				"a=control:track%d\r\n", chn);
		}
	}

	sdp_ = buf;
	return sdp_;
}

bool RTSPSession::HandleFrame(MediaChannelId channel_id, RTPFrame frame)
{
	if (m_source[channel_id]) {
		m_source[channel_id]->HandleFrame(channel_id, frame);
	}
	else {
		return false;
	}

	return true;
}
