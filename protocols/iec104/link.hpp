#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <cstdint>
#include <chrono>

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt/promise.hpp>
#include "core/clockwrapper.hpp"
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

        // Single tick of message processing
        async::promise<void> Tick();

        async::promise<void> Start();

        async::promise<void> Stop();

        async::promise<void> Test();

        const ConnectionConfig& Config() const noexcept { return mConfig; }

        bool IsActive() const noexcept { return mIsActive; }
        bool IsMaster() const noexcept { return mIsMaster; }
        bool IsConnected() const noexcept { return mIsConnected; }
        bool ServicePending() const noexcept;
        
        asio::ip::address LocalIp() const noexcept { return mSocket.local_endpoint().address(); }
        int LocalPort() const noexcept { return mSocket.local_endpoint().port(); }
        asio::ip::address RemoteIp() const noexcept { return mSocket.remote_endpoint().address(); }
        int RemotePort() const noexcept { return mSocket.remote_endpoint().port(); }

        std::chrono::milliseconds TimerT1() const noexcept { return VRTU::ClockWrapper::UtcNow() - mPeerAckPendingSince; }
        std::chrono::milliseconds TimerT2() const noexcept { return VRTU::ClockWrapper::UtcNow() - mMyAckPendingSince; }
        std::chrono::milliseconds TimerT3() const noexcept { return VRTU::ClockWrapper::UtcNow() - mNoTrafficSince; }

        int CurrentW() const noexcept { return seqMyLastAck.Distance(seqRecv); }
        int CurrentK() const noexcept { return seqPeerLastAck.Distance(seqSend); }
        Sequence CurrentSendSeq() const noexcept { return seqSend; }
        Sequence CurrentRecvSeq() const noexcept { return seqRecv; }

    private:
        async::promise<void> Delay(std::chrono::milliseconds msec);
        async::promise<void> ActivateService(const Apdu& service);
        async::promise<void> Send(const Apdu& adpu);
        async::promise<void> SendAck();
        async::promise<void> HandleReceive();
        async::promise<void> HandleTimers();
        async::promise<void> HandleApdu(const Apdu& apdu);
        async::promise<void> HandleApduServiceAct(const Apdu& apdu);
        async::promise<void> HandlePeerSendSequence(const Apdu& apdu);
        void HandleApduServiceCon(const Apdu& apdu);
        void HandlePeerRecvSequence(const Apdu& apdu);
        
        async::promise<void> ActivateLink();
        async::promise<void> DeactivateLink(); 
        void PeerActivated();
        void PeerDeactivated();


        void setConnected(bool value) noexcept;
        void setActive(bool value);
        void InvokeStateChanged() noexcept;

        void CloseSocket() noexcept;

    private:
        bool mIsMaster    = false;
        bool mIsActive    = false;
        bool mIsConnected = true;
        ServiceType mPending = ServiceType::NONE;

        std::chrono::milliseconds mPeerAckPendingSince = VRTU::ClockWrapper::UtcNow();
        std::chrono::milliseconds mMyAckPendingSince   = VRTU::ClockWrapper::UtcNow();
        std::chrono::milliseconds mNoTrafficSince      = VRTU::ClockWrapper::UtcNow();

        Sequence seqRecv;
        Sequence seqSend;
        Sequence seqMyLastAck;
        Sequence seqPeerLastAck;

        boost::asio::ip::tcp::socket mSocket;
        ConnectionConfig mConfig;
        std::vector<uint8_t> sendBuffer;
        ByteStream recvBuffer;
    };
}

#endif