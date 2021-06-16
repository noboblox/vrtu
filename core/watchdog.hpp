#ifndef CORE_WATCHDOG_HPP_
#define CORE_WATCHDOG_HPP_

#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_context.hpp>
#include "core/signal.hpp"

namespace CORE
{
    class Watchdog
    {
    public:
        explicit Watchdog(boost::asio::io_context& arContext,
            const boost::posix_time::time_duration& arTimeout) noexcept
            : mTimer(arContext), mTimeout(arTimeout), mIsRunning(false)
        {
        }

        void Start()
        {
            mTimer.expires_from_now(mTimeout);

            mIsRunning = true;
            mTimer.async_wait([this](const boost::system::error_code& arError)
                {
                    if (!arError) // not cancelled
                    {
                        this->SignalTimeout();
                    }

                    mIsRunning = false;
                });
        }

        void StartOrContinue()
        {
            if (!IsRunning())
                Start();
        }

        bool IsRunning() const noexcept
        {
            return mIsRunning;
        }

        void Restart()
        {
            Stop();
            Start();
        }

        void Stop()
        {
            boost::system::error_code ec;
            mTimer.cancel(ec);
        }

        CORE::SignalEveryone<void> SignalTimeout;

    private:
        boost::asio::deadline_timer mTimer;
        boost::posix_time::time_duration mTimeout;
        bool mIsRunning;
    };
}

#endif
