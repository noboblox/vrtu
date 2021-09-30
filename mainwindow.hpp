#ifndef MAINWINDOW_HPP_
#define MAINWINDOW_HPP_

#include <qmainwindow.h>
#include <qstring.h>

#include <boost/asio/io_context.hpp>
#include <thread>

namespace Ui 
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* apParent = nullptr);
    ~MainWindow();

private:
    boost::asio::io_context mContext;
    std::thread mNetworkThread; // Thread handling all non-qt tcp connections
    QString mThreadErrorMsg, mThreadErrorInfoMsg;

    Ui::MainWindow* ui;

    void NetworkingThreadFunction() noexcept;

private slots: 
    /// Process error reason from network thread. The event is forced to be send over event queue, as only the main thread is allowed to create widgets
    /// The message is stored inside mThreadErrorMsg and mThreadErrorInfoMsg. The network thread writes the data before executing the signal
    void ReceiveThreadAbort();

signals:
    /// Called by the network thread to signal unexpected termination to the main (GUI) thread
    void SendThreadAbort();
};

#endif // MAINWINDOW_HPP
