#ifndef VRTU_CLOCKWRAPPER_HPP_
#define VRTU_CLOCKWRAPPER_HPP_

#include <chrono>

namespace VRTU
{
	class ClockWrapper
	{
	public:
		using ClockFunc = std::chrono::milliseconds(*)();

		static std::chrono::milliseconds UtcNow();

		static void Override(ClockFunc func);

	private:
		static ClockFunc UtcNowFunc;
	};
}

#endif