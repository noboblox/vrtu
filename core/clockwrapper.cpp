#include "core/clockwrapper.hpp"

static std::chrono::milliseconds SystemClockUtcNow() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

namespace VRTU
{
	ClockWrapper::ClockFunc ClockWrapper::UtcNowFunc = &SystemClockUtcNow;

	std::chrono::milliseconds ClockWrapper::UtcNow()
	{
		return UtcNowFunc();
	}

	void ClockWrapper::Override(ClockWrapper::ClockFunc func)
	{
		UtcNowFunc = func;
	}

	void ClockWrapper::Restore()
	{
		UtcNowFunc = &SystemClockUtcNow;
	}

}