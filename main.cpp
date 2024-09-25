//#include "net/testnet.hpp"

//#include "net/TcpServer.h"
//#include "HttpServer.h"
//#include "WebSocketServer.h"
#include "SelectPoller.h"
#include "RTSPServer.h"
#include "H264File.h"

#include <coroutine.h>
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Scroll.H>
//#include "testCompiler.h"


#include "VideoPlay/PlayController.h"
#include "VideoPlay/PlayWidget.h"
#include "VideoEdit/EditPanel.h"
#include "VideoEdit/MoveLabel.h"
#include "VideoEdit/FramePusher.h"
#include "VideoEdit/EditDecoder.h"
//#include "VideoEdit/PushOpencv.h"
//#include "VideoEdit/testOpencv.cpp"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
//#pragma comment(lib, "Ws2_32.lib")

#include <vector>
#include <memory>
#include <iostream>

//class A
//{
//public:
//	A() {
//		std::cout << "construct A" << std::endl;
//	};
//	~A() {
//		std::cout << "release A" << std::endl;
//	};
//};
//void test(std::vector<std::shared_ptr<A>>& aVec)
//{
//	aVec.push_back(std::make_shared<A>());
//}

//线程向session里面的rtp连接发送帧
void sendFrameThread(RTSPServer* server, MediaSessionId sessionId, H264File* file);

void testEditCodecer()
{
	EditDecoder eDecoder;
	//eDecoder.initDecode("C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4");
	eDecoder.initDecode("C:/Users/xctan/Pictures/gif5.gif");
	eDecoder.startDecode();
}

void enableEditor(int argc, char* argv[])
{
	static FramePusher pusher;
	pusher.openCodec();

	Fl_Window fw1(0, 0, 800, 600);
	EditPanel ep(0, 0, 800, 600, "editPanelInstance");
	ep.startPusher(&pusher);
	fw1.end();
	fw1.show(argc, argv);

	Fl::run();
}

void enablePlayer(int argc, char* argv[])
{
	Fl_Window fw(0, 0, 800, 600);
	PlayWidget form(0, 0, 800, 600, "");
	fw.show(argc, argv);

	Fl::run();
}

int main(int argc, char *argv[]) {
		
	timeBeginPeriod(1);		//初始化计时分辨率

	//testCameraPush();

	//SetConsoleOutputCP(65001);	//设置控制台输出的编码方式
	// 
	////初始化网络
	//WSADATA wsaData;
	//int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	//if (result != 0) {
	//	std::cerr << "WSAStartup failed: " << result << std::endl;
	//	return 1;
	//}
	//std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();	//创建事件循环对象
	////std::shared_ptr<HttpServer> ts = std::make_shared<HttpServer>(spoller);
	////ts->listen(spoller, 8890);
	// 
	////std::shared_ptr<WebSocketServer> tws = std::make_shared<WebSocketServer>(spoller);
	////tws->listen(spoller, 8891);
	// 
	//std::shared_ptr<RTSPServer> rtsps = std::make_shared<RTSPServer>(spoller);	//创建服务器对象
	//rtsps->listen(554);	//设置监听端口
	//RTSPSession* session = new RTSPSession();	
	//session->addSource(channel_0, H264Source::CreateNew());
	//MediaSessionId session_id = rtsps->AddSession(session);

	//H264File h264_file;
	//if (!h264_file.Open("../../Resource/test.h264")) {//"../../Resource/test.h264"
	//	printf("Open %s failed.\n", argv[1]);
	//	return 0;
	//}
	//std::thread t1(sendFrameThread, rtsps.get(), session_id, &h264_file);
	//t1.detach();
	//
	//bool stopThread = false;
	//创建协程
	//go [&stopThread, spoller]() {
	//	while (!stopThread)
	//	{
	//		spoller->poll();
	//	}
	//};
	////启动一个线程进行协程循环
	//std::thread t([]() {	
	//	co_sched.Start();
	//});


	Fl_Window fw(0, 0, 800, 600);
	PlayWidget form(0, 0, 800, 600, "");
	fw.show(argc, argv);


	Fl::add_handler([](int event)->int {return event == FL_SHORTCUT && Fl::event_key() == FL_Escape; });
	Fl_Window mainW(0,0,80,80);
	mainW.show(argc, argv);
	int lrtn = Fl::run();
	//stopThread = true;
	//co_sched.Stop();
	//t.join();
	return lrtn;
}



//推送rtp视频流
void sendFrameThread(RTSPServer* rtsp_server, MediaSessionId session_id, H264File* h264_file)
{
	Sleep(1000);
	printf("start push video frame...");
	int buf_size = 2000000;
	std::unique_ptr<uint8_t> frame_buf(new uint8_t[buf_size]);

	while (1) {
		bool end_of_frame = false;
		int frame_size = h264_file->ReadFrame((char*)frame_buf.get(), buf_size, &end_of_frame);
		if (frame_size > 0) {
			RTPFrame videoFrame = { 0 };
			videoFrame.type = 0;
			videoFrame.size = frame_size;
			videoFrame.timestamp = H264Source::GetTimestamp();
			videoFrame.buffer.reset(new uint8_t[videoFrame.size]);
			memcpy(videoFrame.buffer.get(), frame_buf.get(), videoFrame.size);
			rtsp_server->PushFrame(session_id, channel_0, videoFrame);
		}
		else {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(40));
	};
}