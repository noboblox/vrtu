#ifndef VRTU_QTSERVERWRAPPER_HPP_
#define VRTU_QTSERVERWRAPPER_HPP_

#include <memory>
#include <mutex>

#include <qobject.h>

#include "protocols/iec104/server.hpp"

namespace IEC104
{
    class Apdu;
}

namespace VRTU
{

    /**
     * @brief Wrapper for IEC104::Server to be shared between QtWidget thread and network thread
     * 
     * - 1. Signals from the network thread to the gui thread are modelled as QT signal with Qt::QueuedConnection
     * - 2. Signals from the gui thread to the network thread are modelled as CORE::SignalEveryone<T>
     * - 3. When the server is accessed it is locked for parallel access
     */
    class QtServerWrapper : public QObject
    {
        Q_OBJECT

    public:
        explicit QtServerWrapper(QObject* apParent, boost::asio::io_context& arContext, const boost::asio::ip::address& arIP, uint16_t aListeningPort = 2404)
            : QObject(apParent),
              mServer(arContext, arIP, aListeningPort)
        {
        }

        void Start()
        {
            std::lock_guard<std::mutex> lock(mObjectLock);
            mServer.Start();
        }

    private:
        mutable std::mutex mObjectLock;
        IEC104::Server mServer;
    };
}

#endif

