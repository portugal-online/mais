#ifndef NEWGRAPHDIALOG_H__
#define NEWGRAPHDIALOG_H__

#include <QDialog>
#include "registeredgraph.h"
#include "registeredgraphmodel.h"

class QPushButton;
class QTableView;
class QLineEdit;

class NewGraphDialog: public QDialog
{
public:
    NewGraphDialog(const std::vector<RegisteredGraph> &graphs);

    const std::vector<RegisteredGraph> m_stockgraphs;
    const std::vector<RegisteredGraph> m_showgraphs;

    RegisteredGraphModel m_lmodel, m_rmodel;

    void leftTableRowChanged(QModelIndex current,QModelIndex prev);
    void rightTableRowChanged(QModelIndex current,QModelIndex prev);

    void addGraph();
    void removeGraph();


    const std::vector<RegisteredGraph> &get() const;
    QString name();

    QPushButton *left;
    QPushButton *right;
    QTableView *m_ltable;
    QTableView *m_rtable;
    QLineEdit *m_name;
};

#endif
