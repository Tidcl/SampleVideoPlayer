#include "ClickProgress.h"

int ClickProgress::handle(int event)
{
	switch (event) {
	case FL_PUSH:
		if (Fl::event_button() == FL_LEFT_MOUSE) {
			int x = Fl::event_x();
			int y = Fl::event_y();
			double ratio = (double)(x - this->x()) / this->w(); // 计算点击位置在进度条中的比例
			double new_value = this->minimum() + ratio * (this->maximum() - this->minimum()); // 计算新的value值
			this->value(new_value); // 设置进度条的新值
			if (m_callFunc)m_callFunc(this, m_value);
			return 1; // 表示事件被处理
		}
		break;
	}
	return Fl_Progress::handle(event);
}