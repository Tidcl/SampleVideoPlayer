/*
 * @Author: xctang xctang@163.com
 * @Date: 2024-08-18 00:02:09
 * @LastEditors: xctang xctang@163.com
 * @LastEditTime: 2024-08-19 14:47:02
 * @FilePath: \VideoEditor\main.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include "testCompiler.h"
#include "VideoPlay/PlayController.h"
#include "VideoPlay/PlayWidget.h"
#include "VideoEdit/testOpencv.cpp"

#pragma comment(lib, "winmm.lib")

int main(int argc, char *argv[]) {
  // testFFmpeg(argc, argv);

 // //testOpencv();
	timeBeginPeriod(1);
	//auto start = std::chrono::high_resolution_clock::now();

	//int count1 = 0;
	//while (1)
	//{
	//	count1++;
	//	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
	//	std::this_thread::sleep_for(std::chrono::microseconds(1));
	//	if (elapsed.count() >= 1000)
	//	{
	//		break;
	//	}
	//}

	Fl_Window fw(0, 0, 800, 600);
	PlayWidget form(0, 0, 800, 600, "");
	fw.show(argc, argv);
	Fl::add_handler([](int event)->int {return event == FL_SHORTCUT && Fl::event_key() == FL_Escape; });
	return Fl::run();

  // PlayController playC;
  // playC.setVideoUrl("C:/Users/xctan/Videos/SampleVideo_1280x720_10mb.mp4");
  // int rtn = playC.start();
  //return 0;
}
