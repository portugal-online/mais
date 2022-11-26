#ifndef DATASET_H__
#define DATASET_H__

#include <QString>
#include <vector>
#include <QMap>
#include <QVariant>
#include <QColor>
#include <limits>
#include <QDebug>

struct DatasetEntry
{
    DatasetEntry();
    DatasetEntry(unsigned long epoch, double value) : m_epoch(epoch), m_value(value) {}

    uint64_t epoch() const { return m_epoch; };
    double value() const { return m_value; }
private:
    unsigned long m_epoch;
    double m_value;
};

struct Dataunits
{
    double max() const  { return m_max; }
    double min() const  { return m_min; }
    const QString &description() const { return m_description; };

    double m_min, m_max;
    std::string format(double value) const;
    bool operator==(const Dataunits &other) const;
    bool operator<(const Dataunits &other) const {
        return m_description < other.m_description;
    }
    QString m_description;
    Dataunits(const QString &d): m_description(d) {}
};

class Dataset: public std::vector<DatasetEntry>
{
public:
    Dataset( const QString &field, const Dataunits &units): m_field(field), m_units(units){}

    using std::vector<DatasetEntry>::iterator;
    using std::vector<DatasetEntry>::const_iterator;

    struct
    {
        bool operator()(const DatasetEntry &a, const DatasetEntry &b) const{
            return a.epoch() < b.epoch();
        }
    } timeSorter;

    void sort()
    {
        std::sort(begin(), end(), timeSorter);
    }
    const Dataunits &units() const { return m_units; }

    int minTime() const {
        return begin()->epoch();
    }
    int maxTime() const
    {
        // Assume ordered
        return (*this)[ size()-1 ].epoch();
    }

    std::pair<double, double> getMinMax(unsigned long start_epoch, unsigned long end_epoch) const
    {
        std::pair<double, double> minmax;
        minmax.first = std::numeric_limits<double>::max();
        minmax.second = std::numeric_limits<double>::min();
        for (auto i=begin(); i!=end(); i++) {
            if ((i->epoch()<start_epoch) || (i->epoch()>end_epoch))
                continue;
            minmax.first = std::min( minmax.first, i->value() );
            minmax.second = std::max( minmax.second, i->value() );
        }
        return minmax;
    }

    const QString &field() const { return m_field; }

    QString m_field;
    Dataunits m_units;
};

class DatasetPtrList: public std::vector<Dataset*>
{
};

#endif

