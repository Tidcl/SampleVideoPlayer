#pragma once

#include <memory>
#include <vector>
#include <forward_list>
#include <mutex>
#include <string>
#include "Channel.h"
#include "H264Source.h"
#include "RtpConnection.h"

class RTSPSession : public std::enable_shared_from_this<RTSPSession> {
public:
	RTSPSession(std::string url_suffix = "live");
	~RTSPSession() {};

	void addSource(MediaChannelId channel_id, MediaSource* source);

	void addPullClient(std::shared_ptr<RtpConnection> rtpClientHandle);
	void rmPullClient(std::shared_ptr<RtpConnection> rtpClientHandle);

	MediaSource* GetMediaSource(MediaChannelId ch) {
		return m_source[ch].get();
	};

	std::string GetSdpMessage(std::string ip, std::string session_name = "");

	std::string GetRtspUrlSuffix() const
	{
		return suffix_;
	}

	MediaSessionId GetMediaSessionId()
	{
		return session_id_;
	}

	bool HandleFrame(MediaChannelId channel_id, RTPFrame frame);
private:
	std::mutex mutex_;
	std::mutex map_mutex_;
	std::vector<std::unique_ptr<MediaSource>> m_source;
	std::vector<std::shared_ptr<RtpConnection>> m_pullClient;

	std::string sdp_;
	bool is_multicast_ = false;
	std::string multicast_ip_;

	std::string suffix_;
	static std::atomic_uint last_session_id_;

	MediaSessionId session_id_ = 0;
};