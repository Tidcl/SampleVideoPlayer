/*
 * @Author: xctang xctang@163.com
 * @Date: 2024-08-18 11:38:44
 * @LastEditors: xctang xctang@163.com
 * @LastEditTime: 2024-08-19 16:01:51
 * @FilePath: \VideoEditor\VideoPlay\PlayWidget.h
 * @Description: ??????,???`customMade`, ??koroFileHeader???? ????: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Image.H>
#include <FL/Fl_Pixmap.H>
//#include <FL/Fl_Progress.H>
#include <FL/Fl_Choice.H>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libswscale/swscale.h>
}
#include "PlayController.h"
#include "ClickProgress.h"

// 将AVFrame从YUV420P转换为Fl_RGB_Image
Fl_RGB_Image* AVFrameToFlRGBImage(AVFrame* frame, void*& dataAddr);


class PlayWidget : public Fl_Group
{
public:
    PlayWidget() : PlayWidget(0, 0, 500, 400, ""){}

    PlayWidget(int x, int y, int w, int h, char* str);

    ~PlayWidget();

    Fl_Box* imgLabel() { return m_label; };

    //组件回调
    static void btn_clicked(Fl_Widget* widget, void* v);    //定义友元函数
    void btn_pause_do();
    void btn_play_do();
    void choice_playSpeed();
    void progress_play();
private:
    Fl_Button* m_btn = nullptr; //播放按钮
    Fl_Button* m_pauseBtn = nullptr;    //暂停按钮
    Fl_Box* m_label = nullptr;  //显示label
    ClickProgress* m_progress = nullptr;  //进度条
    Fl_Choice* m_choice = nullptr;  //下拉框

    PlayController m_pc;    //播放器对象

    friend static void btn_clicked(Fl_Widget* widget, void* v); //声明友元函数
};