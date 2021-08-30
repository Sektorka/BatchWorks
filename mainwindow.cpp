#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QAbstractButton>
#include <QScreen>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    move(QGuiApplication::primaryScreen()->geometry().center() - frameGeometry().center());

    connect(ui->btnBrowseSourceFolder, &QAbstractButton::pressed, this, &MainWindow::browseSourceFolder);
    connect(ui->btnBrowseDestinationFolder, &QAbstractButton::pressed, this, &MainWindow::browseDestinationFolder);
    connect(ui->btnPreview, &QAbstractButton::pressed, this, &MainWindow::previewFiles);
    connect(ui->btnStart, &QAbstractButton::pressed, this, &MainWindow::startWork);
    connect(ui->btnAbort, &QAbstractButton::pressed, this, &MainWindow::abortThread);
    connect(ui->leSourceFolder, &QLineEdit::selectionChanged, this, &MainWindow::browseSourceFolder);
    connect(ui->leDestinationFolder, &QLineEdit::selectionChanged, this, &MainWindow::browseDestinationFolder);
    connect(ui->rbDelete, &QRadioButton::toggled, this, &MainWindow::disableDestinationControls);
}

MainWindow::~MainWindow()
{
    if(workerThread != nullptr){
        workerThread->quit();
        workerThread->wait();
    }

    delete ui;
}

void MainWindow::checkSrcDestFolder()
{
    if(!ui->leSourceFolder->text().isEmpty()
            && !ui->leDestinationFolder->text().isEmpty()
            && ui->leSourceFolder->text() == ui->leDestinationFolder->text())
    {
        QMessageBox::warning(this, windowTitle(), "Source and Destination folders are same!\nSelect another destination folder.");
    }
}

