#ifndef MSIEMBEEHELPER_H
#define MSIEMBEEHELPER_H

#include <QtCore>

#define MSIEMB_ERR_UNK              0x00

#define MSIEMB_ERR_OK               0x20
#define MSIEMB_ERR_CRC              0x21
#define MSIEMB_ERR_LENGTHDATA       0x22
#define MSIEMB_ERR_DATA             0x23
#define MSIEMB_ERR_PARAMETR         0x24
#define MSIEMB_ERR_LEVEL_ACCESS     0x25
#define MSIEMB_ERR_SUPPORT          0x26
#define MSIEMB_ERR_SERIAL_ABSENT    0x27//no S/N
#define MSIEMB_ERR_RAM              0x28
#define MSIEMB_ERR_ROM              0x29
#define MSIEMB_ERR_INTERVAL_TIME    0x2A
#define MSIEMB_ERR_SN_EXISTS        0x2B


class MsiEMBeeHelper
{
public:
    static QByteArray getDefPasswd(const QString &vrsn) ;

    static QStringList getEnrgList4thisMeter(const quint8 &pollCode, const QString &version) ;

    static bool isPollCodeSupported(const quint16 &pollCode, QVariantHash &hashTmpData);

    static bool isPollCodeSupported(const quint16 &pollCode);

    static QByteArray crcCalc(const QByteArray &nodeID, const QByteArray &passwd, const QByteArray &commandh, const QByteArray &dataArr, const QVariantHash &hashTmpData);

    static bool messageIsValid(const QByteArray &nodeID, const QByteArray &readArr, const bool &verboseMode, QByteArray &commandCode, QString &errMess, QList<quint8> &meterMess);

    static QVariantHash helloMeter(const QVariantHash &hashMeterConstData);

    static QByteArray writeMeterDateTime(const QByteArray &nodeID, const QByteArray &passwd, const QVariantHash &hashTmpData);

    static QByteArray getDateTimeBcd(const QDateTime &dt);

    static QString prettyMess(const QString &mess, const QString &hexDump);

    static QString errWarnKey(int &val, const bool &isErr);

    static QString prettyHexDump(const QList<quint8> &list, const QByteArray &command);

    static bool decodeSerialNumber(const QList<quint8> &meterMess, QVariantHash &hashTmpData);

    static bool decodeVersion(const QList<quint8> &meterMess, QVariantHash &hashTmpData);

    static bool decodeAsciiLine(const QList<quint8> &meterMess, const QString &key, QVariantHash &hashTmpData);

    static bool decodeAsciiLine(const QList<quint8> &meterMess, QString &line);
    static bool decodeAsciiLine(const QList<quint8> &meterMess, QString &line, const int &indxFrom, const int &len);

    static QDateTime getDateTimeFromMeterMess(const QList<quint8> &meterMess);

    static bool decodeDateTime(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData);

    static bool decodeTotalValues(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData);

    static bool decodeEndOfDayValues(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData);

    static bool isThisData2msiEmbee(const QByteArray &arr, QByteArray &modemni);

};

#endif // MSIEMBEEHELPER_H
