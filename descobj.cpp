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

uint32_t DescObj::extractSubfield(uint32_t number, int n, int m)
{
    // 确保 n 和 m 在有效范围内（0 <= n <= m <= 31）
    if ((n < 0) || (m > 31) || (n > m)) {
        return 0;  // 如果输入无效，返回 0
    }

    // 计算掩码：从第 n 位到第 m 位的掩码
    uint32_t mask;
    if ((n == 0) && (m == 31)) {
        return number;
    } else {
        mask = (1UL << (m - n + 1)) - 1;  // 生成 (m - n + 1) 个 1
    }
    mask <<= n;  // 将掩码左移 n 位，对齐到第 n 位

    // 提取字段值
    uint32_t result = (number & mask) >> n;

    return result;
}
