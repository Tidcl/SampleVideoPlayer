#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include "media.h"
#include "Handle.h"
#include "TcpServer.h"
#include "RTSPClientHandle.h"
#include "RTSPSession.h"


class RTSPServer : public TcpServer {
public:
	RTSPServer(std::shared_ptr<Poller> poller) : TcpServer(poller) {};
	~RTSPServer() {};

	virtual void acceptHandle() override;

	MediaSessionId AddSession(RTSPSession* session);
	std::shared_ptr<RTSPSession> LookMediaSession(const std::string& suffix);
	std::shared_ptr<RTSPSession> LookMediaSession(MediaSessionId session_id);
	bool PushFrame(MediaSessionId sessionId, MediaChannelId channelId, RTPFrame frame);
private:
	std::unordered_map<MediaSessionId, std::shared_ptr<RTSPSession>> media_sessions_;
	std::unordered_map<std::string, MediaSessionId> rtsp_suffix_map_;
};
