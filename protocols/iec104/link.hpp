#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <cstdint>

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt/promise.hpp>
#include "core/signal.hpp"

#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/connectionconfig.hpp"

namespace async = boost::cobalt;
namespace asio = boost::asio;

namespace IEC104
{
    class BaseInfoObject;

    class Link
    {
    public:
        /// Signal is invoked, when either IsActive or IsConnected changes
        CORE::SignalEveryone<void, Link&> SignalStateChanged;
        /// Signal is invoked after a connection tick has finished
        CORE::SignalEveryone<void, Link&> SignalTickFinished;
        /// Signal is invoked after an APDU was sent
        CORE::SignalEveryone<void, Link&, const Apdu&> SignalApduSent;
        /// Signal is invoked after an APDU was received
        CORE::SignalEveryone<void, Link&, const Apdu&> SignalApduReceived;

        enum class Mode
        {
            Master,
            Slave
        };

        // Create an connection with an connected socket
        explicit Link(boost::asio::ip::tcp::socket&& arSocket, Mode mode, const ConnectionConfig& arConfig);
        explicit Link(boost::asio::ip::tcp::socket&& arSocket, Mode mode);

        ~Link() noexcept;

        // Start message processing 
        async::promise<void> Run();

        // Single tick of message processing
        async::promise<void> Tick();
        // Cancel message processing
        void Cancel();

        const ConnectionConfig& Config() const noexcept { return mConfig; }

        bool IsRunning() const noexcept { return mIsRunning; }
        bool IsActive() const noexcept { return mIsActive; }
        bool IsMaster() const noexcept { return mIsMaster; }

    private:
        async::promise<void> Delay(std::chrono::milliseconds msec);
        void setRunning(bool value);
        void setActive(bool value);
        void CloseSocket();

    private:
        bool mIsMaster;
        bool mIsRunning = false;
        bool mIsActive = false;
        bool mNeedClose = false;

        boost::asio::ip::tcp::socket mSocket;
        ConnectionConfig mConfig;
    };
}

#endif