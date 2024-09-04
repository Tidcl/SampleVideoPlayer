//#include "net/testnet.hpp"

//#include "net/TcpServer.h"
#include "HttpServer.h"
#include "WebSocketServer.h"
#include "SelectPoller.h"

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

int main(int argc, char *argv[]) {

	//初始化网络
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed: " << result << std::endl;
		return 1;
	}
	std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();
	std::shared_ptr<HttpServer> ts = std::make_shared<HttpServer>(spoller);
	ts->listen(spoller, 8890);
	std::shared_ptr<WebSocketServer> tws = std::make_shared<WebSocketServer>(spoller);
	tws->listen(spoller, 8891);

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
	t.join();
	return lrtn;

  // PlayController playC;
  // playC.setVideoUrl("C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4");
  // int rtn = playC.start();
  //return 0;
}
