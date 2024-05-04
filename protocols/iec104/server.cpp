#include "protocols/iec104/server.hpp"
#include <list>

namespace IEC104
{
    Server::Server(const asio::ip::address& ip, uint16_t port)
        : mLocalAddr{ip, port}
    {
    }

    Server::~Server()
    {
    }

    async::promise<void> Server::Run()
    {
        asio::ip::tcp::acceptor listener = asio::ip::tcp::acceptor({ co_await async::this_coro::executor }, mLocalAddr);

        try
        {
            setRunning(true);
            std::list<async::promise<void>> promises;

            promises.push_back(AcceptOne(listener));

            while (!mNeedClose)
            {
                auto id = co_await async::race(promises);

                if (id == 0) // Accept was first to be added and should always be on front
                    promises.push_back(mLinks.back()->Run());
                else
                {
                    // can't call std::vector::erase because promises are not assignable (bug in boost::cobalt)
                    // see: https://github.com/boostorg/cobalt/issues/159
                    // so let's use a list until this is resolved
                    
                    auto it = promises.begin();
                    std::advance(it, id);
                    promises.erase(it);
                }
            }
        }
        catch (...) {}

        listener.close();
        setRunning(false);
        co_return;
    }

    async::promise<void> Server::AcceptOne(asio::ip::tcp::acceptor& listener)
    {
        auto peer = co_await listener.async_accept(async::use_op);
        mLinks.emplace_back(std::make_unique<Link>(std::move(peer), Link::Mode::Slave));
        co_return;
    }

    void Server::setRunning(bool value)
    {
        mIsRunning = value;
        SignalServerStateChanged(*this);
    }

    void Server::Cancel()
    {
        mNeedClose = true;
    }
}