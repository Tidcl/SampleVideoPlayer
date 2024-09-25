#include <iostream>
#include <FL/Enumerations.H>
#include <FL/fl_draw.H>
#include "MoveLabel.h"

MoveLabel::MoveLabel(int X, int Y, int W, int H, const char* L /*= 0*/) : 
	Fl_Box(X, Y, W, H, L)
{
}

MoveLabel::~MoveLabel()
{
}

void MoveLabel::draw()
{
	// 绘制一个与组件背景相同颜色的矩形，覆盖旧位置
	// 设置边框颜色
	fl_color(FL_RED);
	// 绘制边框
	fl_line_style(FL_SOLID, 2); // 设置线宽为3
	fl_rect(x(), y(), w(), h());
	fl_line_style(0); // 重置线宽
	// 调用基类的draw()方法来处理标签
	Fl_Box::draw();
}

int MoveLabel::handle(int event)
{
	static int offset_x = 0, offset_y = 0;
	switch (event) {
	case FL_PUSH:  // 鼠标按下
	{	// 处理鼠标按下事件
		offset_x = Fl::event_x() - x();
		offset_y = Fl::event_y() - y();
	}
		return 1; // 返回1表示事件已被处理
	case FL_DRAG:  // 鼠标拖动
	{	// 处理鼠标拖动事件
		position(Fl::event_x() - offset_x, Fl::event_y() - offset_y);
		auto parentPtr = parent()->parent();
		if (parentPtr)
		{
			parentPtr->redraw();
		}
	}
		return 1;
	case FL_RELEASE:  // 鼠标释放
		// 处理鼠标释放事件
		this->do_callback();
		return Fl_Box::handle(event); // 处理其他事件
	default:
		return Fl_Box::handle(event); // 处理其他事件
	}
}
