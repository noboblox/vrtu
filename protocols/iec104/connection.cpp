#include "connection.hpp"

#include <iomanip>
#include <iostream>

#include <boost/asio/write.hpp>

#include "core/bytestream.hpp"
#include "protocols/iec104/asdu.hpp"
#include "protocols/iec104/infoobjects.hpp"

namespace IEC104
{
    Connection::Connection(boost::asio::io_context& arContext, boost::asio::ip::tcp::socket&& arSocket, const ConnectionConfig& arConfig,
                           const std::function<void(Connection&)>& arClosedHandler)
        : 
        mrContext(arContext),
        mSocket(std::move(arSocket)),
        ClosedHandler(arClosedHandler),
        mReadBuffer(), mWriteBuffer(),
        mCurrentSize(0),
        mNextReceiveId(0), mNextSendId(0),
        mLastConfirmedByLocal(0), mLastConfirmedByRemote(0),
        mConfig(arConfig),
        mAsduConfirmationTrigger(mrContext, boost::posix_time::seconds(mConfig.GetT2()))
    {
        mAsduConfirmationTrigger.SignalTimeout.Register([this] () { this->ConfirmReceivedAsdus(); });
    }

    bool Connection::HandleSequences(const IEC104::Apdu& arReceived)
    {
        bool success = true;

        if (arReceived.HasSendCounter())
        {
            if (mNextReceiveId == arReceived.GetSendCounter())
            {
                IEC104::Apdu::IncrementSequence(mNextReceiveId);
                mAsduConfirmationTrigger.StartOrContinue();
            }
            else
                success = false;
        }

        if (arReceived.HasReceiveCounter())
        {
            mLastConfirmedByRemote = arReceived.GetReceiveCounter();
        }
        return success;
    }

    bool Connection::IsAsduConfirmThresholdReached() const noexcept
    {
        return Apdu::SequenceDistance(mLastConfirmedByLocal, mNextReceiveId) >= mConfig.GetW();
    }

    bool Connection::ConfirmReceivedAsdus()
    {
        AsduAcknowledgeApdu send(mNextReceiveId);
        boost::system::error_code ec;
        boost::asio::write(mSocket, boost::asio::buffer(send.GetData(), IEC104::Apdu::GetHeaderSize()), ec);

        if (ec)
            return false;

        mLastConfirmedByLocal = mNextReceiveId;
        mAsduConfirmationTrigger.Stop();

        return true;
    }

    void Connection::ReceiveNextMessage()
    {
        mCurrentSize = 0;
        ReadPartial(mReadBuffer.data(), IEC104::Apdu::GetHeaderSize());
    }

    void Connection::ReadPartial(void* apDestination, size_t aTargetSize)
    {
        mSocket.async_read_some(boost::asio::buffer(apDestination, aTargetSize),
            [this](const boost::system::error_code& arError, size_t aBytesReceived) {this->OnBytesReceived(arError, aBytesReceived); });
    }

    void Connection::OnBytesReceived(const boost::system::error_code& arError, size_t aBytesReceived)
    {
        if (arError)
        {
            CloseError(arError.message());
            return;
        }

        mCurrentSize += aBytesReceived;

        if (mCurrentSize < IEC104::Apdu::GetHeaderSize())
            ReadPartial(&mReadBuffer[mCurrentSize], IEC104::Apdu::GetHeaderSize() - mCurrentSize);
        else if (mCurrentSize < IEC104::Apdu::GetMessageSize(mReadBuffer.data()))
            ReadPartial(&mReadBuffer[mCurrentSize], IEC104::Apdu::GetMessageSize(mReadBuffer.data()) - mCurrentSize);
        else if (mCurrentSize == IEC104::Apdu::GetMessageSize(mReadBuffer.data()))
        {
            ProcessMessage();
            ReceiveNextMessage();
        }

        return; // Error
    }


    bool Connection::ProcessMessage()
    {
        ByteStream msg;
        msg.WriteData(mReadBuffer.data(), mCurrentSize);
        mReceived.ReadFrom(msg);

        if (!mReceived.IsValid() || !HandleSequences(mReceived))
            return false;

        RespondTo(mReceived);
        DeployMessage(mReceived);

        return true;
    }

    void Connection::ConfirmService(const Apdu& arReceived)
    {
        const Apdu confirmation = arReceived.CreateServiceConfirmation();
        boost::asio::write(mSocket,
            boost::asio::buffer(confirmation.GetData(), confirmation.GetHeaderSize()));
    }


    void Connection::RespondTo(const Apdu& arReceived)
    {
        if (arReceived.IsServiceRequest())
        {
            ConfirmService(arReceived);
        }
        else if (IsAsduConfirmThresholdReached())
        {
            ConfirmReceivedAsdus();
        }
    }

    void Connection::DeployMessage(const Apdu& arReceived)
    {
        SignalReceivedApdu(*this, arReceived);
    }

    void Connection::Start()
    {
        std::cout << "Connected to " << mSocket.remote_endpoint().address().to_string() << ":" << mSocket.remote_endpoint().port() << std::endl;
        ReceiveNextMessage();
    }

    void Connection::CloseError(const std::string& arErrorMsg)
    {
        std::cout << "[ERROR] " << arErrorMsg << std::endl;

        boost::system::error_code ec;
        mSocket.close(ec);

        ClosedHandler(*this);
    }

    Connection::~Connection()
    {
        std::cout << "Conection destroyed" << std::endl;
    }
}
