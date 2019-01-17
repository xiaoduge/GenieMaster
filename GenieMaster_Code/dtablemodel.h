#ifndef DTABLEMODEL_H
#define DTABLEMODEL_H

#include <QAbstractTableModel>

class DTableModel : public QAbstractTableModel
{
public:
    DTableModel(QObject *parent = Q_NULLPTR);

public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setTableModelVector(QVector<QStringList> &vector);
    void setTableModelColumnCount(int);

private:
    QVector<QStringList> m_tableModelVector;
    int m_columnCount;
};

#endif // DTABLEMODEL_H
