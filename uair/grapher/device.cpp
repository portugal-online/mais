#include "device.h"
#include <QDebug>

Dataset *Device::generate(const QString &field,
                          const QString &validity,
                          const Dataunits &units,
                          std::function<bool(const QVariant &v)> validator
                         )
{
    Dataset *d = new Dataset(field, units);

    for (auto i: m_dataset) {

        QVariant f = (*i).get(field);

        if (validity.size()) {
            int v = (*i).get(validity).toInt();
            if (!v) {
                i++;
                continue;
            }
        }
        if (!validator(f)) {
            i++;
            continue;
        }
        d->push_back( DatasetEntry( (*i).get("epoch").toInt(), f.toDouble()) );
    }
    d->sort();

    return d;

}
