#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <cstdint>
#include <chrono>

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt/promise.hpp>
#include "core/signal.hpp"

#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/connectionconfig.hpp"
#include "protocols/iec104/sequence.hpp"

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

        Link(const Link&)            = delete;
        Link& operator=(const Link&) = delete;

        Link(Link&&)                 = default;
        Link& operator=(Link&&)      = default;

        // Start message processing 
        async::promise<void> Run();

        // Single tick of message processing
        async::promise<void> Tick();
        // Cancel message processing
        void Cancel();

        async::promise<void> Start();

        async::promise<void> Stop();

        async::promise<void> Test();

        const ConnectionConfig& Config() const noexcept { return mConfig; }

        bool IsRunning() const noexcept { return mIsRunning; }
        bool IsActive() const noexcept { return mIsActive; }
        bool IsMaster() const noexcept { return mIsMaster; }
        
        asio::ip::address LocalIp() const noexcept { return mSocket.local_endpoint().address(); }
        int LocalPort() const noexcept { return mSocket.local_endpoint().port(); }
        asio::ip::address RemoteIp() const noexcept { return mSocket.remote_endpoint().address(); }
        int RemotePort() const noexcept { return mSocket.remote_endpoint().port(); }

        std::chrono::milliseconds TimerT1() const noexcept { return UtcNow() - mPeerAckPendingSince; }
        std::chrono::milliseconds TimerT2() const noexcept { return UtcNow() - mMyAckPendingSince; }
        std::chrono::milliseconds TimerT3() const noexcept { return UtcNow() - mNoTrafficSince; }

        int CurrentW() const noexcept { return seqMyLastAck.Distance(seqRecv); }
        int CurrentK() const noexcept { return seqPeerLastAck.Distance(seqSend); }
        Sequence CurrentSendSeq() const noexcept { return seqSend; }
        Sequence CurrentRecvSeq() const noexcept { return seqRecv; }

    private:
        async::promise<void> Delay(std::chrono::milliseconds msec);
        async::promise<void> ActivateService(Apdu service);
        void setRunning(bool value);
        void setActive(bool value);
        void CloseSocket();

        inline std::chrono::milliseconds UtcNow() const noexcept {
            return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
        }

    private:
        bool mIsMaster;
        bool mIsRunning = false;
        bool mIsActive = false;
        bool mNeedClose = false;

        std::chrono::milliseconds mPeerAckPendingSince;
        std::chrono::milliseconds mMyAckPendingSince;
        std::chrono::milliseconds mNoTrafficSince;

        Sequence seqRecv;
        Sequence seqSend;
        Sequence seqMyLastAck;
        Sequence seqPeerLastAck;

        boost::asio::ip::tcp::socket mSocket;
        ConnectionConfig mConfig;
    };
}

#endif