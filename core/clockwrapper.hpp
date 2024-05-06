#ifndef VRTU_CLOCKWRAPPER_HPP_
#define VRTU_CLOCKWRAPPER_HPP_

#include <chrono>
#include <functional>

namespace VRTU
{
	class ClockWrapper
	{
	public:
		using ClockFunc = std::function<std::chrono::milliseconds()>;

		static std::chrono::milliseconds UtcNow();

		static void Override(ClockFunc func);
		static void Restore();

	private:
		static ClockFunc UtcNowFunc;
	};
}

#endif