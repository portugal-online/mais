#ifndef GRAPHER_H__
#define GRAPHER_H__

#include <QMainWindow>
#include <QVariant>
#include <QJsonArray>
#include <QMap>
#include <QString>

#include "dataset.h"

class QTabWidget;
class DeviceCollection;

class Grapher: public QMainWindow
{
    
public:
    void setupui();

    void loadConfigFile(const QString &name);
protected:
    void generateCharts(const QString &name);

    void onOpenFile();
    void parseDevices(QJsonArray devices);


    void createChart(DeviceCollection *dc,
                     const QString &name,
                     const QString &label,
                     const QString &field,
                     const QString &validity="",
                     std::function<bool(const QVariant&)> validator = &forcevalid);
    static bool forcevalid(const QVariant&) { return true; }
private:
    QTabWidget *m_tabwidget;
    QMap<QString,QString> m_devicenames;
};

#endif
