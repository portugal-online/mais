#include "jsonutils.h"
#include <QFile>
#include <QString>

QJsonDocument loadJSONFromFile(const QString &filename)
{
    QFile f(filename);
    QJsonDocument doc;

    f.open(QIODevice::ReadOnly | QIODevice::Text);

    if (f.isOpen())
    {
        QJsonParseError error;
        QByteArray data = f.readAll();

        doc = QJsonDocument::fromJson(data, &error);
    }
    return doc;
}

QJsonObject JSONfindObject(QJsonObject root, const QString &path)
{
    QStringList items = path.split("/");
    int count = items.size();
    QJsonObject r = root;

    for (auto item: items)
    {
        count--;

        //qDebug()<<"Search "<<item<< r.keys().join(":");

        QJsonObject::const_iterator it =
            r.find(item);

        if (it==r.end()) {
            qDebug()<<"Cannot locate"<<item<<"in"<<path;
            throw JSONInvalidObject();
        }

        if (!it->isObject()) {
            qDebug()<<"Not an object"<<*it;
            throw JSONInvalidObject();
        }
        r = it->toObject();
    }
    return r;
}

QJsonArray JSONfindArray(QJsonObject root, QString path)
{
    QStringList items = path.split("/");
    int count = items.size();
    QJsonObject r = root;

    for (auto item: items)
    {
        count--;

        QJsonObject::const_iterator it =
            r.find(item);

        if (it==r.end()) {
            qDebug()<<"Cannot locate"<<item<<"in"<<path;
            throw JSONInvalidObject();
        }

        // If last, and if array, we're ok
        if (count==0) {
            if (!it->isArray()) {
                qDebug()<<"Not an array";
                throw JSONInvalidObject();
            }
            return it->toArray();
        }

        if (it->isObject()) {
            throw JSONInvalidObject();
        }
        r = it->toObject();
    }
    throw JSONInvalidObject();
}

QVariant JSONfindValue(QJsonObject root, const QString &path)
{
    QStringList items = path.split("/");
    int count = items.size();
    QJsonObject r = root;

    for (auto item: items)
    {
        count--;

        QJsonObject::const_iterator it =
            r.find(item);

        if (it==r.end()) {
            qDebug()<<"Cannot locate"<<item<<"in"<<path;
            throw JSONInvalidObject();
        }

        // If last, and if array, we're ok
        if (count==0) {
            return it->toVariant();
        }

        if (!it->isObject()) {
            qDebug()<<"Not child object count"<<count<<"item"<<item<<*it;
            throw JSONInvalidObject();
        }
        r = it->toObject();
    }
    throw JSONInvalidObject();
}

