#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QThread>
#include <QList>
#include <QFileInfo>

class WorkerThread : public QThread
{
    Q_OBJECT
    void run() override;



public:
    enum class WorkTypes
    {
        Copy,
        Move,
        Delete,
        Preview
    };

    WorkerThread(WorkTypes workType);
    const QFileInfo &getFile(int progress);
    void setMod(int mod);
    void setStartWith(int startWith);
    void setSourceDir(QString sourceDir);
    void setDestinationDir(QString destinationDir);
    void abort();

private:
    WorkTypes workType;
    QList<QFileInfo> files;
    QString sourceDir, destinationDir;
    int mod = 2, startWith = 1;
    bool aborted = false;

    bool copy(QString source, QString destination);
    bool move(QString source, QString destination);
    QList<QFileInfo> getFiles();

signals:
    void onFinished(bool success);
    void onPublish(int progress, bool success);
    void onPublishTotalFiles(int totalFiles);
    void onPreviewFinished(QList<QFileInfo> files);
    void onAborted();
};

#endif // WORKERTHREAD_H
