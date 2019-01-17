#include "dhistroythread.h"
#include <QDate>
#include <QDir>
#include <QTextStream>
#include <QStack>
#include <QMessageBox>
//#include <QDebug>


DHistroyThread::DHistroyThread(QObject* parent):QThread(parent)
{
    initFile();
    isFeedUpdate = false;
    isRoUpdate = false;
    isEDIUpdate = false;
    isPresUpdate = false;
    isRoRateUpdate = false;
    isDrainRateUpdate = false;
    isTempUpdate = false;
}

DHistroyThread::~DHistroyThread()
{
    if(m_feedFile.isOpen())
    {
        m_feedFile.close();
    }
    if(m_RoFile.isOpen())
    {
        m_RoFile.close();
    }
    if(m_EDIFile.isOpen())
    {
        m_EDIFile.close();
    }
    if(m_RoRateFile.isOpen())
    {
        m_RoRateFile.close();
    }
    if(m_PresFile.isOpen())
    {
        m_PresFile.close();
    }
    if(m_DrainRateFile.isOpen())
    {
        m_DrainRateFile.close();
    }
    if(m_TemperFile.isOpen())
    {
        m_TemperFile.close();
    }

}

void DHistroyThread::run()
{
    exec();
}

void DHistroyThread::stop()
{
    quit();
}

void DHistroyThread::initFile()
{
    initFeedFile();
    initRoFile();
    initEDIFile();
    initRORateFile();
    initRODrainRateFile();
    initOperationPresFile();
    initTemperatureFile();
}

