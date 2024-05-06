#include "protocols/iec104/server.hpp"

#include <list>
#include <boost/cobalt/race.hpp>
#include <boost/cobalt/op.hpp>

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

                if (id == 0) {
                    // Accept is finished... so we queue a new one.
                    promises.pop_front();
                    promises.push_front(AcceptOne(listener)); 
                    
                    // And also the link runner
                    promises.push_back(mLinks.back().Run());
                }
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

        auto link = Link(std::move(peer), Link::Mode::Slave);
        link.SignalApduReceived.Register([this](auto& l, auto& msg) { OnApduReceived(l, msg); });
        link.SignalApduSent    .Register([this](auto& l, auto& msg) { OnApduSent(l, msg);     });
        link.SignalTickFinished.Register([this](auto& l)            { OnLinkTickFinished(l);  });
        link.SignalStateChanged.Register([this](auto& l)            { OnLinkStateChanged(l);  });
        
        mLinks.push_back(std::move(link));
        co_return;
    }

    void Server::OnApduSent(Link& l, const Apdu& msg) const
    {
        SignalApduSent(l, msg);
    }

    void Server::OnApduReceived(Link& l, const Apdu& msg) const
    {
        SignalApduReceived(l, msg);
    }

    void Server::OnLinkTickFinished(Link& l) const
    {
        SignalLinkTickFinished(l);
    }

    void Server::OnLinkStateChanged(Link& l)
    {
        try
        {
            SignalLinkStateChanged(l);
        }
        catch (...)
        {
            if (!l.IsRunning())
                RemoveLink(l);
            throw;
        }

        if (!l.IsRunning())
            RemoveLink(l);
    }

    void Server::RemoveLink(const Link& l)
    {
        auto it = std::find_if(mLinks.begin(), mLinks.end(), [&l](auto& stored) {
            return &stored == &l; 
        });

        if (it != mLinks.end())
            mLinks.erase(it);
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