#pragma once

#include <coroutine.h>
#include "Common.h"
#include "Poller.h"

class SelectPoller : public Poller {
public:
	SelectPoller() = default;
	~SelectPoller() = default;

	virtual void poll() override;

private:
	fd_set m_readSet;
	fd_set m_writeSet;
	fd_set m_errorSet;
};
