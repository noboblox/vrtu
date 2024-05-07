#ifndef IEC104_SERVER_HPP_
#define IEC104_SERVER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt/promise.hpp>
#include <boost/cobalt/task.hpp>

#include <cstdint>
#include <memory>
#include <vector>

#include "protocols/iec104/link.hpp"

namespace asio = boost::asio;
namespace async = boost::cobalt;

namespace IEC104
{
    class Apdu;

    class Server
    {
    public:
        // Forwarded from child links
        CORE::SignalEveryone<void, Link&> SignalLinkStateChanged;
        // Forwarded from child links
        CORE::SignalEveryone<void, Link&> SignalLinkTickFinished;
        // Forwarded from child links
        CORE::SignalEveryone<void, Link&, const Apdu&> SignalApduSent;
        // Forwarded from child links
        CORE::SignalEveryone<void, Link&, const Apdu&> SignalApduReceived;

        explicit Server(const asio::ip::address& ip, uint16_t port = 2404);
        ~Server();

        Server(const Server&)            = delete;
        Server& operator=(const Server&) = delete;

        Server(Server&&)                 = default;
        Server& operator=(Server&&)      = default;

        async::promise<void> Tick();

        asio::ip::address LocalIp() const noexcept { return mLocalAddr.address(); }
        int LocalPort() const noexcept { return mLocalAddr.port(); }

    private:
        async::promise<void> AcceptOne();
        async::task<void> TickLinks();
        void RemoveLink(const Link& l);

        void OnApduSent(Link& l, const Apdu& msg) const;
        void OnApduReceived(Link& l, const Apdu& msg) const;
        void OnLinkTickFinished(Link& l) const;
        void OnLinkStateChanged(Link& l);

    private:
        asio::ip::tcp::acceptor mListener;
        asio::ip::tcp::endpoint mLocalAddr;
        std::vector<Link> mLinks;
    };
}
#endif

