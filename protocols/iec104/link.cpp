#include "link.hpp"

#include <iomanip>
#include <iostream>

#include <boost/asio/write.hpp>
#include <boost/cobalt/join.hpp>
#include <boost/cobalt/op.hpp>
#include <boost/cobalt/race.hpp>

#include "core/bytestream.hpp"
#include "protocols/iec104/asdu.hpp"
#include "protocols/iec104/infoobjects.hpp"

namespace IEC104
{
    Link::Link(boost::asio::ip::tcp::socket&& arSocket, Mode mode, const ConnectionConfig& arConfig)
        : mIsMaster(mode == Mode::Master)
        , mSocket(std::move(arSocket))
        , mConfig(arConfig)
        , recvBuffer(4096)
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
                co_await async::join(promiseTick, promiseDelay);
            }
        }
        catch (...) {}

        setRunning(false);
        co_return;
    }

    async::promise<void> Link::Tick()
    {
        co_await HandleReceive();
        co_await HandleTimers();

        SignalTickFinished(*this);
        co_return;
    }

    async::promise<void> Link::Delay(std::chrono::milliseconds msec)
    {
        asio::steady_timer t(co_await asio::this_coro::executor, msec);
        co_await t.async_wait(async::use_op);
        co_return;
    }

    async::promise<void> Link::ActivateService(const Apdu& service)
    {
        if (ServicePending())
            throw std::runtime_error("cannot activate service while another is pending");

        mPending = service.ServiceActivation();
        co_await Send(service);
        co_return;
    }

    bool Link::ServicePending() const noexcept
    {
        return mPending != ServiceType::NONE;
    }

    async::promise<void> Link::Send(const Apdu& apdu)
    {
        apdu.WriteTo(sendBuffer);
        co_await mSocket.async_send(boost::asio::buffer(sendBuffer), async::use_op);
        sendBuffer.clear();
        SignalApduSent(*this, apdu);
        co_return;
    }

    async::promise<void> Link::SendAck()
    {
        co_await Send(Apdu(seqRecv));
        seqMyLastAck = seqRecv;
        mMyAckPendingSince = UtcNow();
    }

    async::promise<void> Link::HandleReceive()
    {
        if (!mSocket.available())
            co_return;

        auto buf = boost::asio::buffer(recvBuffer.WriteBegin(), recvBuffer.WritableBytes());
        auto recv = co_await mSocket.async_read_some(buf, async::use_op);
        recvBuffer.BytesWritten(recv);

        while (Apdu::IsFullyAvailable(recvBuffer))
        {
            auto apdu = Apdu(recvBuffer);
            co_await HandleApdu(apdu);
        }

        recvBuffer.Flush();
        co_return;
    }

    async::promise<void> Link::HandleTimers()
    {
        if (!ServicePending() && seqPeerLastAck == seqSend)
            mPeerAckPendingSince = UtcNow();
        
        if (seqMyLastAck == seqRecv)
            mMyAckPendingSince = UtcNow();

        if (TimerT1() > std::chrono::seconds(mConfig.GetT1()))
            throw std::runtime_error("peer ack timed out");

        if (TimerT2() > std::chrono::seconds(mConfig.GetT2()))
            co_await SendAck();

        bool testEnabled = (mConfig.GetT3() > 0);

        if (testEnabled && !ServicePending() && (TimerT3() > std::chrono::seconds(mConfig.GetT3())))
            co_await Test();

        co_return;
    }

    async::promise<void> Link::HandleApdu(const Apdu& apdu)
    {
        mNoTrafficSince = UtcNow();
        SignalApduReceived(*this, apdu);

        HandleApduServiceCon(apdu);
        co_await HandleApduServiceAct(apdu);
        HandlePeerRecvSequence(apdu);
        co_await HandlePeerSendSequence(apdu);
    }

    async::promise<void> Link::HandleApduServiceAct(const Apdu& apdu)
    {
        switch (apdu.ServiceActivation()) {
        case ServiceType::START:
            co_await ActivateLink();
            break;
        case ServiceType::STOP:
            co_await DeactivateLink();
            break;
        case ServiceType::TEST:
            co_await Send(Apdu::TESTFR_CON);
            break;
        default:
            break;
        }
        co_return;
    }

    async::promise<void> Link::HandlePeerSendSequence(const Apdu& apdu)
    {
        auto sent = apdu.SendSequence();

        if (!sent.has_value())
            co_return;

        if (sent.value() != seqRecv)
            throw std::runtime_error("peer telegram has unexpected send sequence");

        ++seqRecv;

        if (CurrentW() >= mConfig.GetW())
            co_await SendAck();
        co_return;
    }

    void Link::HandleApduServiceCon(const Apdu& apdu)
    {
        auto confirmed = apdu.ServiceConfirmation();

        if (confirmed != mPending)
            return;

        mPending = ServiceType::NONE;

        switch (confirmed) {
        case ServiceType::START:
            PeerActivated();
            break;
        case ServiceType::STOP:
            PeerDeactivated();
            break;
        default:
            break;
        }
    }

    void Link::HandlePeerRecvSequence(const Apdu& apdu)
    {
        auto recv = apdu.ReceiveSequence();

        if (!recv.has_value())
            return;

        auto acked = seqPeerLastAck.Distance(recv.value());

        if (acked < 0)
            throw std::runtime_error("peer recv sequence is not plausible");

        if (acked > 0) {
            seqPeerLastAck = recv.value();
            mPeerAckPendingSince = UtcNow();
        }
    }

    async::promise<void> Link::ActivateLink()
    {
        co_await Send(Apdu::STARTDT_CON);

        if (!IsMaster())
            setActive(true);
    }

    async::promise<void> Link::DeactivateLink()
    {
        co_await Send(Apdu::STOPDT_CON);

        if (!IsMaster())
            setActive(false);
    }

    void Link::PeerActivated()
    {
        if (IsMaster())
            setActive(true);
    }

    void Link::PeerDeactivated()
    {
        if (IsMaster())
            setActive(false);
    }

    void Link::Cancel()
    {
        mNeedClose = true;
    }

    async::promise<void> Link::Start()
    {
        co_await ActivateService(Apdu::STARTDT_ACT);
        co_return;
    }

    async::promise<void> Link::Stop()
    {
        co_await ActivateService(Apdu::STOPDT_ACT);
        co_return;
    }

    async::promise<void> Link::Test()
    {
        co_await ActivateService(Apdu::TESTFR_ACT);
        co_return;
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

    Link::~Link() noexcept
    {
        CloseSocket();
    }
}
