#include "Common.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Table.H>
#include <FL/Fl_Scroll.H>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <coroutine.h>


#include "SelectPoller.h"
#include "RTSPServer.h"
#include "H264File.h"
#include "VideoPlay/PlayController.h"
#include "VideoPlay/PlayWidget.h"
#include "VideoEdit/EditPanel.h"
#include "VideoEdit/MoveLabel.h"
#include "VideoEdit/FramePusher.h"
#include "VideoEdit/EditDecoder.h"
#include "VideoPlay/VideoTimer.h"
#include "net/WebSocket/WebSocketServer.h"
#include "net/Http/HttpServer.h"



class A
{
public:
	A() {};
	~A() {};

	A(A& left)
	{
		std::cout << "construct" << std::endl;
	};
	A(A&& left)
	{
		std::cout << "move construct" << std::endl;
	};
	A& operator=(const A& left) {
		std::cout << "= construct" << std::endl;
		return *this;
	};
	A& operator=(const A&& left) {
		std::cout << "= move construct" << std::endl;
		return *this;
	};
};

class UseA {
public:
	UseA() {};
	~UseA() {};

	void setA(std::shared_ptr<A> a)
	{
		m_a = a;
	};

	void setA(A a)
	{
		m_aa = a;
	};

	A m_aa;
	std::shared_ptr<A> m_a;
};


void createLogFile()
{
	try
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto now = std::chrono::system_clock::now();
		auto now_c = std::chrono::system_clock::to_time_t(now);
		auto now_tm = *std::localtime(&now_c);
		std::stringstream ss;
		ss << std::put_time(&now_tm, "%Y%m%d%H%M%S");
		char bufferStr[64] = {0};
		sprintf_s(bufferStr, sizeof(bufferStr), "./logs/%s.txt", ss.str().c_str());
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(bufferStr, true);
		auto logger = std::make_shared<spdlog::logger>("log", spdlog::sinks_init_list{ file_sink, console_sink });
		//logger->set_level(spdlog::level::err);
		logger->set_pattern("[%n] [%Y-%m-%d %H:%M:%S.%e] [%l] [%t] [%s:%#] %v");
		spdlog::flush_every(std::chrono::seconds(5));
		spdlog::register_logger(logger);
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log init failed: " << ex.what() << std::endl;
	}
}

// 处理 Ctrl+C 信号的函数
BOOL WINAPI ConsoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		// 可以在这里执行你想要的任何操作
		// 但是，我们不执行退出操作
		return TRUE; // 返回 TRUE 表示成功处理了信号
	}
	return FALSE; // 返回 FALSE 表示未处理信号
}

//线程向session里面的rtp连接发送帧
void sendFrameThread(RTSPServer* server, MediaSessionId sessionId, H264File* file);


char* introduce = R"(
1.video player
2.video pusher
3.http webserver
4.websocket echo webserver
5.rtmp push client

input q to quit:
)";

int main(int argc, char *argv[]) {

	timeBeginPeriod(1);		//初始化计时分辨率

	//初始化网络
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		spdlog::get("log")->warn("WSAStartup failed: {}", result);
		return 1;
	}

	createLogFile();//创建日志文件

	// 注册控制句柄
	if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
		std::cerr << "Error registering console control handler." << std::endl;
		return 1;
	}

	//编写引导打开
	printf("please input number:%s", introduce);
	int number = -1;
	do{
		std::cin >> number;
	}while (number == -1);

	if (number == 1)
	{
		spdlog::get("log")->info("open video player");
		Fl::add_handler([](int event)->int {return event == FL_SHORTCUT && Fl::event_key() == FL_Escape; });
		Fl_Window fw(0, 0, 800, 600);
		PlayWidget form(0, 0, 800, 600, "");
		fw.show(argc, argv);
		return Fl::run();
	}
	else if (number == 2)
	{
		spdlog::get("log")->info("open video pusher");
		Fl::add_handler([](int event)->int {return event == FL_SHORTCUT && Fl::event_key() == FL_Escape; });
		Fl_Window fw1(0, 0, 1280, 720);
		EditPanel ep(0, 0, 1280, 720);
		fw1.end();
		fw1.show(argc, argv);
		return Fl::run();
	}
	else if (number == 3)
	{
		spdlog::get("log")->info("open http webserver port 8890");
		std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();	//创建事件循环对象
		std::shared_ptr<HttpServer> ts = std::make_shared<HttpServer>(spoller);
		ts->listen(8890);

		bool stopThread = false;
		//创建协程
		go[&stopThread, spoller]() {
			printf("server starting...\n");
			while (!stopThread)
			{
				spoller->poll();
			}
		};
		//启动一个线程进行协程循环
		std::thread t([]() {
			co_sched.Start();
		});


		char buffer[32];
		while (true)
		{
			memset(buffer, 0, 32);
			std::cout << "input q to quit:" << std::endl;
			std::cin >> buffer;
			if (strcmp(buffer, "q") == 0)
			{
				break;
			}
		}


		stopThread = true;
		co_sched.Stop();
		t.join();
	}
	else if (number == 4)
	{
		spdlog::get("log")->info("open websocket echo webserver port 8891");
		std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();	//创建事件循环对象
		std::shared_ptr<WebSocketServer> tws = std::make_shared<WebSocketServer>(spoller);
		tws->listen(8891);

		bool stopThread = false;
		//创建协程
		go[&stopThread, spoller]() {
			printf("server starting...\n");
			while (!stopThread)
			{
				spoller->poll();
			}
		};
		//启动一个线程进行协程循环
		std::thread t([]() {
			co_sched.Start();
		});


		char buffer[32];
		while (true)
		{
			memset(buffer, 0, 32);
			std::cout << "input q to quit:" << std::endl;
			std::cin >> buffer;
			if (strcmp(buffer, "q") == 0)
			{
				break;
			}
		}


		stopThread = true;
		co_sched.Stop();
		t.join();
	}
	else if (number == 5)
	{
		spdlog::get("log")->info("open rtmp push client port 554");
		std::shared_ptr<SelectPoller> spoller = std::make_shared<SelectPoller>();	//创建事件循环对象
		std::shared_ptr<RTSPServer> rtsps = std::make_shared<RTSPServer>(spoller);	//创建服务器对象
		rtsps->listen(554);	//设置监听端口
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
		//创建协程
		go[&stopThread, spoller]() {
			printf("rtsp://127.0.0.1:554/live\n");
			printf("server starting...\n");
			while (!stopThread)
			{
				spoller->poll();
			}
		};
		//启动一个线程进行协程循环
		std::thread t([]() {
			co_sched.Start();
		});


		char buffer[32];
		while (true)
		{
			memset(buffer, 0, 32);
			std::cout << "input q to quit:" << std::endl;
			std::cin >> buffer;
			if (strcmp(buffer, "q") == 0)
			{
				break;
			}
		}


		stopThread = true;
		co_sched.Stop();
		t.join();
	}
	return 0;
}



//推送rtp视频流
void sendFrameThread(RTSPServer* rtsp_server, MediaSessionId session_id, H264File* h264_file)
{
	Sleep(1000);
	spdlog::get("log")->info("start push video frame...");
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