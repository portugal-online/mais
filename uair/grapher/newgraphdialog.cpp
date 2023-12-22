#include "newgraphdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QPushButton>
#include "registeredgraphmodel.h"
#include <QHeaderView>
#include <QDialogButtonBox>

NewGraphDialog::NewGraphDialog(const std::vector<RegisteredGraph> &graphs): QDialog(), m_stockgraphs(graphs)
{
    QVBoxLayout * l = new QVBoxLayout();
    setLayout(l);


    QHBoxLayout *n = new QHBoxLayout();
    n->addWidget( new QLabel("Name:") );
    m_name = new QLineEdit();
    m_name->setText("Graph 1");
    n->addWidget( m_name );
    l->addLayout(n);

    m_ltable = new QTableView();
    m_rtable = new QTableView();

    QVBoxLayout *buttons =  new QVBoxLayout();
    QHBoxLayout *tablebut =  new QHBoxLayout();

    tablebut->addWidget(m_ltable);
    tablebut->addLayout(buttons);
    tablebut->addWidget(m_rtable);

    left = new QPushButton("<");
    right = new QPushButton(">");

    buttons->addWidget(left);
    buttons->addWidget(right);

    left->setEnabled(false);
    right->setEnabled(false);

    m_lmodel.update(graphs);

    m_ltable->setModel(&m_lmodel);
    m_rtable->setModel(&m_rmodel);

    m_ltable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_rtable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(m_ltable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this,
            &NewGraphDialog::leftTableRowChanged);

    connect(m_rtable->selectionModel(), &QItemSelectionModel::currentRowChanged,
            this,
            &NewGraphDialog::rightTableRowChanged);

    l->addLayout(tablebut);

    connect(right, &QPushButton::clicked,
            this,
            &NewGraphDialog::addGraph);
    connect(left, &QPushButton::clicked,
            this,
            &NewGraphDialog::removeGraph);

    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    l->addWidget(bb);

    connect(bb, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bb, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void NewGraphDialog::leftTableRowChanged(QModelIndex current,QModelIndex prev)
{
    right->setEnabled(true);
}

void NewGraphDialog::rightTableRowChanged(QModelIndex current,QModelIndex prev)
{
    left->setEnabled(true);
}

void NewGraphDialog::addGraph()
{
    QModelIndexList items = m_ltable->selectionModel()->selectedIndexes();
    for (auto i: items) {
        RegisteredGraph g = m_lmodel.at(i.row());
        m_lmodel.remove(i.row());
        m_rmodel.append(g);
    }
}

void NewGraphDialog::removeGraph()
{
    QModelIndexList items = m_rtable->selectionModel()->selectedIndexes();
    for (auto i: items) {
        RegisteredGraph g = m_rmodel.at(i.row());
        m_rmodel.remove(i.row());
        m_lmodel.append(g);
    }
}

const std::vector<RegisteredGraph> &NewGraphDialog::get() const
{
    return m_rmodel.get();
}

QString NewGraphDialog::name()
{
    return m_name->displayText();
}

//~RegisteredGraphModel
