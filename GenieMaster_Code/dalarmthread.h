#ifndef DALARMTHREAD_H
#define DALARMTHREAD_H

#include <QThread>
#include <QFile>

class DAlarmThread : public QThread
{
    Q_OBJECT
public:
    DAlarmThread(QObject *parent = Q_NULLPTR);
    ~DAlarmThread();

public:
    void run();
    void stop();

    void initFile();
    bool isPathExist(const QString&);

signals:
    void openFileError(const QString&);
    void sendAlarmMessage(QStack<QString>&);

protected slots:
    void writeAlarm(const QString&);
    void readAlarm();

private:
    QFile m_file;
};

#endif // DALARMTHREAD_H
