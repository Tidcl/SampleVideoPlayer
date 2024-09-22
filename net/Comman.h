#pragma once

#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <map>
#include "Socket.h"

enum FDEventType {
	none = 0,
	acceptEvent,
	readEvent,
	writeEvent,
	errorEvent
};
