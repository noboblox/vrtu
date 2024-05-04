#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <boost/asio/io_context.hpp>
//#include <boost/cobalt/task.hpp>
//#include "protocols/iec104/server.hpp"

namespace IEC104
{
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
//    boost::cobalt::task<void> RunServer(const boost::asio::ip::address& ip, uint16_t port);

private:
    QTimer networkTick;
    void FillIpSelectBox();
    Ui::MainWindow *ui;
    boost::asio::io_context ctx;
    std::unique_ptr<IEC104::Server> server;

};
#endif // MAINWINDOW_H
