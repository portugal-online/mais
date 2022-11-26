#ifndef REGISTEREDGRAPH_H__
#define REGISTEREDGRAPH_H__

#include <QString>
class Device;
class Dataset;

struct RegisteredGraph {
    RegisteredGraph();
    RegisteredGraph(const QString &name, Device *dev, Dataset *dataset): m_name(name), m_dev(dev), m_dataset(dataset)
    {
    }

    Device *device() const { return m_dev; }
    Dataset *dataset() const { return m_dataset; }

    QString m_name;
    Device *m_dev;
    Dataset *m_dataset;
    bool operator<(const RegisteredGraph&other) const { return m_name<other.m_name; }
};


#endif
