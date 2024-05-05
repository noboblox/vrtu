#ifndef IEC104_SERVER_HPP_
#define IEC104_SERVER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt/promise.hpp>

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
        // Signal is invoked when the server starts or stops 
        CORE::SignalEveryone<void, Server&> SignalServerStateChanged;

        explicit Server(const asio::ip::address& ip, uint16_t port = 2404);
        ~Server();

        Server(const Server&)            = delete;
        Server& operator=(const Server&) = delete;

        Server(Server&&)                 = default;
        Server& operator=(Server&&)      = default;

        async::promise<void> Run();
        void Cancel();

        asio::ip::address LocalIp() const noexcept { return mLocalAddr.address(); }
        int LocalPort() const noexcept { return mLocalAddr.port(); }

    private:
        void setRunning(bool value);
        async::promise<void> AcceptOne(asio::ip::tcp::acceptor& listener);
        void RemoveLink(const Link& l);

        void OnApduSent(Link& l, const Apdu& msg) const;
        void OnApduReceived(Link& l, const Apdu& msg) const;
        void OnLinkTickFinished(Link& l) const;
        void OnLinkStateChanged(Link& l);

    private:
        bool mIsRunning = false;
        bool mNeedClose = false;
        asio::ip::tcp::endpoint mLocalAddr;
        std::vector<Link> mLinks;
    };
}
#endif

