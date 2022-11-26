#ifndef REGISTEREDGRAPHMODEL_H__
#define REGISTEREDGRAPHMODEL_H__

#include "registeredgraph.h"
#include <QAbstractTableModel>

class RegisteredGraphModel: public QAbstractTableModel
{
public:
    RegisteredGraphModel(QObject *parent=NULL);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    const RegisteredGraph &at(int index);
    void remove(int index);
    void append(const RegisteredGraph&);

    const std::vector<RegisteredGraph> &get() const { return m_graphs; }

public:
    void update(const std::vector<RegisteredGraph> &);
private:
    std::vector<RegisteredGraph> m_graphs;
};

#endif
