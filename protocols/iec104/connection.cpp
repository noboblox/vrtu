#include "connection.hpp"

#include <iomanip>
#include <iostream>

#include <boost/asio/write.hpp>

#include "core/bytestream.hpp"

namespace IEC104
{

    Connection::Connection(boost::asio::ip::tcp::socket&& arSocket, const std::function<void(Connection&)>& arClosedHandler)
        : mSocket(std::move(arSocket)),
        ClosedHandler(arClosedHandler),
        mReadBuffer(), mWriteBuffer(),
        mCurrentSize(0),
        mNextReceiveId(0), mNextSendId(0), mLastAcknoledgedId(0)
    {
    }

    bool Connection::HandleSequences(const IEC104::Apdu& arReceived)
    {
        bool success = true;

        if (arReceived.HasSendCounter())
        {
            if (mNextReceiveId == arReceived.GetSendCounter())
                IEC104::Apdu::IncrementSequence(mNextReceiveId);
            else
                success = false;
        }

        if (arReceived.HasReceiveCounter())
        {
            mLastAcknoledgedId = arReceived.GetReceiveCounter();
        }
        return success;
    }

    bool Connection::AcknowledgeReceivedAsdus()
    {
        AsduAcknowledgeApdu send(mNextReceiveId);
        boost::system::error_code ec;
        PrintMessage(send, true);

        boost::asio::write(mSocket, boost::asio::buffer(send.GetData(), IEC104::Apdu::GetHeaderSize()), ec);
        return !ec.failed();
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

        PrintMessage(mReceived, false);
        RespondTo(mReceived);
        return true;
    }

    void Connection::ConfirmService(const Apdu& arReceived)
    {
        const Apdu confirmation = arReceived.CreateConfirmation();
        PrintMessage(confirmation, true);
        boost::asio::write(mSocket,
            boost::asio::buffer(confirmation.GetData(), confirmation.GetHeaderSize()));
    }


    void Connection::RespondTo(const Apdu& arReceived)
    {
        if (arReceived.NeedsConfirmation())
        {
            ConfirmService(arReceived);
        }
        else if (arReceived.HasAsdu())
        {
            AcknowledgeReceivedAsdus();
        }
    }

    void Connection::PrintMessage(const IEC104::Apdu& arMessage, bool aIsSend)
    {

        std::cout << "[" << (aIsSend ? "--> " : "<-- ") << mSocket.remote_endpoint().address().to_string() << ":" << std::setw(5) << mSocket.remote_endpoint().port() << "]"
            << " [" << std::setw(12) << std::left << arMessage.TypeToString() << "]";

        if (aIsSend)
        {
            std::cout << " r = " << std::setw(5) << (arMessage.HasReceiveCounter() ? std::to_string(arMessage.GetReceiveCounter()) : std::string(""))
                << ", s = " << std::setw(5) << (arMessage.HasSendCounter() ? std::to_string(arMessage.GetSendCounter()) : std::string(""));
        }
        else
        {
            std::cout << " s = " << std::setw(5) << (arMessage.HasSendCounter() ? std::to_string(arMessage.GetSendCounter()) : std::string(""))
                << ", r = " << std::setw(5) << (arMessage.HasReceiveCounter() ? std::to_string(arMessage.GetReceiveCounter()) : std::string(""));
        }


        std::cout << ", ASDU size = " << arMessage.GetAsduSize() << ";" << std::endl;
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
