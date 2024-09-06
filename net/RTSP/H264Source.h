#pragma once
#include <memory>
#include <chrono>
#include "MediaSource.h"

class H264Source : public MediaSource {
public:
	static H264Source* CreateNew(uint32_t framerate = 25);
	~H264Source();

	void SetFramerate(uint32_t framerate)
	{
		framerate_ = framerate;
	}

	uint32_t GetFramerate() const
	{
		return framerate_;
	}

	virtual std::string GetMediaDescription(uint16_t port);

	virtual std::string GetAttribute();

	virtual bool HandleFrame(MediaChannelId channel_id, RTPFrame frame);

	static uint32_t GetTimestamp();

private:
	H264Source(uint32_t framerate);

	uint32_t framerate_ = 25;
};