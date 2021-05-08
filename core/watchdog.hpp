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
            : mTimer(arContext), mTimeout(arTimeout)
        {
        }

        void Start()
        {
            mTimer.expires_from_now(mTimeout);
            mTimer.async_wait([this](const boost::system::error_code& arError)
                {
                    if (!arError) // not cancelled
                    {
                        this->SignalTimeout();
                    }
                });
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
    };
}

#endif
