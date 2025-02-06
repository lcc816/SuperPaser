#ifndef DESCOBJ_H
#define DESCOBJ_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>

class DescFieldObj : public QJsonObject
{
public:
    DescFieldObj() {}
    DescFieldObj(const QJsonObject &jsonObj) : QJsonObject(jsonObj) {}
    DescFieldObj(const QJsonValue &value) : DescFieldObj(value.toObject()) {}
    DescFieldObj(const DescFieldObj &other) : QJsonObject(other) {}
    bool checkFormat() const;
};

class DescDWordObj : public std::vector<DescFieldObj>
{
public:
    DescDWordObj() {}
    DescDWordObj(const QJsonArray &arr);
    DescDWordObj(const QJsonValue &value);
    bool checkFormat() const;
    QJsonArray toJsonArray() const;
};

class DescObj : public std::vector<DescDWordObj>
{
public:
    DescObj() { qDebug() << __func__ << "default creator"; }
    DescObj(const QJsonArray &arr);
    DescObj(const DescObj &other);
    DescObj& operator=(const DescObj &other) {
        if (this != &other) { // 检查自赋值
            // 调用基类的赋值运算符
            std::vector<DescDWordObj>::operator=(other);
        }
        return *this;
    }
    bool checkFormat() const;
    bool checkValid() const;
    QJsonArray toJsonArray() const;
    QByteArray toBtyeArray() const;

    static DescObj fromJson(const QByteArray &json, bool *ok = nullptr);
};

#endif // DESCOBJ_H
