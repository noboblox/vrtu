#include "link.hpp"

#include <iomanip>
#include <iostream>

#include <boost/asio/write.hpp>

#include "core/bytestream.hpp"
#include "protocols/iec104/asdu.hpp"
#include "protocols/iec104/infoobjects.hpp"

namespace IEC104
{
    Link::Link(boost::asio::ip::tcp::socket&& arSocket, Mode mode, const ConnectionConfig& arConfig)
        : mIsMaster(mode == Mode::Master)
        , mSocket(std::move(arSocket))
        , mConfig(arConfig)
    {
    }

    Link::Link(boost::asio::ip::tcp::socket&& arSocket, Mode mode)
        : Link(std::move(arSocket), mode, ConnectionConfig())
    {
    }

    async::promise<void> Link::Run()
    {
        try
        {
            setRunning(true);
            while (!mNeedClose)
            {
                auto promiseTick = Tick();
                auto promiseDelay = Delay(std::chrono::milliseconds(30));
                async::join(promiseTick, promiseDelay);
            }
        }
        catch (...) {}

        setRunning(false);
        CloseSocket();
        co_return;
    }

    async::promise<void> Link::Tick()
    {
        co_return;
    }

    async::promise<void> Delay(std::chrono::milliseconds msec)
    {
        asio::steady_timer t(co_await asio::this_coro::executor, msec);
        co_await t.async_wait(async::use_op);
        co_return;
    }

    void Link::Cancel()
    {
        mNeedClose = true;
    }

    void Link::setRunning(bool value)
    {
        mIsRunning = value;

        if (value == false)
            setActive(value);
        else
            SignalStateChanged(*this);
    }

    void Link::setActive(bool value)
    {
        mIsActive = value;
        SignalStateChanged(*this);
    }

    void Link::CloseSocket()
    {
        if (!mSocket.is_open())
            return;

        boost::system::error_code ec;
        mSocket.close(ec);
    }

    Link::~Link()
    {
        CloseSocket();
    }
}
