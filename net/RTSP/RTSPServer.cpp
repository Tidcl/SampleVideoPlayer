#include "RTSPServer.h"

void RTSPServer::acceptHandle()
{
	SOCKET fd = m_acceptChannel->fd();
	sockaddr addr;
	int slen = sizeof(addr);
	SOCKET clientFd = accept(fd, &addr, &slen);
	if (clientFd > 0)
	{
		std::shared_ptr<RTSPClientHandle> clientHandle = std::make_shared<RTSPClientHandle>(std::dynamic_pointer_cast<RTSPServer>(shared_from_this()));
		std::shared_ptr<Channel> clientChannel = std::make_shared<Channel>();	//channel负责读写，事件检测后，channel就将字符读到缓冲
		clientHandle->setChannel(clientChannel);	//关联后续处理函数
		clientChannel->setFD(clientFd);	//管理fd
		clientChannel->setHandle(clientHandle);	//管理后续处理函数
		clientChannel->setPoller(m_poller);
	}
}

MediaSessionId RTSPServer::AddSession(RTSPSession* session)
{
	if (rtsp_suffix_map_.find(session->GetRtspUrlSuffix()) != rtsp_suffix_map_.end()) {
		return 0;
	}

	std::shared_ptr<RTSPSession> media_session(session);
	MediaSessionId sessionId = media_session->GetMediaSessionId();
	rtsp_suffix_map_.emplace(std::move(media_session->GetRtspUrlSuffix()), sessionId);
	media_sessions_.emplace(sessionId, std::move(media_session));

	return sessionId;
}

std::shared_ptr<RTSPSession> RTSPServer::LookMediaSession(const std::string& suffix)
{
	auto iter = rtsp_suffix_map_.find(suffix);
	if (iter != rtsp_suffix_map_.end()) {
		MediaSessionId id = iter->second;
		return media_sessions_[id];
	}

	return nullptr;
}

std::shared_ptr<RTSPSession> RTSPServer::LookMediaSession(MediaSessionId session_id)
{
	auto iter = media_sessions_.find(session_id);
	if (iter != media_sessions_.end()) {
		return iter->second;
	}

	return nullptr;
}

bool RTSPServer::PushFrame(MediaSessionId session_id, MediaChannelId channel_id, RTPFrame frame)
{
	std::shared_ptr<RTSPSession> sessionPtr = nullptr;

	{
		auto iter = media_sessions_.find(session_id);
		if (iter != media_sessions_.end()) {
			sessionPtr = iter->second;
		}
		else {
			return false;
		}
	}

	if (sessionPtr != nullptr) {
		return sessionPtr->HandleFrame(channel_id, frame);
	}

	return false;
}
