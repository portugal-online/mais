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
#include "colorallocator.h"
#include "registeredgraph.h"
#include "registeredgraphmodel.h"
#include "newgraphdialog.h"

using namespace QtCharts;

static Dataunits percentage("Humidity (%)");
static Dataunits oaq("Air Quality Index");
static Dataunits sound("Sound level");
static Dataunits temperature("Temperature (C)");
static Dataunits voltage("Voltage (mV)");
static Dataunits spreadfactor("Spreading Factor");

static bool validHumidity(const QVariant &v)
{
    int h = v.toInt();
    return h>=0  && h<=100;
}

static bool validBattery(const QVariant &v)
{
    int h = v.toInt();
    return h>=1800;
}

struct FieldInfo
{
    const char *field;
    const char *validity_field;
    Dataunits *units;
    bool (*validator)(const QVariant&);
    const char *short_description;
    const char *description;
};

static FieldInfo fields[] = {

    { "max_int_hum",   "health_internal", &percentage, &validHumidity, "Max. Int. Hum", "Maximum internal humidity (%)" },
    { "max_int_temp",  "health_internal", &temperature, NULL, "Max. Int. Temp", "Maximum internal temperature (C)" },
    { "max_oaq",       "health_oaq", &oaq, NULL, "Max OAQ", "Maximum OAQ" },
    { "epa_oaq",       "health_oaq", &oaq, NULL, "EPA OAQ", "EPA OAQ" },
    { "max_sound",     "health_mic", &sound, NULL, "Max Sound", "Maximum Sound Level" },
    { "avg_sound",     "health_mic", &sound , NULL, "Avg. Sound", "Average Sound Level" },
    { "avg_ext_temp",  "health_external", &temperature, NULL, "Avg. Ext. Temp", "Average external Temperature (C)" },
    { "avg_ext_hum",   "health_external", &percentage, &validHumidity, "Avg. Ext. Hum", "Average external humidity (%)" },
    { "battery",       "", &voltage, &validBattery, "Battery", "Battery (mV)" },
    { "sf",            "", &spreadfactor, NULL, "SF", "Spreading Factor" },
};

const FieldInfo *getFieldInfo(const char *field)
{
    for (unsigned i=0; i< sizeof(fields)/sizeof(fields[0]); i++) {
        if (strcmp(fields[i].field, field)==0) {
            return &fields[i];
        }
    }
    return NULL;
}

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

    QMenu *graphMenu = menuBar()->addMenu(tr("&Graphs"));
    newg = new QAction(tr("&New graph..."));
    connect(newg, &QAction::triggered, this, &Grapher::onNewGraph);

    fileMenu->addAction(open);
    graphMenu->addAction(newg);

    newg->setEnabled(false);
}

