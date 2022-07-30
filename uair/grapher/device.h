#ifndef DEVICE_H__
#define DEVICE_H__

#include "dataset.h"

struct Device
{
    Device(const QString &dname, const QColor &color): m_name(dname), m_color(color) {}

    //void parseFromFile(const QString &file);
    const Dataset &dataset() const { return m_dataset; }
    const QColor &color() const { return m_color; }
    const QString &name() const { return m_name; }
    void addDatasetEntry(DatasetEntry *d) { m_dataset.push_back(d); }
    void sort() {
        m_dataset.sort();
    }
private:
    QString m_name;
    QColor m_color;
    Dataset m_dataset;
};

#endif
