#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QFileInfo>
#include <QDebug>
#include "workerthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    //QList<QFileInfo> files;
    //QList<QFileInfo> getFiles();
    QStringList tmpFiles;
    WorkerThread *workerThread = nullptr;
    qint64 updated = 0;

    void checkSrcDestFolder();
    bool checkSettings(bool onlySourceDir = false);
    void enableButtons(bool enabled);


public slots:
    void browseSourceFolder();
    void browseDestinationFolder();
    void previewFiles();
    void startWork();
    void abortThread();
    void onFinished(bool success);
    void onPublish(int progress, bool success);
    void onPublishTotalFiles(int totalFiles);
    void onPreviewFinished(QList<QFileInfo> files);
    void onAborted();
    void disableDestinationControls(bool disabled);
};
#endif // MAINWINDOW_H