void Grapher::onNewGraph()
{
    NewGraphDialog *n = new NewGraphDialog(m_graphs);
    if (n->exec()) {
        const std::vector<RegisteredGraph> &graphs = n->get();
        DatasetCollection dset;

        for (auto i: graphs) {
            dset.append(i.device(), i.dataset());
        }
        createChart( n->name(), dset);
    }
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

        newg->setEnabled(true);
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

struct Graphinfo
{
    Graphinfo(const std::string &name, const Dataunits &units);
};

void Grapher::initGraph(Device* d,
                        DatasetCollection &col,
                        const QString &field,
                        const QString &health_field,
                        const Dataunits &unit,
                        std::function<bool(const QVariant &v)> valid)
{
    Dataset *dset = d->generate( field, health_field, unit, valid );
    col.append(d, dset);
    //description, shortdesc,

    QString displayname = d->name();

    if (m_devicenames.find(d->name()) != m_devicenames.end())
    {
        displayname = m_devicenames[ d->name() ];
    }

    registerGraph( displayname + " : " + getFieldInfo(field.toLocal8Bit())->short_description ,
                  d,
                  dset
                 );
    // Register graph
    //
    //QString
}




void Grapher::generateCharts(const QString &file)
{
    TTNJson tj;
    tj.load(file);

    DatasetCollection int_hum, int_temp, max_oaq, epa_oaq, max_sound, avg_sound, ext_temp, ext_hum, sf, battery;

    std::vector<QColor> colors;
    colors.push_back(QColor(0,0,128,128));
    colors.push_back(QColor(0,128,0,128));
    colors.push_back(QColor(128,0,0,128));
    colors.push_back(QColor(128,128,0,128));
    colors.push_back(QColor(128,0,128,128));
    colors.push_back(QColor(0,128,128,128));

    DeviceCollection *dc = tj.createCollection(colors);

    dc->computeTime();

    for (auto d: *dc)
    {
        initGraph(d, int_hum, "max_int_hum", "health_internal", percentage, &validHumidity);
        initGraph(d, int_temp, "max_int_temp", "health_internal", temperature);
        initGraph(d, max_oaq, "max_oaq", "health_oaq", oaq);
        initGraph(d, epa_oaq, "epa_oaq", "health_oaq", oaq);
        initGraph(d, max_sound,"max_sound", "health_mic", sound);
        initGraph(d, avg_sound, "avg_sound", "health_mic", sound);
        initGraph(d, ext_temp,"avg_ext_temp", "health_external", temperature);
        initGraph(d, ext_hum, "avg_ext_hum", "health_external", percentage, &validHumidity);
        initGraph(d, battery, "battery", "", voltage, &validBattery);
        initGraph(d, sf, "sf", "", spreadfactor);
    }


    createChart( "Max. Int. Hum", int_hum );
    createChart( "Max. Int. Temp", int_temp );
    createChart( "Max OAQ", max_oaq );
    createChart( "EPA OAQ", epa_oaq );
    createChart( "Max Sound", max_sound );
    createChart( "Avg Sound", avg_sound );
    createChart( "Avg. Ext. Temp", ext_temp );
    createChart( "Avg. Ext. Hum", ext_hum );
    createChart( "SF", sf );
    createChart( "Battery", battery );

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

struct axisinfo
{
    QValueAxis*axis;
    float min;
    float max;
    std::vector<Dataset*> datasets; // Datasets using this axis
};

struct QMyChart: public QChart
{
    QMap<Dataunits, axisinfo> axisY;
};


void Grapher::createChart(const QString &title,
                          const DatasetCollection &items,
                          bool range_color)
{
    QMyChart *chart = new QMyChart();
    chart->setTitle( title );

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    chart->addAxis(axisX, Qt::AlignBottom);


    if (items.unitsCount()==0)
        return;

    // Check how many axes we need.

    for (auto i = items.perUnit().keyValueBegin();
         i !=    items.perUnit().keyValueEnd();
         i++ ) {
        QValueAxis *axis = new QValueAxis();

        chart->axisY[ i->first ].axis = axis;
        chart->axisY[ i->first ].min = std::numeric_limits<float>::max();
        chart->axisY[ i->first ].max = std::numeric_limits<float>::min();

        axis->setTickInterval(10);
        axis->setTickCount(21);
        axis->setTitleText( i->first.description() );

        // const FieldInfo *info

        chart->addAxis(axis,
                       i ==items.perUnit().keyValueBegin() ? Qt::AlignLeft : Qt::AlignRight );
    }

    QColor pen(0,0,128,128);

    axisX->setFormat("dd-MM-yyyy<br/>h:mm");

    QChartView *chartView = new GrapherCustomChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    chartView->setRubberBand(QChartView::HorizontalRubberBand);

    m_tabwidget->addTab(chartView, title);


    int mintime  = items.minTime();
    int maxtime  = items.maxTime();

    qDebug()<<"Min time"<<mintime;
    qDebug()<<"Max time"<<maxtime;
    QDateTime dtmax;
    dtmax.setSecsSinceEpoch(maxtime);
    QDateTime dtmin;
    dtmin.setSecsSinceEpoch(mintime);

    axisX->setMax( dtmax );
    axisX->setMin( dtmin );


    ColorAllocator col_alloc;

    for (auto d: items)
    {
        QLineSeries *series = new QLineSeries();
        chart->addSeries(series);
        series->setPen(QPen( col_alloc.get(), 3));
        series->setPointsVisible();
        axisinfo &info = chart->axisY[ d.second->units() ];

        series->attachAxis(info.axis);
        info.datasets.push_back(d.second);

        Dataset::const_iterator i = d.second->begin();
        QString displayname = d.first->name();

        if (m_devicenames.find(d.first->name()) != m_devicenames.end())
        {
            displayname = m_devicenames[ d.first->name() ];
        }


        const FieldInfo *finfo = getFieldInfo(d.second->field().toLocal8Bit());

        if (finfo) {
            series->setName( displayname + "<br/>" + finfo->description);
        } else {
            series->setName( displayname + "<br/>" + "Unknown field");
        }

        while (i != d.second->end())
        {
            float v = (*i).value();
            if (info.min>v) {
                info.min=v;
                info.axis->setMin(v);
            }
            if (info.max<v) {
                info.max=v;
                info.axis->setMax(v);
            }

            quint64 ep = (quint64)(*i).epoch() * 1000;

            series->append( (qreal)ep, // In msec
                           (qreal)v);
            i++;
        }
        series->attachAxis(axisX);

    }

    connect( axisX, &QDateTimeAxis::rangeChanged,
            [chart](QDateTime dmin, QDateTime dmax) {

                quint64 dmin_epoch = dmin.toMSecsSinceEpoch()/1000;
                quint64 dmax_epoch = dmax.toMSecsSinceEpoch()/1000;

                // Iterate through all axis, compute min/max

                for (auto axisinfo: chart->axisY.values()) {

                    double min = std::numeric_limits<float>::max();
                    double max = std::numeric_limits<float>::min();
                    // For all datasets unsing this axis, get min/max
                    for (auto d: axisinfo.datasets) {
                        std::pair<double, double> minmax  = d->getMinMax(dmin_epoch, dmax_epoch);
                        min = std::min( min, minmax.first );
                        max = std::max( max, minmax.second );
                    }
                    axisinfo.axis->setMin(min);
                    axisinfo.axis->setMax(max);

#if 0
                    //if (range_color) {
                    QLinearGradient plotAreaGradient;
                    plotAreaGradient.setStart(QPointF(0, 1));
                    plotAreaGradient.setFinalStop(QPointF(0, 0));

#define EPA_RANGE(x) (((float)x)/500.0)
                    //min/500

                    plotAreaGradient.setColorAt(EPA_RANGE(0), QRgb(0x90d856));
                    plotAreaGradient.setColorAt(EPA_RANGE(51), QRgb(0xf7fb6a));
                    plotAreaGradient.setColorAt(EPA_RANGE(101), QRgb(0xd0913e));
                    plotAreaGradient.setColorAt(EPA_RANGE(151), QRgb(0xd25927));
                    plotAreaGradient.setColorAt(EPA_RANGE(201), QRgb(0x794fb5));
                    plotAreaGradient.setColorAt(EPA_RANGE(301), QRgb(0x6d3934));

                    plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
                    chart->setPlotAreaBackgroundBrush(plotAreaGradient);
                    chart->setPlotAreaBackgroundVisible(true);
#endif
                }

            });

#if 0
    if (range_color) {
        QLinearGradient plotAreaGradient;
        plotAreaGradient.setStart(QPointF(0, 1));
        plotAreaGradient.setFinalStop(QPointF(0, 0));

#define EPA_RANGE(x) (((float)x)/500.0)

        plotAreaGradient.setColorAt(EPA_RANGE(0), QRgb(0x90d856));
        plotAreaGradient.setColorAt(EPA_RANGE(51), QRgb(0xf7fb6a));
        plotAreaGradient.setColorAt(EPA_RANGE(101), QRgb(0xd0913e));
        plotAreaGradient.setColorAt(EPA_RANGE(151), QRgb(0xd25927));
        plotAreaGradient.setColorAt(EPA_RANGE(201), QRgb(0x794fb5));
        plotAreaGradient.setColorAt(EPA_RANGE(301), QRgb(0x6d3934));

        plotAreaGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        chart->setPlotAreaBackgroundBrush(plotAreaGradient);
        chart->setPlotAreaBackgroundVisible(true);
    }
#endif

}
