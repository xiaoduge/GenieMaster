#include "dtablemodel.h"
#include <QFont>
#include <QBrush>

DTableModel::DTableModel(QObject *parent):QAbstractTableModel(parent)
{

}
int DTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_tableModelVector.count();
}

int DTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_columnCount;
}

QVariant DTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return int(Qt::AlignCenter);
    case Qt::FontRole:
        return QFont("Arial", 12, QFont::Bold);
    case Qt::BackgroundRole:
    {
        if(index.row() == 0)
        {
            return QBrush(QColor(230, 230, 230));
        }
        else if(index.row()%2 == 0)
        {
            return QBrush(QColor(240, 240, 240));
        }
        else
        {
            return QBrush(Qt::white);
        }
    }
    case Qt::DisplayRole:
    {
        int nCol = index.column();
        int nRow = index.row();
        QStringList rowList = m_tableModelVector.at(nRow);
        if(rowList.size() > nCol)
        {
            return rowList.at(nCol);
        }
        else
        {
            return QVariant();
        }
    }
    default:
        return QVariant();
    }
}

QVariant DTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();
    if(orientation == Qt::Horizontal)
    {
        return QString("%1").arg(section);
    }
    else
    {
        return QString("%1").arg(section);
    }
}

void DTableModel::setTableModelVector(QVector<QStringList> &vector)
{
    m_tableModelVector = vector;
    endResetModel();
}

void DTableModel::setTableModelColumnCount(int i)
{
    m_columnCount = i;
}
