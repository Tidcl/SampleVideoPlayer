#pragma once

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

//通过鼠标点击拖动，可以修改自身位置的组件
class MoveLabel : public Fl_Box {
public:
	MoveLabel(int X, int Y, int W, int H, const char* L = 0);
	~MoveLabel();

	void draw() override;

	int handle(int event) override;
private:

};