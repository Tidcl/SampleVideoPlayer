#pragma once
#include "VideoPlay/VideoDecoder.h"
#include "VideoPlay/PlayController.h"

//�������������gif����Ƶ�Ĺؼ�֡��������ָ��������֡��ָ��ʱ����֡��������
class EditDecoder : public VideoDecoder{
public:
	EditDecoder();
	~EditDecoder();
protected:
	virtual void decode() override;
private:

};