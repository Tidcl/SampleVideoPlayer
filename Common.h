#pragma once
//���빫��ͷ�ļ������幫������

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <Socket.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <csignal>

#include <vector>
#include <memory>
#include <iostream>

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

enum FDEventType {
	none = 0,
	acceptEvent,
	readEvent,
	writeEvent,
	errorEvent
};