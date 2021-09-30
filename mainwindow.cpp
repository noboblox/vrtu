#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <boost/asio/io_context.hpp>
#include <sstream>
#include <thread>

#include <qmessagebox.h>

#include "qtserverwrapper.hpp"

MainWindow::MainWindow(QWidget* apParent) :
    QMainWindow(apParent),
    mContext(),
    mNetworkThread(&MainWindow::NetworkingThreadFunction, this),
    ui(new Ui::MainWindow)
{
    connect(this, &MainWindow::SendThreadAbort, this, &MainWindow::ReceiveThreadAbort, Qt::QueuedConnection);
    ui->setupUi(this);

    // TODO: dummy for testing
    new VRTU::QtServerWrapper(this, mContext, boost::asio::ip::make_address("127.0.0.1"), 2404);
}

void
MainWindow::NetworkingThreadFunction() noexcept
{
    try
    {
        boost::asio::executor_work_guard<boost::asio::io_context::executor_type> never_return_from_run
            = boost::asio::make_work_guard(mContext); 
        mContext.run();
    }
    catch (std::exception& arError)
    {
        std::stringstream error_msg;
        error_msg << "Exception inside network thread: " << arError.what();
        mThreadErrorMsg = error_msg.str().c_str();
        mThreadErrorInfoMsg = "All network commuication was shut down";
        SendThreadAbort();
    }

    catch (...)
    {
        mThreadErrorMsg = "Unknown exception inside network thread";
        mThreadErrorInfoMsg = "All network commuication was shut down";
        SendThreadAbort();
    }
}

void
MainWindow::ReceiveThreadAbort()
{
    QMessageBox fatal_error;
    fatal_error.setIcon(QMessageBox::Icon::Critical);
    fatal_error.setStandardButtons(QMessageBox::Ok);
    fatal_error.setText(mThreadErrorMsg);
    fatal_error.setInformativeText(mThreadErrorInfoMsg);
    fatal_error.exec();
}

MainWindow::~MainWindow()
{
    mContext.stop(); // io_context::stop should be thread safe
    mNetworkThread.join();
    delete ui;
}
