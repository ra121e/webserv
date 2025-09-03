#include "../include/Timer.hpp"
#include <stdexcept>

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

void	Timer::setTimer(time_t duration) const
{
	struct itimerspec new_value = {};
	new_value.it_value.tv_sec = duration > 0 ? duration : 0;
	new_value.it_value.tv_nsec = static_cast<__syscall_slong_t>(duration < 0);

	if (timerfd_settime(getFd(), 0, &new_value, NULL) == -1)
	{
		throw std::runtime_error("Error setting timer: " + std::string(strerror(errno)));
	}
}
