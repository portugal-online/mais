#ifndef DATASET_H__
#define DATASET_H__

#include <QString>
#include <vector>
#include <QMap>
#include <QVariant>
#include <QColor>
#include <limits>
#include "datasetentry.h"

class Dataset: public std::vector<DatasetEntry*>
{
public:
    using std::vector<DatasetEntry*>::iterator;
    using std::vector<DatasetEntry*>::const_iterator;

    struct
    {
        bool operator()(const DatasetEntry *a, const DatasetEntry *b) const{
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

