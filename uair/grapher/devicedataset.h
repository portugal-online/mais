#ifndef DEVICEDATASET_H__
#define DEVICEDATASET_H__

#include <QString>
#include <vector>
#include <QMap>
#include <QVariant>
#include <QColor>
#include <limits>
#include "devicedatasetentry.h"

class DeviceDataset: public std::vector<DeviceDatasetEntry*>
{
public:
    using std::vector<DeviceDatasetEntry*>::iterator;
    using std::vector<DeviceDatasetEntry*>::const_iterator;

    struct
    {
        bool operator()(const DeviceDatasetEntry *a, const DeviceDatasetEntry *b) const{
            return a->get("epoch").toInt() < b->get("epoch").toInt();
        }
    } timeSorter;

    void sort()
    {
        std::sort(begin(), end(), timeSorter);
    }

    float minF(const QString &field) const
    {
        float min = std::numeric_limits<float>::max();
        for (auto i= begin(); i!=end(); i++) {
            float v = (*i)->get(field).toFloat();
            if (v<min)
                min=v;
        }
        return min;
    }
    float maxF(const QString &field) const
    {
        float max = std::numeric_limits<float>::min();
        for (auto i= begin(); i!=end(); i++) {
            int v = (*i)->get(field).toFloat();
            if (max<v)
                max=v;
        }
        return max;
    }

    int min(const QString &field) const {
        int min = std::numeric_limits<int>::max();
        for (auto i= begin(); i!=end(); i++) {
            int v = (*i)->get(field).toInt();
            if (min>v)
                min=v;
        }
        return min;
    }

    int max(const QString &field) const
    {
        int max = std::numeric_limits<int>::min();
        for (auto i= begin(); i!=end(); i++) {
            int v = (*i)->get(field).toInt();
            if (v>max)
                max=v;
        }
        return max;
    }
};

#endif

