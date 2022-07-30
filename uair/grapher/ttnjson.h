#include <QJsonObject>
#include <QJsonDocument>
#include <QColor>

class DeviceCollection;

class TTNJson
{
public:
    TTNJson();

    bool load(const QString &filename);
    DeviceCollection *createCollection(const std::vector<QColor> &colormap);

    QJsonDocument m_doc;
};
