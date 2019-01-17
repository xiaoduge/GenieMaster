#include "dalarmthread.h"
#include <QDateTime>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <QStack>

DAlarmThread::DAlarmThread(QObject *parent):QThread(parent)
{
    initFile();
}

DAlarmThread::~DAlarmThread()
{
    if(m_file.isOpen())
    {
        m_file.close();
    }
}

void DAlarmThread::run()
{
    exec();
}

void DAlarmThread::stop()
{
    quit();
}

void DAlarmThread::initFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("AlarmLog");
    QString str_fileName;
    if(!isPathExist(pathName))
    {
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:Alarm"));
        emit openFileError(tr("Failed to create directory:Alarm"));
        str_fileName = QString("Alarmlog") + QString(".log");
    }
    else
    {
        str_fileName = pathName + QString("/") + QString("Alarmlog") + QString(".log");
    }
    m_file.setFileName(str_fileName);
    if(!m_file.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open AlarmLog.log fail"));
        emit openFileError(tr("Open AlarmLog.log fail"));
    }
}

bool DAlarmThread::isPathExist(const QString &curPath)
{
    QDir dir(curPath);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        return dir.mkpath(curPath);
    }
}

void DAlarmThread::writeAlarm(const QString& msg)
{
    QTextStream out(&m_file);
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString str_curDateTime = curDateTime.toString("yyyy-MM-dd  HH:mm:ss");
    QString strTemp = str_curDateTime + tr("  ") + msg + tr("\n");
    out << strTemp;
}

void DAlarmThread::readAlarm()
{
    QStack<QString> strStack;
    QTextStream in(&m_file);
    QString strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTemp = in.readLine();
        strStack.push(strTemp);
        strTemp.clear();
    }
    emit sendAlarmMessage(strStack);
}


