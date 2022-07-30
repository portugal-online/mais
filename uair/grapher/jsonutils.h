#ifndef JSONUTILS_H__
#define JSONUTILS_H__

#include <QJsonDocument>
#include <exception>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class QString;

class JSONInvalidObject: public std::exception
{
};


QJsonDocument loadJSONFromFile(const QString &filename);
QJsonObject JSONfindObject(QJsonObject root, const QString &path);
QJsonArray JSONfindArray(QJsonObject root, QString path);
QVariant JSONfindValue(QJsonObject root, const QString &path);

#endif