void DHistroyThread::initFeedFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("Feed.log");
    }
    else
    {
        strFileName = QString("Feed.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_feedFile.setFileName(strFileName);
    if(!m_feedFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open Feed.log fail"));
        emit openFileError(tr("Open Feed.log fail"));
    }
}

void DHistroyThread::initRoFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("RO.log");
    }
    else
    {
        strFileName = QString("RO.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_RoFile.setFileName(strFileName);
    if(!m_RoFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open RO.log fail"));
        emit openFileError(tr("Open RO.log fail"));
    }
}

void DHistroyThread::initEDIFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("EDI.log");
    }
    else
    {
        strFileName = QString("EDI.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_EDIFile.setFileName(strFileName);
    if(!m_EDIFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open EDI.log fail"));
        emit openFileError(tr("Open EDI.log fail"));
    }
}

void DHistroyThread::initOperationPresFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("OperationPres.log");
    }
    else
    {
        strFileName = QString("OperationPres.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_PresFile.setFileName(strFileName);
    if(!m_PresFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open OperationPres.log fail"));
        emit openFileError(tr("Open OperationPres.log fail"));
    }
}

void DHistroyThread::initRORateFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("RORate.log");
    }
    else
    {
        strFileName = QString("RORate.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_RoRateFile.setFileName(strFileName);
    if(!m_RoRateFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open RORate.log fail"));
        emit openFileError(tr("Open RORate.log fail"));
    }
}

void DHistroyThread::initRODrainRateFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("RODrainRate.log");
    }
    else
    {
        strFileName = QString("RODrainRate.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_DrainRateFile.setFileName(strFileName);
    if(!m_DrainRateFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open RODrainRate.log fail"));
        emit openFileError(tr("Open RODrainRate.log fail"));
    }
}

void DHistroyThread::initTemperatureFile()
{
    QString pathName = QDir::currentPath() + QString("/") + QString("HistoricalData");
    QString strFileName;
    if(isPathExist(pathName))
    {
        strFileName = pathName + QString("/") + QString("Temperature.log");
    }
    else
    {
        strFileName = QString("Temperature.log");
        QMessageBox::warning(0, tr("Path Error"), tr("Failed to create directory:HistoricalData"));
        emit openFileError(tr("Failed to create directory:HistoricalData"));
    }
    m_TemperFile.setFileName(strFileName);
    if(!m_TemperFile.open(QIODevice::Append | QIODevice::ReadWrite | QIODevice::Text))
    {
        QMessageBox::warning(0, tr("Log Error"), tr("Open Temperature.log fail"));
        emit openFileError(tr("Open Temperature.log fail"));
    }
}

bool DHistroyThread::isPathExist(const QString &pathName)
{
    QDir dir(pathName);
    if(dir.exists())
    {
        return true;
    }
    else
    {
        return dir.mkpath(pathName);
    }
}

void DHistroyThread::writeHistoricalData(const QString &data, int& index)
{
    switch(index)
    {
    case 0:
        writeFeedData(data);
        break;
    case 1:
        writeRoData(data);
        break;
    case 2:
        writeEDIData(data);
        break;
    case 3:
        writeOperationPresData(data);
        break;
    case 4:
        writeRORateData(data);
        break;
    case 5:
        writeRODrainRateData(data);
        break;
    case 6:
        writeTemperatureData(data);
        break;
    default:
        break;
    }
}

void DHistroyThread::writeFeedData(const QString& date)
{
    QTextStream out(&m_feedFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareFeedData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isFeedUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeRoData(const QString& date)
{
    QTextStream out(&m_RoFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareRoData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isRoUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeEDIData(const QString& date)
{
    QTextStream out(&m_EDIFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareEDIData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isEDIUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeOperationPresData(const QString& date)
{
    QTextStream out(&m_PresFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareOperationPresData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isPresUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeRORateData(const QString& date)
{
    QTextStream out(&m_RoRateFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareRORateData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isRoRateUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeRODrainRateData(const QString& date)
{
    QTextStream out(&m_DrainRateFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
    QString strTemp = str_curDate + QString("  ") + date;
    int offset;
    int spaceNum = 0;
    if(!compareRODrainRateData(date, offset, spaceNum))
    {
        out.seek(offset);
    }
    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isDrainRateUpdate)
    {
        out << strTemp;
    }
}
void DHistroyThread::writeTemperatureData(const QString& data)
{
    QTextStream out(&m_TemperFile);
    QDate curDate = QDate::currentDate();
    QString str_curDate = curDate.toString("yyyy-MM-dd");
//    QString strTemp = str_curDate + QString("  ") + data + QString("\n");
    QString strTemp = str_curDate + QString("  ") + data;
    int offset;
    int spaceNum = 0;
    if(!compareTemperatureData(data, offset, spaceNum))
    {
        out.seek(offset);
    }

    for(int i = 0; i < spaceNum; i++)
    {
        strTemp += QString(" ");
    }
    strTemp += QString("\n");

    if(isTempUpdate)
    {
        out << strTemp;
    }
}

void DHistroyThread::readHistoricalData(int& index)
{
    QStack<QString> strStack;
    QString strTemp;
    QTextStream in;
    switch (index)
    {
    case 0:
        in.setDevice(&m_feedFile);
        break;
    case 1:
        in.setDevice(&m_RoFile);
        break;
    case 2:
        in.setDevice(&m_EDIFile);
        break;
    case 3:
        in.setDevice(&m_PresFile);
        break;
    case 4:
        in.setDevice(&m_RoRateFile);
        break;
    case 5:
        in.setDevice(&m_DrainRateFile);
        break;
    case 6:
        in.setDevice(&m_TemperFile);
        break;
    default:
        return;
    }
    in.seek(0);
    while(!in.atEnd())
    {
        strTemp = in.readLine();
        strStack.push(strTemp);
        strTemp.clear();
    }
    emit sendHistoricalData(strStack);
}

bool DHistroyThread::compareFeedData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_feedFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isFeedUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toInt() > strDate.toInt())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }
                offset = m_feedFile.size() - strTemp.length() - 2;
                isFeedUpdate = true;
                return false;
            }
            else
            {
                isFeedUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isFeedUpdate = true;
        return true;
    }
}
bool DHistroyThread::compareRoData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_RoFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isRoUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toFloat() > strDate.toFloat())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_RoFile.size() - strTemp.length() - 2;
                isRoUpdate = true;
                return false;
            }
            else
            {
                isRoUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isRoUpdate = true;
        return true;
    }
}
bool DHistroyThread::compareEDIData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_EDIFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isEDIUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toFloat() < strDate.toFloat())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_EDIFile.size() - strTemp.length() - 2;
                isEDIUpdate = true;
                return false;
            }
            else
            {
                isEDIUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isEDIUpdate = true;
        return true;
    }
}
bool DHistroyThread::compareOperationPresData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_PresFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isPresUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toFloat() < strDate.toFloat())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_PresFile.size() - strTemp.length() - 2;
                isPresUpdate = true;
                return false;
            }
            else
            {
                isPresUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isPresUpdate = true;
        return true;
    }
}
bool DHistroyThread::compareRORateData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_RoRateFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isRoRateUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toInt() < strDate.toInt())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_RoRateFile.size() - strTemp.length() - 2;
                isRoRateUpdate = true;
                return false;
            }
            else
            {
                isRoRateUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isRoRateUpdate = true;
        return true;
    }

}
bool DHistroyThread::compareRODrainRateData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_DrainRateFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isDrainRateUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toInt() < strDate.toInt())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_DrainRateFile.size() - strTemp.length() - 2;
                isDrainRateUpdate = true;
                return false;
            }
            else
            {
                isDrainRateUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isDrainRateUpdate = true;
        return true;
    }
}
bool DHistroyThread::compareTemperatureData(const QString &curDate, int& offset, int& spaceNum)
{
    QString pattern = "\\d{4}[-]{1}\\d{2}[-]{1}\\d{2}[\\s]+[\\d]+[.]?[\\d]?[\\s]?[\\s]?[\\s]?[\\s]?";
    QRegExp regExp(pattern);

    QTextStream in(&m_TemperFile);
    QString strTempAll, strTemp;
    in.seek(0);
    while(!in.atEnd())
    {
        strTempAll.clear();
        strTempAll = in.readLine();
    }
    int pos = regExp.indexIn(strTempAll);
    if(pos > -1)
    {
        strTemp = regExp.cap(0);
        QString strDate = strTemp.mid(0, 10);
        QDate date = QDate::fromString(strDate, QString("yyyy-MM-dd"));
        if(date != QDate::currentDate())
        {
            isTempUpdate = true;
            return true;
        }
        else
        {
            QString strDate = curDate;
            QString preDate = strTemp.mid(12, strTemp.length() - 12);
            if(preDate.toFloat() < strDate.toFloat())
            {
                if(preDate.length() > strDate.length())
                {
                    spaceNum = preDate.length() - strDate.length();
                }

                offset = m_TemperFile.size() - strTemp.length() - 2;
                isTempUpdate = true;
                return false;
            }
            else
            {
                isTempUpdate = false;
                return true;
            }
        }
    }
    else
    {
        isTempUpdate = true;
        return true;
    }
}

