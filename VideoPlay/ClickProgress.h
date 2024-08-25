#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Progress.H>
#include <FL/fl_draw.H>
#include <iostream>

typedef void(progressCallback)(Fl_Widget*, void*);

class ClickProgress : public Fl_Progress {
public:
	ClickProgress(int X, int Y, int W, int H, const char* L = 0)
		: Fl_Progress(X, Y, W, H, L) {}
	~ClickProgress() = default;

	int handle(int event) override;

	void setCallBackFunc(progressCallback func, void* v) {
		m_callFunc = func; 
		m_value = v;
	};
private:
	progressCallback* m_callFunc = nullptr;
	void* m_value = nullptr;
};