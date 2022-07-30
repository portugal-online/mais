#ifndef DEVICECOLLECTION_H__
#define DEVICECOLLECTION_H__

#include "device.h"

class DeviceCollection: public std::vector<Device*>
{
public:
    DeviceCollection() {
    }
    using std::vector<Device*>::iterator;
    using std::vector<Device*>::const_iterator;

    void sort()
    {
        for (auto i: *this) {
            i->sort();
        }
    }

    Device *find(const QString &id) {
        for (auto i: *this) {
            if (i->name()==id)
                return i;
        }
        return nullptr;
    }

    void computeTime() {
        maxtime = std::numeric_limits<int>::min();
        mintime = std::numeric_limits<int>::max();
        for (auto i=begin(); i!=end();i++) {
            int dmax = (*i)->dataset().max("epoch");
            int dmin = (*i)->dataset().min("epoch");
            if (maxtime<dmax)
                maxtime = dmax;
            if (mintime>dmin)
                mintime = dmin;
        }
    }
    int mintime;
    int maxtime;

    float minF(const QString &name)
    {
        float min = std::numeric_limits<float>::max();
        for (auto i=begin(); i != end(); i++) {
            float dmin = (*i)->dataset().minF(name);
            if (min>dmin)
                min=dmin;
        }
        return min;
    }
    float maxF(const QString &name)
    {
        float max = std::numeric_limits<float>::min();
        for (auto i=begin(); i != end(); i++) {
            float dmax = (*i)->dataset().maxF(name);
            if (max<dmax)
                max=dmax;
        }
        return max;

    }
    int min(const QString &name);
    int max(const QString &name);

};

#endif
