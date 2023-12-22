#ifndef DEVICEDATASETENTRY_H__
#define DEVICEDATASETENTRY_H__

#include <QMap>
#include <QString>
#include <QVariant>

struct DeviceDatasetEntry
{
    void set(const QString &name, const QVariant &value)
    {
        m_properties[name] = value;
    }
    const QVariant get(const QString &name) const
    {
        return m_properties[name];
    }

    void parseFromCSV(const QString &csv);

    void parseFromBinary(const QString &date, int sf,
                         const QString &payload_b64);
private:
    QMap<QString,QVariant> m_properties;
};

#endif
