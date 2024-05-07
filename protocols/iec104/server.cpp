#include "protocols/iec104/server.hpp"

#include <list>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/race.hpp>
#include <boost/cobalt/op.hpp>
namespace IEC104
{
    Server::Server(const asio::ip::address& ip, uint16_t port)
        : mLocalAddr{ip, port}
        , mListener(async::this_thread::get_executor())
    {
    }

    Server::~Server()
    {
    }

    async::promise<void> Server::Tick()
    {
        try
        {
            auto promiseAccept = AcceptOne();
            auto promiseLinks = TickLinks();
            co_await async::race(promiseAccept, promiseLinks);
        }
        catch (...) {}
        co_return;
    }

    async::promise<void> Server::AcceptOne()
    {
        if (!mListener.is_open()) {
            mListener.open(mLocalAddr.protocol());
            mListener.set_option(asio::socket_base::reuse_address(true));
            mListener.bind(mLocalAddr);
            mListener.listen();
        }

        auto peer = co_await mListener.async_accept(async::use_op);

        auto link = Link(std::move(peer), Link::Mode::Slave);
        link.SignalApduReceived.Register([this](auto& l, auto& msg) { OnApduReceived(l, msg); });
        link.SignalApduSent    .Register([this](auto& l, auto& msg) { OnApduSent(l, msg);     });
        link.SignalTickFinished.Register([this](auto& l)            { OnLinkTickFinished(l);  });
        link.SignalStateChanged.Register([this](auto& l)            { OnLinkStateChanged(l);  });
        
        mLinks.push_back(std::move(link));
        co_return;
    }

    async::task<void> Server::TickLinks()
    {
        std::vector <async::promise<void>> promises;

        std::for_each(mLinks.begin(), mLinks.end(), [&promises] (Link& l) {
                promises.push_back(l.Tick());
            });

        co_await async::join(promises);
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
            if (!l.IsConnected())
                RemoveLink(l);
            throw;
        }

        if (!l.IsConnected())
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
}