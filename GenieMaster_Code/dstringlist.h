#ifndef DSTRINGLIST_H
#define DSTRINGLIST_H

#include <QStringList>

class DStringList : public QStringList
{
public:
    DStringList();

public:
    void removeFixItem(QString&);
    void listStringClear();
    QString& listToString();
    bool findFixItem(QString&);

private:
    QString m_str;
};

#endif // DSTRINGLIST_H
