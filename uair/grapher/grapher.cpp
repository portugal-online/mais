#include "grapher.h"

#include <QTabWidget>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QLineSeries>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <endian.h>
#include <QMenu>
#include <QMenuBar>
#include <QFileDialog>

#include "ttnjson.h"
#include "devicecollection.h"
#include "payload.h"
#include "jsonutils.h"

using namespace QtCharts;

#define MIN(x,y) ((x)<(y)?(x):(y))

void Grapher::setupui()
{
    QWidget *wmain = new QWidget(this);
    resize(1280, 800);

    setCentralWidget(wmain);
    m_tabwidget = new QTabWidget();

    // Main tab
    QWidget *maintab = new QWidget();
    m_tabwidget->addTab(maintab, "Main");



    QVBoxLayout *l = new QVBoxLayout();

    wmain->setLayout(l);
    l->addWidget(m_tabwidget);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QAction *open = new QAction(tr("&Open..."));
    connect(open, &QAction::triggered, this, &Grapher::onOpenFile);
    fileMenu->addAction(open);
}

void Grapher::parseDevices(QJsonArray devices)
{
    for (auto d: devices)
    {
        QString eui = JSONfindValue(d.toObject(), "eui").toString();
        QString name = JSONfindValue(d.toObject(), "name").toString();
        m_devicenames[eui] = name;
    }
}

void Grapher::loadConfigFile(const QString&name)
{
    QJsonDocument doc = loadJSONFromFile(name);
    if (!doc.isNull())
    {

        QJsonArray devices = JSONfindArray(doc.object(), "Devices");
        QString datafile = JSONfindValue(doc.object(), "Datafile").toString();

        QFileInfo dfInfo(datafile);

        if (dfInfo.isRelative()) {
            QFileInfo fileInfo(name);
            QString path = fileInfo.canonicalPath();
            fileInfo = QFileInfo(path, datafile);
            datafile = fileInfo.absoluteFilePath();
        }

        parseDevices(devices);
        qDebug()<<"Processing data file"<<datafile;

        generateCharts(datafile);
    }
}


void Grapher::onOpenFile()
{
    QFileDialog dialog(this, tr("Open config file"), "",
                       "JSON files (*.json)");
    if (dialog.exec()) {
        QStringList fileNames = dialog.selectedFiles();
        if (fileNames.length()==1)
            loadConfigFile(fileNames[0]);

    }
}

bool validHumidity(const QVariant &v)
{
    int h = v.toInt();
    return h>=0  && h<=100;
}

bool validBattery(const QVariant &v)
{
    int h = v.toInt();
    return h>=1800;
}



void Grapher::generateCharts(const QString &file)
{
    TTNJson tj;
    tj.load(file);

    std::vector<QColor> colors;
    colors.push_back(QColor(0,0,128,128));
    colors.push_back(QColor(0,128,0,128));
    colors.push_back(QColor(128,0,0,128));
    colors.push_back(QColor(128,128,0,128));
    colors.push_back(QColor(128,0,128,128));
    colors.push_back(QColor(0,128,128,128));

    DeviceCollection *dc = tj.createCollection(colors);

    createChart(dc, "Max. Int. Hum", "Maximum internal humidity (%)", "max_int_hum", "health_internal", &validHumidity);
    createChart(dc, "Max. Int. Temp", "Maximum internal temp (C)", "max_int_temp", "health_internal");

    createChart(dc, "Max. OAQ", "Max. OAQ", "max_oaq", "health_oaq");
    createChart(dc, "EPA OAQ", "EPA OAQ", "epa_oaq", "health_oaq");
    createChart(dc, "Max. Sound", "Max sound", "max_sound", "health_mic");
    createChart(dc, "Avg. Sound", "Average sound", "avg_sound", "health_mic");
    createChart(dc, "Avg. Ext. Temp", "Average external temp (C)", "avg_ext_temp", "health_external");
    createChart(dc, "Avg. Ext. Hum", "Average external humidity (%)", "avg_ext_hum", "health_external", &validHumidity);
    createChart(dc, "Battery", "Battery (mV)", "battery", "", &validBattery);

}


class GrapherCustomChartView: public QChartView
{
public:
    GrapherCustomChartView(QChart *chart): QChartView(chart)
    {
    }
    bool viewportEvent(QEvent *event) override
    {
     //   qDebug()<<"Viewport event";

        return QChartView::viewportEvent(event);
    }
};


