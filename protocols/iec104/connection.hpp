#ifndef IEC104_CONNECTION_HPP_
#define IEC104_CONNECTION_HPP_

#include <array>
#include <cstdint>
#include <functional>

#include <boost/asio/ip/tcp.hpp>

#include "core/watchdog.hpp"
#include "core/signal.hpp"

#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/connectionconfig.hpp"

namespace IEC104
{
    class BaseInfoObject;

    class Connection
    {
    public:
        // Create an connection with an connected socket
        explicit Connection(boost::asio::io_context& arContext, boost::asio::ip::tcp::socket&& arSocket, const ConnectionConfig& arConfig,
                            const std::function<void(Connection&)>& arClosedHandler);
        ~Connection();

        // Start message processing 
        void Start();

        bool ConfirmReceivedAsdus();

    private:
        // Start an async read of the next APDU -> ReadPartial()
        void ReceiveNextMessage();

        // Read a partial message from the socket -> OnBytesReceived
        void ReadPartial(void* apDestination, size_t aTargetSize);
        
        // Check the APDU for completion
        //  - If the message is incompleted, another async read is requested via -> ReadPartial()
        //  - If the message is complete, it's proceesed via -> ProcessMessage() 
        //    and the next message is expected afterwards -> ReceiveNextMessage()
        void OnBytesReceived(const boost::system::error_code& arError, size_t aBytesReceived);
        
        // Decode the message and respond confirmations
        bool ProcessMessage();
        void RespondTo(const Apdu& arReceived);
        void DeployMessage(const Apdu& arReceived);

        // Print the message on screen
        void PrintMessage(const IEC104::Apdu& arMessage, bool aIsSend) const;
        void PrintAsdu(const IEC104::Asdu& arAsdu) const;
        void PrintInfoObject(const BaseInfoObject& arInfoObject) const;


        // Verify that a message is expected and update sequence counters
        bool HandleSequences(const IEC104::Apdu& arReceived);
        bool IsAsduConfirmThresholdReached() const noexcept;
        
        void ConfirmService(const Apdu& arReceived);

        // Close the connection with an error message
        void CloseError(const std::string& arErrorMsg);


    private:
        static constexpr const char SUB_TREE_PREFIX[] =        "|   ";
        static constexpr const char SUB_TREE_BRANCH_PREFIX[] = "|__ ";

        boost::asio::io_context& mrContext;
        boost::asio::ip::tcp::socket mSocket;
        std::function<void(Connection&)> ClosedHandler;
        std::array<uint8_t, 256> mReadBuffer, mWriteBuffer;
        size_t mCurrentSize;
        int mNextReceiveId, mNextSendId;
        int mLastConfirmedByLocal, mLastConfirmedByRemote;
        Apdu mReceived;
        ConnectionConfig mConfig;
        CORE::Watchdog mAsduConfirmationTrigger;

        CORE::SignalEveryone<void, Connection&, const Apdu&> SignalReceivedApdu;
    };
}

#endif