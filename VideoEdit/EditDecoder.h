#pragma once
#include "VideoPlay/VideoDecoder.h"
#include "VideoPlay/PlayController.h"

//可以用来解码出gif和视频的关键帧，并缓存指定数量的帧和指定时长的帧到队列中
class EditDecoder : public VideoDecoder{
public:
	EditDecoder();
	~EditDecoder();
protected:
	virtual void decode() override;
private:

};