void Grapher::createChart(DeviceCollection *dc,
                          const QString &name,
                          const QString &label,
                          const QString &field,
                          const QString &validity,
                          std::function<bool(const QVariant&)> validator
                         )
{
    QChart *chart = new QChart();
    //chart->legend()->hide();
    chart->setTitle(name);


    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    //axisY->setLinePenColor(series->pen().color());
    axisY->setTickInterval(10);
    axisY->setTickCount(21);
    axisY->setTitleText(label);
    chart->addAxis(axisY, Qt::AlignLeft);


    QColor pen(0,0,128,128);

    //axisX->setMax(100);
    axisX->setFormat("dd-MM-yyyy<br/>h:mm");



    QChartView *chartView = new GrapherCustomChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    chartView->setRubberBand(QChartView::HorizontalRubberBand);

    m_tabwidget->addTab(chartView, name);

    dc->computeTime();

#if 1
    int mintime  = dc->mintime;
    int maxtime  = dc->maxtime;

    qDebug()<<"Min time"<<mintime;
    qDebug()<<"Max time"<<maxtime;
    QDateTime dtmax;
    dtmax.setSecsSinceEpoch(maxtime);
    QDateTime dtmin;
    dtmin.setSecsSinceEpoch(mintime);

    axisX->setMax( dtmax );//maxtime - mintime );
    axisX->setMin( dtmin );//maxtime - mintime );
#endif

    float min = std::numeric_limits<float>::max();
    float max = std::numeric_limits<float>::min();

    for (auto d: *dc)
    {
        QLineSeries *series = new QLineSeries();
        chart->addSeries(series);
        series->setPen(QPen(d->color(), 3));
        series->setPointsVisible();
        series->attachAxis(axisY);

        //chart->setAxisX(axisX, series);

        Dataset::const_iterator i = d->dataset().begin();
        //
        QString displayname = d->name();
        if (m_devicenames.find(d->name()) != m_devicenames.end())
        {
            displayname = m_devicenames[ d->name() ];
        }
        series->setName( displayname );

        while (i != d->dataset().end())
        {
            if (validity.size()) {
                int v = (*i)->get(validity).toInt();
                if (!v) {
                    i++;
                    continue;
                }
            }
            if (!validator((*i)->get(field))) {
                i++;
                continue;
            }
            float v = (*i)->get(field).toFloat();
            if (min>v)
                min=v;
            if (max<v)
                max=v;
            //QDateTime dt;
            //dt.setSecsSinceEpoch((*i)->get("epoch").toInt());
            quint64 ep = (quint64)(*i)->get("epoch").toInt() * 1000;
            qDebug()<< ep;
            series->append( (qreal)ep, // In msec
            (qreal)v);
            i++;
        }
        series->attachAxis(axisX);

    }

    axisY->setMin(min);
    axisY->setMax(max);

    connect( axisX, &QDateTimeAxis::rangeChanged,
            [dc, field, axisY, validity, validator](QDateTime dmin, QDateTime dmax) {

                // This can be slow, maybe optimize?

                float min = std::numeric_limits<float>::max();
                float max = std::numeric_limits<float>::min();

                quint64 dmin_epoch = dmin.toMSecsSinceEpoch();
                quint64 dmax_epoch = dmax.toMSecsSinceEpoch();

                for (auto d: *dc)
                {
                    Dataset::const_iterator i = d->dataset().begin();
                    while (i != d->dataset().end())
                    {
                        quint64 ep = (quint64)(*i)->get("epoch").toInt() * 1000;

                        if (ep<dmin_epoch) {
                            i++;
                            continue;
                        }
                        if (ep>dmax_epoch) {
                            i = d->dataset().end();
                            continue;
                        }


                        if (validity.size()) {
                            int v = (*i)->get(validity).toInt();
                            if (!v) {
                                i++;
                                continue;
                            }
                        }
                        if (!validator((*i)->get(field))) {
                            i++;
                            continue;
                        }
                        float v = (*i)->get(field).toFloat();
                        if (min>v)
                            min=v;
                        if (max<v)
                            max=v;
                        i++;
                    }
                }
                axisY->setMin(min);
                axisY->setMax(max);

            });

}
