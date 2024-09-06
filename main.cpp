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
//#include "testCompiler.h"


#include "VideoPlay/PlayController.h"
#include "VideoPlay/PlayWidget.h"
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


int main(int argc, char *argv[]) {
	//SetConsoleOutputCP(65001);
	//初始化网络
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
	std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();
	//std::shared_ptr<HttpServer> ts = std::make_shared<HttpServer>(spoller);
	//ts->listen(spoller, 8890);
	//std::shared_ptr<WebSocketServer> tws = std::make_shared<WebSocketServer>(spoller);
	//tws->listen(spoller, 8891);
	std::shared_ptr<RTSPServer> rtsps = std::make_shared<RTSPServer>(spoller);
	rtsps->listen(554);

	RTSPSession* session = new RTSPSession();
	session->addSource(channel_0, H264Source::CreateNew());
	MediaSessionId session_id = rtsps->AddSession(session);

	H264File h264_file;
	if (!h264_file.Open("../../Resource/test.h264")) {//"../../Resource/test.h264"
		printf("Open %s failed.\n", argv[1]);
		return 0;
	}
	std::thread t1(sendFrameThread, rtsps.get(), session_id, &h264_file);
	t1.detach();
	
	bool stopThread = false;

	go [&stopThread, spoller]() {
		while (!stopThread)
		{
			//Sleep(1);
			spoller->poll();
		}
	};

	std::thread t([]() {
		co_sched.Start();
	});


	//std::vector<std::shared_ptr<A>> aVec;

	//test(aVec);
	//std::thread t([&aVec]() {
	//	
	//	std::shared_ptr<A> ao = aVec[0];
	//	//std::shared_ptr<A> aO = aVec.back();
	//	//aVec.clear();
	//	//aVec.pop_back();
	//});
		
	// testFFmpeg(argc, argv);

 // //testOpencv();

	//testNet();

	timeBeginPeriod(1);

	Fl_Window fw(0, 0, 800, 600);
	PlayWidget form(0, 0, 800, 600, "");
	fw.show(argc, argv);
	Fl::add_handler([](int event)->int {return event == FL_SHORTCUT && Fl::event_key() == FL_Escape; });
	int lrtn = Fl::run();
	stopThread = true;
	co_sched.Stop();
	t.join();
	return lrtn;

  // PlayController playC;
  // playC.setVideoUrl("C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4");
  // int rtn = playC.start();
  //return 0;
}




void sendFrameThread(RTSPServer* rtsp_server, MediaSessionId session_id, H264File* h264_file)
{
	Sleep(5000);
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
		//Timer::Sleep(40);
	};
}