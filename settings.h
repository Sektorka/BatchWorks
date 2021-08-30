#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
    Q_OBJECT
public:
    static Settings *instance(QObject *parent = 0);
    static const QString programName;

private:
    explicit Settings(QObject *parent = 0);
    static Settings *settings;
    static const QString file;
    static const QString dir, strDir;
    static QString getEnv(const wchar_t *key);
};

#endif // SETTINGS_H
