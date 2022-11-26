#ifndef DEVICE_H__
#define DEVICE_H__

#include "devicedataset.h"
#include "dataset.h"
#include <unordered_map>
#include <QString>

struct Device
{
    Device(const QString &dname, const QColor &color): m_name(dname), m_color(color) {}

    //void parseFromFile(const QString &file);
    const DeviceDataset &dataset() const { return m_dataset; }
    const QColor &color() const { return m_color; }
    const QString &name() const { return m_name; }
    void addDatasetEntry(DeviceDatasetEntry *d) { m_dataset.push_back(d); }
    void sort() {
        m_dataset.sort();
    }
    static bool alwaysValid(const QVariant&){ return true; }
    Dataset *generate(const QString &field,
                      const QString &validity,
                      const Dataunits &units,
                      std::function<bool(const QVariant &v)> valid  = alwaysValid
                     );
private:
    QString m_name;
    QColor m_color;
    DeviceDataset m_dataset;
    std::unordered_map<std::string, Dataset*> m_valuedatasets;
};

#endif
