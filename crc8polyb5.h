#ifndef CRC8POLYB5_H
#define CRC8POLYB5_H

#include <QByteArray>
#include <QVariantHash>
#include <QList>


class Crc8PolyB5
{
public:

    static QByteArray crcCalc(const QByteArray &nodeID,  const QByteArray &passwd, const QByteArray &dataArr);

    static QByteArray crcCalcMSI(const QByteArray &nodeID,  const QByteArray &passwd, const QByteArray &commandh, const QByteArray &dataArr);

    static QByteArray crcCalcExt(const quint8 &optByte, const QByteArray &nodeID,  const QByteArray &argsLeftH, const QByteArray &argsRightH, const QByteArray &dataArr);

    static quint8 calculateCrc4mess(const QList<quint8> &mas);

    static bool addArgs2mas(QList<quint8> &mas, const QByteArray &argsh);

    static bool messageIsValid(const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, QList<quint8> &meterMess);

    static bool messageIsValidMci(const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, quint8 &codeAnswr, QList<quint8> &meterMess);


    static bool messageIsValidExt(const bool &isMci, const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, quint8 &codeAnswr, QList<quint8> &meterMess);

    static bool unByteStaffing(QList<quint8> &mas, int &masSize);
};

#endif // CRC8POLYB5_H
