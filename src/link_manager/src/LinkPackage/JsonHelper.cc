/****************************************************************************
 *
 * (c) 2009-2024 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "JsonHelper.h"


#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonParseError>
#include <QtCore/QObject>
#include <QtCore/QTranslator>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(JsonHelperLog, "qgc.utilities.jsonhelper")

namespace JsonHelper
{
    QString _jsonValueTypeToString(QJsonValue::Type type);
    QStringList _addDefaultLocKeys(QJsonObject &jsonObject);
    QJsonObject _translateRoot(QJsonObject &jsonObject, const QString &translateContext, const QStringList &translateKeys);
    QJsonObject _translateObject(QJsonObject &jsonObject, const QString &translateContext, const QStringList &translateKeys);
    QJsonArray _translateArray(QJsonArray &jsonArray, const QString &translateContext, const QStringList &translateKeys);

    constexpr const char *_translateKeysKey = "translateKeys";
    constexpr const char *_arrayIDKeysKey = "_arrayIDKeys";
    constexpr const char *_jsonGroundStationKey = "groundStation";
    constexpr const char *_jsonGroundStationValue = "QGroundControl";
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QtCore/qapplicationstatic.h>
    Q_APPLICATION_STATIC(QTranslator, s_jsonTranslator);
#else
    #include <QtGlobal>
    Q_GLOBAL_STATIC(QTranslator, s_jsonTranslator)
#endif

QTranslator *JsonHelper::translator()
{
    return s_jsonTranslator();
}

bool JsonHelper::validateRequiredKeys(const QJsonObject &jsonObject, const QStringList &keys, QString &errorString)
{
    QString missingKeys;

    for (const QString &key : keys) {
        if (!jsonObject.contains(key)) {
            if (!missingKeys.isEmpty()) {
                missingKeys += QStringLiteral(", ");
            }
            missingKeys += key;
        }
    }

    if (!missingKeys.isEmpty()) {
        errorString = QObject::tr("The following required keys are missing: %1").arg(missingKeys);
        return false;
    }

    return true;
}

bool JsonHelper::validateKeyTypes(const QJsonObject &jsonObject, const QStringList &keys, const QList<QJsonValue::Type> &types, QString & errorString)
{
    for (qsizetype i = 0; i < types.count(); i++) {
        const QString valueKey = keys[i];
        if (jsonObject.contains(valueKey)) {
            const QJsonValue &jsonValue = jsonObject[valueKey];
            if ((jsonValue.type() == QJsonValue::Null) && (types[i] == QJsonValue::Double)) {
                // Null type signals a NaN on a double value
                continue;
            }
            if (jsonValue.type() != types[i]) {
                errorString  = QObject::tr("Incorrect value type - key:type:expected %1:%2:%3").arg(valueKey, _jsonValueTypeToString(jsonValue.type()), _jsonValueTypeToString(types[i]));
                return false;
            }
        }
    }

    return true;
}

bool JsonHelper::isJsonFile(const QByteArray &bytes, QJsonDocument &jsonDoc, QString &errorString)
{
    QJsonParseError parseError;
    jsonDoc = QJsonDocument::fromJson(bytes, &parseError);

    if (parseError.error == QJsonParseError::NoError) {
        return true;
    }

    const int startPos = qMax(0, parseError.offset - 100);
    const int length = qMin(bytes.length() - startPos, 200);
    qCDebug(JsonHelperLog) << "Json read error" << bytes.mid(startPos, length).constData();
    errorString = parseError.errorString();

    return false;
}

bool JsonHelper::isJsonFile(const QString &fileName, QJsonDocument &jsonDoc, QString &errorString)
{
    QFile jsonFile(fileName);
    if (!jsonFile.open(QFile::ReadOnly)) {
        errorString = QObject::tr("File open failed: file:error %1 %2").arg(jsonFile.fileName(), jsonFile.errorString());
        return false;
    }

    const QByteArray jsonBytes = jsonFile.readAll();
    jsonFile.close();

    return isJsonFile(jsonBytes, jsonDoc, errorString);
}

bool JsonHelper::validateInternalQGCJsonFile(const QJsonObject &jsonObject, const QString &expectedFileType, int minSupportedVersion, int maxSupportedVersion, int &version, QString &errorString)
{
    static const QList<JsonHelper::KeyValidateInfo> requiredKeys = {
        { jsonFileTypeKey, QJsonValue::String, true },
        { jsonVersionKey, QJsonValue::Double, true },
    };

    if (!JsonHelper::validateKeys(jsonObject, requiredKeys, errorString)) {
        return false;
    }

    const QString fileTypeValue = jsonObject[jsonFileTypeKey].toString();
    if (fileTypeValue != expectedFileType) {
        errorString = QObject::tr("Incorrect file type key expected:%1 actual:%2").arg(expectedFileType, fileTypeValue);
        return false;
    }

    version = jsonObject[jsonVersionKey].toInt();
    if (version < minSupportedVersion) {
        errorString = QObject::tr("File version %1 is no longer supported").arg(version);
        return false;
    }

    if (version > maxSupportedVersion) {
        errorString = QObject::tr("File version %1 is newer than current supported version %2").arg(version).arg(maxSupportedVersion);
        return false;
    }

    return true;
}

bool JsonHelper::validateExternalQGCJsonFile(const QJsonObject &jsonObject, const QString &expectedFileType, int minSupportedVersion, int maxSupportedVersion, int &version, QString &errorString)
{
    static const QList<JsonHelper::KeyValidateInfo> requiredKeys = {
        { _jsonGroundStationKey, QJsonValue::String, true },
    };

    if (!JsonHelper::validateKeys(jsonObject, requiredKeys, errorString)) {
        return false;
    }

    return validateInternalQGCJsonFile(jsonObject, expectedFileType, minSupportedVersion, maxSupportedVersion, version, errorString);
}

QStringList JsonHelper::_addDefaultLocKeys(QJsonObject &jsonObject)
{
    QString translateKeys;
    const QString fileType = jsonObject[jsonFileTypeKey].toString();
    if (!fileType.isEmpty()) {
        if (fileType == "MavCmdInfo") {
            if (jsonObject.contains(_translateKeysKey)) {
                translateKeys = jsonObject[_translateKeysKey].toString();
            } else {
                translateKeys = QStringLiteral("label,enumStrings,friendlyName,description,category");
                jsonObject[_translateKeysKey] = translateKeys;
            }

            if (!jsonObject.contains(_arrayIDKeysKey)) {
                jsonObject[_arrayIDKeysKey] = QStringLiteral("rawName,comment");
            }
        } else if (fileType == "FactMetaData") {
            if (jsonObject.contains(_translateKeysKey)) {
                translateKeys = jsonObject[_translateKeysKey].toString();
            } else {
                translateKeys = QStringLiteral("shortDescription,longDescription,enumStrings");
                jsonObject[_translateKeysKey] = QStringLiteral("shortDescription,longDescription,enumStrings");
            }

            if (!jsonObject.contains(_arrayIDKeysKey)) {
                jsonObject[_arrayIDKeysKey] = QStringLiteral("name");
            }
        }
    }

    return translateKeys.split(",");
}

QJsonObject JsonHelper::_translateObject(QJsonObject &jsonObject, const QString &translateContext, const QStringList &translateKeys)
{
    for (const QString &key: jsonObject.keys()) {
        if (jsonObject[key].isString()) {
            QString locString = jsonObject[key].toString();
            if (!translateKeys.contains(key)) {
                continue;
            }

            QString disambiguation;
            QString disambiguationPrefix("#loc.disambiguation#");

            if (locString.startsWith(disambiguationPrefix)) {
                locString = locString.right(locString.length() - disambiguationPrefix.length());
                const int commentEndIndex = locString.indexOf("#");
                if (commentEndIndex != -1) {
                    disambiguation = locString.left(commentEndIndex);
                    locString = locString.right(locString.length() - disambiguation.length() - 1);
                }
            }

            const QString xlatString = translator()->translate(translateContext.toUtf8().constData(), locString.toUtf8().constData(), disambiguation.toUtf8().constData());
            if (!xlatString.isNull()) {
                jsonObject[key] = xlatString;
            }
        } else if (jsonObject[key].isArray()) {
            QJsonArray childJsonArray = jsonObject[key].toArray();
            jsonObject[key] = _translateArray(childJsonArray, translateContext, translateKeys);
        } else if (jsonObject[key].isObject()) {
            QJsonObject childJsonObject = jsonObject[key].toObject();
            jsonObject[key] = _translateObject(childJsonObject, translateContext, translateKeys);
        }
    }

    return jsonObject;
}

QJsonArray JsonHelper::_translateArray(QJsonArray &jsonArray, const QString &translateContext, const QStringList &translateKeys)
{
    for (qsizetype i = 0; i < jsonArray.count(); i++) {
        QJsonObject childJsonObject = jsonArray[i].toObject();
        jsonArray[i] = _translateObject(childJsonObject, translateContext, translateKeys);
    }

    return jsonArray;
}

QJsonObject JsonHelper::_translateRoot(QJsonObject &jsonObject, const QString &translateContext, const QStringList &translateKeys)
{
    return _translateObject(jsonObject, translateContext, translateKeys);
}

QJsonObject JsonHelper::openInternalQGCJsonFile(const QString &jsonFilename, const QString &expectedFileType, int minSupportedVersion, int maxSupportedVersion, int &version, QString &errorString)
{
    QFile jsonFile(jsonFilename);
    if (!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorString = QObject::tr("Unable to open file: '%1', error: %2").arg(jsonFilename, jsonFile.errorString());
        return QJsonObject();
    }

    const QByteArray bytes = jsonFile.readAll();
    jsonFile.close();
    QJsonParseError jsonParseError;
    const QJsonDocument doc = QJsonDocument::fromJson(bytes, &jsonParseError);
    if (jsonParseError.error != QJsonParseError::NoError) {
        errorString = QObject::tr("Unable to parse json file: %1 error: %2 offset: %3").arg(jsonFilename, jsonParseError.errorString()).arg(jsonParseError.offset);
        return QJsonObject();
    }

    if (!doc.isObject()) {
        errorString = QObject::tr("Root of json file is not object: %1").arg(jsonFilename);
        return QJsonObject();
    }

    QJsonObject jsonObject = doc.object();
    const bool success = validateInternalQGCJsonFile(jsonObject, expectedFileType, minSupportedVersion, maxSupportedVersion, version, errorString);
    if (!success) {
        errorString = QObject::tr("Json file: '%1'. %2").arg(jsonFilename, errorString);
        return QJsonObject();
    }

    const QStringList translateKeys = _addDefaultLocKeys(jsonObject);
    const QString context = QFileInfo(jsonFile).fileName();
    return _translateRoot(jsonObject, context, translateKeys);
}

void JsonHelper::saveQGCJsonFileHeader(QJsonObject &jsonObject, const QString &fileType, int version)
{
    jsonObject[_jsonGroundStationKey] = _jsonGroundStationValue;
    jsonObject[jsonFileTypeKey] = fileType;
    jsonObject[jsonVersionKey] = version;
}







bool JsonHelper::validateKeys(const QJsonObject &jsonObject, const QList<JsonHelper::KeyValidateInfo> &keyInfo, QString &errorString)
{
    QStringList keyList;
    QList<QJsonValue::Type> typeList;

    for (const JsonHelper::KeyValidateInfo &info : keyInfo) {
        if (info.required) {
            keyList.append(info.key);
        }
    }
    if (!validateRequiredKeys(jsonObject, keyList, errorString)) {
        return false;
    }

    keyList.clear();
    for (const JsonHelper::KeyValidateInfo &info : keyInfo) {
        keyList.append(info.key);
        typeList.append(info.type);
    }

    return validateKeyTypes(jsonObject, keyList, typeList, errorString);
}

QString JsonHelper::_jsonValueTypeToString(QJsonValue::Type type)
{
    struct TypeToString {
        QJsonValue::Type type;
        const char *string;
    };

    static constexpr const TypeToString rgTypeToString[] = {
        { QJsonValue::Null, "NULL" },
        { QJsonValue::Bool, "Bool" },
        { QJsonValue::Double, "Double" },
        { QJsonValue::String, "String" },
        { QJsonValue::Array, "Array" },
        { QJsonValue::Object, "Object" },
        { QJsonValue::Undefined, "Undefined" },
    };

    for (const TypeToString &conv : rgTypeToString) {
        if (type == conv.type) {
            return conv.string;
        }
    }

    return QObject::tr("Unknown type: %1").arg(type);
}


double JsonHelper::possibleNaNJsonValue(const QJsonValue &value)
{
    if (value.type() == QJsonValue::Null) {
        return std::numeric_limits<double>::quiet_NaN();
    } else {
        return value.toDouble();
    }
}
