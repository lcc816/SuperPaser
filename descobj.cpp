#include "descobj.h"

bool DescFieldObj::checkFormat() const
{
    if (empty())
        return true;

    return QJsonObject::contains("field")
           && QJsonObject::contains("LSB")
           && QJsonObject::contains("MSB");
}

DescDWordObj::DescDWordObj(const QJsonArray &arr)
{
    for (int i = 0; i < arr.size(); i++) {
        if (!arr[i].isObject()) {
            qWarning("%s[%d]: %d: Not a valid Filed object", __func__, __LINE__, i);
            break;
        }
        push_back(DescFieldObj(arr[i]));
    }
}

DescDWordObj::DescDWordObj(const QJsonValue &value)
    : DescDWordObj(value.toArray())
{
}

bool DescDWordObj::checkFormat() const
{
    if (empty())
        return true;
    for (auto it = begin(); it != end(); ++it) {
        if (!it->checkFormat())
            return false;
    }
    return true;
}

QJsonArray DescDWordObj::toJsonArray() const
{
    QJsonArray arr;
    for (int i = 0; i < this->size(); i++) {
        arr.push_back(this->at(i));
    }
    return arr;
}

DescObj::DescObj(const QJsonArray &arr)
{
    for (int i = 0; i < arr.size(); i++) {
        if (!arr[i].isArray()) {
            qWarning("%s[%d]: %d: Not a valid DW object", __func__, __LINE__, i);
            break;
        }
        push_back(DescDWordObj(arr[i]));
    }
}

DescObj::DescObj(const DescObj &other)
    : QList<DescDWordObj>(other)
{
    qDebug() << __func__ << "Desc copied";
}

bool DescObj::checkFormat() const
{
    if (empty())
        return true;
    for (auto it = begin(); it != end(); ++it) {
        if (!it->checkFormat())
            return false;
    }
    return true;
}

bool DescObj::checkValid() const
{
    // toto
    return true;
}

QJsonArray DescObj::toJsonArray() const
{
    QJsonArray arr;
    for (int i = 0; i < this->size(); i++) {
        arr.push_back(this->at(i).toJsonArray());
    }
    return arr;
}

QByteArray DescObj::toBtyeArray() const
{
    QJsonDocument doc(toJsonArray());
    return doc.toJson();
}

DescObj DescObj::fromJson(const QByteArray &json, bool *ok)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json, &error);
    if (error.error != QJsonParseError::NoError)
        qDebug() << "JSON error:" << error.error << error.errorString();
    if (ok)
        *ok = (error.error == QJsonParseError::NoError);

    return DescObj(doc.array());
}
