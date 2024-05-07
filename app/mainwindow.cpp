#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHostInfo>

#include <boost/asio/detached.hpp>
#include <boost/cobalt/spawn.hpp>
#include <boost/cobalt/task.hpp>
#include <boost/cobalt/run.hpp>

#include "protocols/iec104/apdu.hpp"
#include "protocols/iec104/link.hpp"
#include "protocols/iec104/server.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkTick(this)
{
    ui->setupUi(this);
    async::this_thread::set_executor(ctx.get_executor());

    FillIpSelectBox();

    networkTick.callOnTimeout(this, &MainWindow::executeNetworkTasks);
    networkTick.start(50);

    ui->editPort->setValidator(new QIntValidator(0, 65535, this));
    ui->tableConnections->setColumnCount(4);
    ui->tableConnections->hideColumn(0);
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

    server.reset(new IEC104::Server(ip, port));

    ui->btStart->setEnabled(false);
    ui->btStop->setEnabled(true);
    ui->cbIpSelect->setEnabled(false);
    ui->editPort->setEnabled(false);

    server->SignalApduReceived.Register([this](IEC104::Link& l, const IEC104::Apdu& msg) { OnApduReceived(l, msg);    });
    server->SignalApduSent.Register([this](IEC104::Link& l, const IEC104::Apdu& msg) { OnApduSent(l, msg);        });
    server->SignalLinkStateChanged.Register([this](IEC104::Link& l) { OnLinkStateChanged(l);});
    server->SignalLinkTickFinished.Register([this](IEC104::Link& l) { OnLinkTickFinished(l);     });

    AddServer();
}

void MainWindow::onStopClicked()
{
    RemoveServer();
    server.reset(nullptr);

    ui->btStart->setEnabled(true);
    ui->btStop->setEnabled(false);
    ui->cbIpSelect->setEnabled(true);
    ui->editPort->setEnabled(true);
}

void MainWindow::OnLinkStateChanged(IEC104::Link& l)
{  
    auto table = ui->tableConnections;
    auto items = table->findItems(QString::number(reinterpret_cast<intptr_t>(&l)), Qt::MatchFlag::MatchExactly);

    auto row = 0;
    if (items.empty()) {
        row = table->rowCount();
        table->insertRow(row);

        table->setItem(row, 0, new QTableWidgetItem(QString::number(reinterpret_cast<intptr_t>(&l))));
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(l.RemoteIp().to_string())));
        table->setItem(row, 2, new QTableWidgetItem(QString::number(l.RemotePort())));
    }
    else if(!l.IsConnected()) {
        table->removeRow(items.front()->row());
        return;
    }
    else {
        row = items.front()->row();
    }

    if (l.IsActive())
        table->setItem(row, 3, new QTableWidgetItem("ACTIVE"));
    else
        table->setItem(row, 3, new QTableWidgetItem("PASSIVE"));
}

void MainWindow::OnLinkTickFinished(IEC104::Link& l)
{
    if (1)
        return; // debug stub
}

void MainWindow::OnApduReceived(IEC104::Link& l, const IEC104::Apdu& msg)
{
    if (1)
        return; // debug stub
}

void MainWindow::OnApduSent(IEC104::Link& l, const IEC104::Apdu& msg)
{
    if (1)
        return; // debug stub
}

void MainWindow::AddServer()
{
    if (server == nullptr)
        return;

    auto table = ui->tableConnections;
    auto row = table->rowCount();
    table->insertRow(row);

    table->setItem(row, 0, new QTableWidgetItem(QString::number(reinterpret_cast<intptr_t>(server.get()))));
    table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(server->LocalIp().to_string())));
    table->setItem(row, 2, new QTableWidgetItem(QString::number(server->LocalPort())));
    table->setItem(row, 3, new QTableWidgetItem(QString("RUNNING")));
}

void MainWindow::RemoveServer()
{
    auto table = ui->tableConnections;
    auto items = table->findItems(QString::number(reinterpret_cast<intptr_t>(server.get())), Qt::MatchFlag::MatchExactly);

    if (!items.isEmpty()) 
        table->removeRow(items.first()->row());
}

boost::cobalt::task<void> MainWindow::ServerTick()
{
    if (server == nullptr)
        co_return;

    co_await server->Tick();
    co_return;
}

void MainWindow::executeNetworkTasks()
{
    try
    {
        async::spawn(ctx, ServerTick(), asio::detached);
        ctx.poll();
    }
    catch (std::exception e) 
    {
    }

}
