#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHostInfo>

#include <boost/asio/detached.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>

#include "protocols/iec104/server.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkTick(this)
{
    ui->setupUi(this);
    FillIpSelectBox();

    networkTick.callOnTimeout(this, &MainWindow::executeNetworkTasks);
    networkTick.start(20);

    ui->editPort->setValidator(new QIntValidator(0, 65535, this));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::FillIpSelectBox()
{
    ui->cbIpSelect->addItem(QHostAddress(QHostAddress::SpecialAddress::LocalHost).toString());
    ui->cbIpSelect->addItem(QHostAddress(QHostAddress::SpecialAddress::LocalHostIPv6).toString());
    ui->cbIpSelect->setCurrentIndex(0);

    auto info = QHostInfo::fromName(QHostInfo::localHostName());

    for (auto& addr : info.addresses()) 
    {
        ui->cbIpSelect->addItem(addr.toString());
    }
}

void MainWindow::onStartClicked()
{
    boost::system::error_code ec;
    auto ip = boost::asio::ip::address::from_string(ui->cbIpSelect->currentText().toStdString(), ec);

    if (ec.failed() || !ui->editPort->hasAcceptableInput())
        return;

    auto port = ui->editPort->text().toInt();
    async::spawn(ctx, RunServer(ip, port), boost::asio::detached);

    ui->btStart->setEnabled(false);
    ui->btStop->setEnabled(true);
    ui->cbIpSelect->setEnabled(false);
    ui->editPort->setEnabled(false);
}

void MainWindow::onStopClicked()
{
    ui->btStart->setEnabled(true);
    ui->btStop->setEnabled(false);
    ui->cbIpSelect->setEnabled(true);
    ui->editPort->setEnabled(true);
}

boost::cobalt::task<void> MainWindow::RunServer(const boost::asio::ip::address ip, uint16_t port)
{
    server.reset(new IEC104::Server(ip, port));
    co_await server->Run();
    co_return;
}

void MainWindow::executeNetworkTasks()
{
    try
    {
        boost::system::error_code ec;
        ctx.poll(ec);

        // todo report the error
    }
    catch (...) 
    {
    }
}