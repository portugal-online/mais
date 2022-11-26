#include "devicedatasetentry.h"

#include <QDebug>
#include <QDateTime>

#include "payload.h"

void DeviceDatasetEntry::parseFromBinary(const QString &date,
                                   int sf,
                                   const QString &payload_b64)
{

    QDateTime datetime = QDateTime::fromString(date,Qt::ISODate);
    if (!datetime.isValid()) {
        qDebug()<<"Cannot convert"<<date;
        throw;
    }
    QByteArray bpayload = QByteArray::fromBase64(payload_b64.toLocal8Bit());

    const uint8_t *raw = (const uint8_t*)bpayload.constData();

    if (bpayload.size()<2) {
        qDebug()<<"Invalid payload size"<<bpayload.size();
        return ;
    }

    struct generic_payload *gen = (struct generic_payload*)raw;

    if (gen->payload_type!=0) {
        qDebug()<<"Cannot handle payload type"<<gen->payload_type;
        return ;
    }

    struct payload_type0_debug *p = (struct payload_type0_debug*)raw;

    bool extended = false;

    switch (bpayload.size()) {
    case sizeof(payload_type0):
        break;
    case sizeof(payload_type0_debug):
        extended = true;
        break;
    default:
        qDebug()<<"Invalid payload 0 size"<<bpayload.size();
        return;
        break;
    }

    set("date", date);
    set("epoch", datetime.toSecsSinceEpoch());
    set("sf", sf);
    //set("payload", l[index++]);
    set("health_external", ( p->p0.health_ext_temp_hum ? true:false ));
    set("health_internal", ( p->p0.health_int_temp_hum ? true:false ));
    set("health_oaq",( p->p0.health_oaq ? true:false ));
    set("health_mic",( p->p0.health_microphone ? true:false ));

    uint16_t max_oaq, epa_oaq;
    uint8_t max_sound, avg_sound;

    max_oaq = (uint16_t)p->p0.max_oaq_lsb | (((uint16_t)p->p0.max_oaq_msb)<<8);
    epa_oaq = (uint16_t)p->p0.epa_oaq_lsb | (((uint16_t)p->p0.epa_oaq_msb)<<8);

    max_sound = p->p0.max_sound_level_lsb | (p->p0.max_sound_level_msb<<4);
    avg_sound = p->p0.avg_sound_level_lsb | (p->p0.avg_sound_level_msb<<4);

    set("max_oaq", max_oaq);
    set("epa_oaq", epa_oaq);
    set("max_sound", max_sound);
    set("avg_sound", avg_sound);

#define TEMP(x) ((float)(x-47)/4.0)

    set("max_int_hum", p->p0.max_int_hum);
    set("max_int_temp", TEMP(p->p0.max_int_temp));
    set("avg_ext_temp", TEMP(p->p0.avg_ext_temp));
    set("avg_ext_hum", p->p0.avg_ext_hum);
    if (extended)
        set("battery", be16toh(p->battery));

}
