#include "dstringlist.h"
#include <QDebug>
#include <QObject>

DStringList::DStringList()
{
    m_str.clear();
}

void DStringList::removeFixItem(QString& strTemp)
{
    iterator it = begin();
    while(it != end())
    {
        if(*it == strTemp)
        {
            erase(it);
            break;
        }
        else
        {
            it++;
        }
    }
}

void DStringList::listStringClear()
{
    m_str.clear();
}

QString& DStringList::listToString()
{
    m_str.clear();
    if(count() < 7)
    {
        for(int i = 0; i < count(); i++)
        {
            if(m_str.length() == 0)
            {
                m_str += at(i);
            }
            else
            {
                m_str += QString("      ");
                m_str += at(i);
            }
        }
    }
    else
    {
        m_str += QString("报警信息数量过多");
    }
    return m_str;
}

bool DStringList::findFixItem(QString &strTemp)
{
    iterator it = begin();
    while(it != end())
    {
        if(*it == strTemp)
        {
//            qDebug() << QObject::tr("return true");
            return true;
            break;
        }
        else
        {
            it++;
        }
    }
    return false;
}
