#include "settings.h"
#include <stdlib.h>
#include <QDir>

const QString Settings::programName = "BatchWorks";
const QString Settings::strDir = QDir::toNativeSeparators(getEnv(L"APPDATA") + "/" + programName + "/");
const QString Settings::dir = (QDir(strDir).exists() ? strDir + "/" : QDir("C:/").exists() ? "C:/" : QDir::currentPath() + "/");
const QString Settings::file = programName + ".conf";
Settings *Settings::settings = nullptr;

Settings::Settings(QObject *parent) :
    QSettings(dir + file, QSettings::IniFormat, parent)
{
}

Settings *Settings::instance(QObject *parent){
    if(settings == NULL){
        settings = new Settings(parent);
    }

    return settings;
}

QString Settings::getEnv(const wchar_t *key)
{
    wchar_t *data;
    size_t len = 0;
    if(_wdupenv_s(&data, &len, key)) {
        QString qData = QString::fromWCharArray(data);
        free(data);
        return qData;
    }

    return "";
}
