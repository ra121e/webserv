#include "Timer.hpp"

Timer::Timer() : BaseFile(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
{
}

Timer::Timer(const Timer& other) : BaseFile(other)
{
}

Timer& Timer::operator=(const Timer& other)
{
	if (this != &other)
	{
		BaseFile::operator=(other);
	}
	return *this;
}

Timer::~Timer()
{
}
