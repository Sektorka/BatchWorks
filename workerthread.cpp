#include "workerthread.h"
#include <QDir>

void WorkerThread::run() {
    QDir dir(sourceDir);

    int c = 0;
    bool ignoreDone = false;

    foreach (QFileInfo fileInfo, dir.entryInfoList())
    {
        if(aborted)
        {
            emit onAborted();
            return;
        }

        if(fileInfo.isFile() && fileInfo.isReadable())
        {
            c++;

            if(!ignoreDone){
                if(c < startWith)
                {
                    continue;
                }
                else
                {
                    ignoreDone = true;
                    c = 1;
                }
            }

            if(c % mod == 1 || mod == 1)
            {
                files.push_back(fileInfo);
            }
        }
    }

    if(workType == WorkTypes::Preview)
    {
        emit onPreviewFinished(files);
    }
    else
    {
        emit onPublishTotalFiles(files.count());

        bool success = false;
        c = 0;

        foreach(QFileInfo fileInfo, files)
        {
            if(aborted)
            {
                emit onAborted();
                return;
            }

            c++;

            switch (workType) {

            case WorkTypes::Delete:
                success = QFile::remove(fileInfo.absoluteFilePath());
                break;
            case WorkTypes::Move:
                success = move(fileInfo.absoluteFilePath(), destinationDir + QDir::separator() + fileInfo.fileName());
                break;
            default:
                success = copy(fileInfo.absoluteFilePath(),destinationDir + QDir::separator() + fileInfo.fileName());
                break;

            }

            emit onPublish(c, success);
        }

        emit onFinished(true);
    }
}

WorkerThread::WorkerThread(WorkTypes workType):
    workType(workType)
{

}

const QFileInfo &WorkerThread::getFile(int progress)
{
    return files.at(progress);
}

void WorkerThread::setMod(int mod)
{
    this->mod = mod;
}

void WorkerThread::setStartWith(int startWith)
{
    this->startWith = startWith;
}

void WorkerThread::setSourceDir(QString sourceDir)
{
    this->sourceDir = sourceDir;
}

void WorkerThread::setDestinationDir(QString destinationDir)
{
    this->destinationDir = destinationDir;
}

void WorkerThread::abort()
{
    this->aborted = true;
}

bool WorkerThread::copy(QString source, QString destination)
{
    bool removed = false;

    while (!QFile::copy(source, destination) && !removed)
    {
        removed = QFile::remove(destination);

        if(!removed)
        {
            return false;
        }
    }

    return true;
}

bool WorkerThread::move(QString source, QString destination)
{
    return copy(source, destination) && QFile::remove(source);
}
