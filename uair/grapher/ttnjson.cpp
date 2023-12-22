#include <QFile>
#include <QJsonArray>

#include "ttnjson.h"

#include "devicecollection.h"
#include "devicedataset.h"
#include "jsonutils.h"

TTNJson::TTNJson()
{
}

bool TTNJson::load(const QString &filename)
{
    m_doc = loadJSONFromFile(filename);
    return !m_doc.isNull();
}

DeviceCollection *TTNJson::createCollection(const std::vector<QColor> &colormap)
{
    QJsonArray items = JSONfindArray(m_doc.object(), "Items");
    unsigned int colorindex = 0;

    DeviceCollection *dc = new DeviceCollection();

    for (auto i: items)
    {
        QJsonObject io = i.toObject();

        QJsonObject uplink = JSONfindObject(io,"device_data/M/uplink_message/M");

        QJsonObject devids = JSONfindObject(io,"device_data/M/end_device_ids/M");

        QVariant payload = JSONfindValue(uplink, "frm_payload/S");

        QVariant at = JSONfindValue(io, "device_data/M/received_at/S");

        QVariant dev = JSONfindValue(devids,"dev_eui/S");

        if (!dev.toString().startsWith("0080"))
            continue;
        QVariant sf = JSONfindValue(uplink, "settings/M/data_rate/M/lora/M/spreading_factor/N");

        Device *d = dc->find(dev.toString());

        if (nullptr==d)
        {
            d = new Device(dev.toString(), colormap[colorindex++]);
            if (colorindex>=colormap.size())
                colorindex = 0;
            dc->push_back(d);
            qDebug()<<"Created new device"<<dev.toString();
        }

        DeviceDatasetEntry *e = new DeviceDatasetEntry();

        e->parseFromBinary(at.toString(),
                           sf.toInt(),
                           payload.toString());
        d->addDatasetEntry(e);
//        qDebug()<<"Added"<<dev.toString()<<"dataset"<<e->get("epoch").toString();
    }
    qDebug()<<"Loaded "<<dc->size()<<"devices";
    dc->sort();
    return dc;
};