void MainWindow::browseSourceFolder()
{
    ui->leSourceFolder->setText(
                QFileDialog::getExistingDirectory(this, "Select source folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    checkSrcDestFolder();
}

void MainWindow::browseDestinationFolder()
{
    ui->leDestinationFolder->setText(
                QFileDialog::getExistingDirectory(this, "Select destination folder", "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
    checkSrcDestFolder();
}

bool MainWindow::checkSettings(bool onlySourceDir)
{
    bool error = false;
    QString strError = "Missing settings:\n";

    QDir srcDir(ui->leSourceFolder->text());
    QDir dstDir(ui->leDestinationFolder->text());

    if(!ui->leSourceFolder->text().isEmpty() && srcDir.exists() && !ui->leDestinationFolder->text().isEmpty() && dstDir.exists())
    {
        if(srcDir.absolutePath() == dstDir.absolutePath())
        {
            error = true;
            strError += " - source and Destination folders are same! -> select another destination folder\n";
        }
    }
    else
    {
        if(ui->leSourceFolder->text().isEmpty())
        {
            error = true;
            strError += " - select source folder\n";
        }
        else {
            if(!srcDir.exists())
            {
                error = true;
                strError += " - source folder does not exist\n";
            }
        }

        if(!onlySourceDir)
        {
            if(ui->leDestinationFolder->text().isEmpty())
            {
                error = true;
                strError += " - select destination folder\n";
            }
            else {
                if(!dstDir.exists())
                {
                    error = true;
                    strError += " - destination folder does not exist\n";
                }
            }
        }
    }

    if(error)
    {
        QMessageBox::warning(this, windowTitle(), strError);
    }

    return !error;
}

void MainWindow::enableButtons(bool enabled)
{
    ui->btnPreview->setEnabled(enabled);
    ui->btnStart->setEnabled(enabled);
    ui->btnAbort->setEnabled(!enabled);
}

void MainWindow::previewFiles()
{
    if(checkSettings(true)){
        ui->teLog->clear();
        enableButtons(false);

        ui->pbTaskState->setMaximum(0);

        workerThread = new WorkerThread(WorkerThread::WorkTypes::Preview);
        workerThread->setMod(ui->sbEveryX->value());
        workerThread->setStartWith(ui->sbStartWith->value());
        workerThread->setSourceDir(ui->leSourceFolder->text());

        connect(workerThread, &WorkerThread::onPreviewFinished, this, &MainWindow::onPreviewFinished);
        connect(workerThread, &WorkerThread::onAborted, this, &MainWindow::onAborted);
        connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);

        workerThread->start();
    }
}

void MainWindow::onPreviewFinished(QList<QFileInfo> files)
{
    if(workerThread != nullptr)
    {
        //delete workerThread;
        workerThread = nullptr;
    }

    QString result = "";

    foreach (QFileInfo fileInfo, files)
    {
        result.append(fileInfo.absoluteFilePath() + "\n");
    }

    ui->teLog->setPlainText(result);
    enableButtons(true);
    ui->pbTaskState->setMaximum(1);
}

void MainWindow::onAborted()
{
    QMessageBox::information(this, windowTitle(), "Process aborted");
    workerThread = nullptr;
    enableButtons(true);
}

void MainWindow::disableDestinationControls(bool disabled)
{
    ui->btnBrowseDestinationFolder->setEnabled(!disabled);
    ui->leDestinationFolder->setEnabled(!disabled);
}

void MainWindow::startWork()
{
    if(checkSettings())
    {
        WorkerThread::WorkTypes workType;

        if(ui->rbDelete->isChecked())
        {
            workType = WorkerThread::WorkTypes::Delete;
        }
        else if(ui->rbMove->isChecked())
        {
            workType = WorkerThread::WorkTypes::Move;
        }
        else {
            workType = WorkerThread::WorkTypes::Copy;
        }

        ui->pbTaskState->setMaximum(0);
        ui->teLog->clear();

        workerThread = new WorkerThread(workType);

        workerThread->setMod(ui->sbEveryX->value());
        workerThread->setStartWith(ui->sbStartWith->value());
        workerThread->setSourceDir(ui->leSourceFolder->text());
        workerThread->setDestinationDir(ui->leDestinationFolder->text());

        connect(workerThread, &WorkerThread::onPublish, this, &MainWindow::onPublish);
        connect(workerThread, &WorkerThread::onPublishTotalFiles, this, &MainWindow::onPublishTotalFiles);
        connect(workerThread, &WorkerThread::onFinished, this, &MainWindow::onFinished);
        connect(workerThread, &WorkerThread::onAborted, this, &MainWindow::onAborted);
        connect(workerThread, &WorkerThread::finished, workerThread, &QObject::deleteLater);

        workerThread->start();
        enableButtons(false);
    }
}

void MainWindow::abortThread()
{
    if(workerThread != nullptr)
    {
        workerThread->abort();
    }
}

void MainWindow::onFinished(bool success)
{
    if(workerThread != nullptr)
    {
        //delete workerThread;
        workerThread = nullptr;
    }

    ui->pbTaskState->setValue(ui->pbTaskState->maximum());
    enableButtons(true);

    QMessageBox::information(this,  windowTitle(), "Operation finished");
}

void MainWindow::onPublish(int progress, bool success)
{

    //qInfo() << QDateTime::currentMSecsSinceEpoch() << " | " << QDateTime::currentMSecsSinceEpoch() - updated;
    if(QDateTime::currentMSecsSinceEpoch() - updated > 50)
    {
        //qInfo() << progress << " " << success;
        tmpFiles.push_back(workerThread->getFile(--progress).absoluteFilePath() + " - " + (success ? "OK" : "FAIL"));
        ui->pbTaskState->setValue(progress);
        ui->teLog->appendPlainText(tmpFiles.join("\n"));
        tmpFiles.clear();
        updated = QDateTime::currentMSecsSinceEpoch();
    }
    else
    {
        tmpFiles.push_back(workerThread->getFile(--progress).absoluteFilePath() + " - " + (success ? "OK" : "FAIL"));
    }
}

void MainWindow::onPublishTotalFiles(int totalFiles)
{
    ui->pbTaskState->setMaximum(totalFiles);
}


