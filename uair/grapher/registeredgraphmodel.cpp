#include "registeredgraphmodel.h"


RegisteredGraphModel::RegisteredGraphModel(QObject *parent): QAbstractTableModel(parent)
{
}

int RegisteredGraphModel::rowCount(const QModelIndex &parent) const
{
    return m_graphs.size();
}
int RegisteredGraphModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant RegisteredGraphModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();

    if (row>=(int)m_graphs.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole  || role == Qt::EditRole) {

        int col = index.column();
        switch (col) {
        case 0:
            return m_graphs[row].m_name;
        default:
            return QVariant();
        }
    } else {
        return QVariant();
    }

}

Qt::ItemFlags RegisteredGraphModel::flags(const QModelIndex &index) const
{
    return QAbstractTableModel::flags(index);
}


QVariant RegisteredGraphModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

const RegisteredGraph &RegisteredGraphModel::at(int index)
{
    return m_graphs[index];
}

void RegisteredGraphModel::remove(int index)
{
    beginResetModel();
    m_graphs.erase( m_graphs.begin() + index);
    endResetModel();
}

void RegisteredGraphModel::append(const RegisteredGraph&g)
{
    m_graphs.push_back(g);
    beginResetModel();
    std::sort(m_graphs.begin(), m_graphs.end());
    endResetModel();
}

void RegisteredGraphModel::update(const std::vector<RegisteredGraph> &list)
{
    beginResetModel();
    m_graphs = list;
    endResetModel();
}
