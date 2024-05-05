#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#undef emit // it is unfortunate, that qt defines emit as macro, which also is a function in boost's forward_cancellation.hpp...
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/cobalt/task.hpp>

namespace IEC104
{
    class Apdu;
    class Link;
    class Server;
}

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onStartClicked();
    void onStopClicked();
    void executeNetworkTasks();

private:
    boost::cobalt::task<void> RunServer(const boost::asio::ip::address ip, uint16_t port);

    void OnLinkStateChanged(IEC104::Link& l);
    void OnLinkTickFinished(IEC104::Link& l);
    void OnApduReceived(IEC104::Link& l, const IEC104::Apdu& msg);
    void OnApduSent(IEC104::Link& l, const IEC104::Apdu& msg);
    void OnServerStartedStopped(IEC104::Server& s);

private:
    QTimer networkTick;
    void FillIpSelectBox();
    Ui::MainWindow *ui;
    boost::asio::io_context ctx;
    std::unique_ptr<IEC104::Server> server;

};
#endif // MAINWINDOW_H
