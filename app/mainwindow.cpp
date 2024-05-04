#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QHostInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , networkTick(this)
{
    ui->setupUi(this);
    FillIpSelectBox();

    networkTick.callOnTimeout(this, &MainWindow::executeNetworkTasks);
    networkTick.start(20);
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