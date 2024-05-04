#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <boost/asio/io_context.hpp>

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
    QTimer networkTick;
    void FillIpSelectBox();
    Ui::MainWindow *ui;
    boost::asio::io_context ctx;
};
#endif // MAINWINDOW_H
