#pragma once
#include <QObject>
#include <QStringList>

class ApplicationAPI;

class CopyWorkerCore : public QObject
{
    Q_OBJECT
public:
    CopyWorkerCore(const QStringList &files,
                   const QString &targetDir,
                   ApplicationAPI *api,
                   QObject *parent = nullptr);

public slots:
    void start();

signals:
    void finished();

private:
    QStringList    m_files;
    QString        m_targetDir;
    ApplicationAPI *m_api;
};
