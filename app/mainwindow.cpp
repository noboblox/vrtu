#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
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