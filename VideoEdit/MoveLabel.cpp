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
	// ����һ�������������ͬ��ɫ�ľ��Σ����Ǿ�λ��
	// ���ñ߿���ɫ
	fl_color(FL_RED);
	// ���Ʊ߿�
	fl_line_style(FL_SOLID, 2); // �����߿�Ϊ3
	fl_rect(x(), y(), w(), h());
	fl_line_style(0); // �����߿�
	// ���û����draw()�����������ǩ
	Fl_Box::draw();
}

int MoveLabel::handle(int event)
{
	static int offset_x = 0, offset_y = 0;
	switch (event) {
	case FL_PUSH:  // ��갴��
	{	// ������갴���¼�
		offset_x = Fl::event_x() - x();
		offset_y = Fl::event_y() - y();
	}
		return 1; // ����1��ʾ�¼��ѱ�����
	case FL_DRAG:  // ����϶�
	{	// ��������϶��¼�
		position(Fl::event_x() - offset_x, Fl::event_y() - offset_y);
		auto parentPtr = parent()->parent();
		if (parentPtr)
		{
			parentPtr->redraw();
		}
	}
		return 1;
	case FL_RELEASE:  // ����ͷ�
		// ��������ͷ��¼�
		this->do_callback();
		return Fl_Box::handle(event); // ���������¼�
	default:
		return Fl_Box::handle(event); // ���������¼�
	}
}
