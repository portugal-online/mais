#ifndef GRAPHER_H__
#define GRAPHER_H__

#include <QMainWindow>
#include <QVariant>
#include <QJsonArray>
#include <QMap>
#include <QHash>
#include <QString>

#include "devicedataset.h"
#include "device.h"
#include "registeredgraph.h"

class QTabWidget;
class DeviceCollection;
class Device;

#include <dataset.h>

struct DataunitsHasher
{
    size_t operator()(const Dataunits& p) const
    {
        return (unsigned)p.max() ^ (unsigned)p.min();
    }
};

class DatasetCollection
{
public:
    typedef std::pair<Device*,Dataset*> ddpair;
    void append(Device *d, Dataset *set)
    {
        m_collection.push_back( ddpair(d,set) );
        m_per_unit[ set->units() ].push_back( set );
    }

    void append(const ddpair &pair)
    {
        m_collection.push_back( pair );
        m_per_unit[ pair.second->units() ].push_back( pair.second );
    }
    unsigned unitsCount() const { return m_per_unit.keys().size(); }
    const QMap<Dataunits, DatasetPtrList> &perUnit() const  { return m_per_unit; }

    int minTime() const {
        int min = std::numeric_limits<int>::max();

        for (auto i: m_collection)
        {
            int thismin =  i.second->minTime();
            if (min>thismin)
                min=thismin;
        }
        return min;
    }
    int maxTime() const {
        int max = std::numeric_limits<int>::min();

        for (auto i: m_collection)
        {
            int thismax = i.second->maxTime();
            if (max<thismax)
                max=thismax;
        }
        return max;
    }

    std::vector<ddpair>::const_iterator begin() const { return m_collection.begin(); }
    std::vector<ddpair>::const_iterator end() const { return m_collection.end(); }
#if 0
    void setDescription(const QString &s) { m_description=s; }
    void setShortDescription(const QString &s) { m_short_description=s; }

    const QString &description() const { return m_description; }
    const QString &shortDescription() const { return m_short_description; }

    QString m_description;
    QString m_short_description;
#endif
private:
    std::vector<ddpair> m_collection;
    QMap<Dataunits, DatasetPtrList> m_per_unit;
};

class Grapher: public QMainWindow
{
    
public:
    void setupui();

    void loadConfigFile(const QString &name);
protected:
    void generateCharts(const QString &name);

    void onOpenFile();
    void onNewGraph();
    void parseDevices(QJsonArray devices);


    void createChart(const QString &title, const DatasetCollection &items, bool range_color=true);

    void initGraph(Device*,
                   DatasetCollection &col,
                   const QString &name,
                   const QString &health,
                   const Dataunits &unit,
                   std::function<bool(const QVariant &v)> valid  = &Device::alwaysValid
                  );


    static bool forcevalid(const QVariant&) { return true; }
protected:
    void registerGraph(const QString &name, Device *dev, Dataset *dataset)
    {
        m_graphs.push_back(RegisteredGraph(name,dev,dataset));
    }

private:
    QTabWidget *m_tabwidget;
    QAction *newg;
    QMap<QString,QString> m_devicenames;
    std::vector<RegisteredGraph> m_graphs;
};

#endif
