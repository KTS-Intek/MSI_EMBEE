#include "msiembeehelper.h"
#include "shared/definedpollcodes.h"
#include "crc8polyb5.h"
#include "shared/meterpluginhelper.h"
#include "shared/matildalimits.h"


#define MSIEMB_DATA_STTS_OK         0
#define MSIEMB_DATA_STTS_WAS_OFF    1
#define MSIEMB_DATA_STTS_NODATA     2

#define MSIEMB_MAX_TARIFF_COUNT     8

//-----------------------------------------------------------------------------------------------------

QByteArray MsiEMBeeHelper::getDefPasswd(const QString &vrsn)
{
    Q_UNUSED(vrsn);
    return QByteArray("0");
}

//-----------------------------------------------------------------------------------------------------

QStringList MsiEMBeeHelper::getEnrgList4thisMeter(const quint8 &pollCode, const QString &version)
{
    Q_UNUSED(version);
    if(!isPollCodeSupported(pollCode))
        return QStringList();
    return QString("A+").split(" ");
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::isPollCodeSupported(const quint16 &pollCode, QVariantHash &hashTmpData)
{
    if(hashTmpData.value("plgChecked").toBool())
        return true;

    const bool isSupp = isPollCodeSupported(pollCode);

    hashTmpData.insert("sprtdVls", isSupp);
    hashTmpData.insert("plgChecked", true);
    return isSupp;
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::isPollCodeSupported(const quint16 &pollCode)
{
    return (pollCode == POLL_CODE_READ_TOTAL || pollCode == POLL_CODE_READ_END_DAY);
}

//-----------------------------------------------------------------------------------------------------

QByteArray MsiEMBeeHelper::crcCalc(const QByteArray &nodeID, const QByteArray &passwd, const QByteArray &commandh, const QByteArray &dataArr, const QVariantHash &hashTmpData)
{
    bool ok = !passwd.isEmpty();
    if(ok)
        passwd.toUInt(nullptr, ok);
    return Crc8PolyB5::crcCalcMSI(nodeID, ok ? passwd : getDefPasswd(hashTmpData.value("vrsn").toString()), commandh, dataArr);
}
//-----------------------------------------------------------------------------------------------------
bool MsiEMBeeHelper::messageIsValid(const QByteArray &nodeID, const QByteArray &readArr, const bool &verboseMode, QByteArray &commandCode, QString &errMess, QList<quint8> &meterMess)
{
    quint8 codeAnswr = MSIEMB_ERR_UNK;


    const bool r = Crc8PolyB5::messageIsValidMci(nodeID, readArr, verboseMode, commandCode, codeAnswr, meterMess);
    if(r){
        if(codeAnswr == MSIEMB_ERR_OK)
            return  true;

        bool retv = false;
        switch (codeAnswr) {
        case MSIEMB_ERR_OK               : errMess = "ok"           ; retv = true; break;//0x20
        case MSIEMB_ERR_CRC              : errMess = "bad crc"      ; break;//0x21
        case MSIEMB_ERR_LENGTHDATA       : errMess = "data length"  ; break;//0x22
        case MSIEMB_ERR_DATA             : errMess = "bad data"     ; break;//0x23
        case MSIEMB_ERR_PARAMETR         : errMess = "bad parameter"; break;//0x24
        case MSIEMB_ERR_LEVEL_ACCESS     : errMess = "access level" ; break;//0x25
        case MSIEMB_ERR_SUPPORT          : errMess = "not supported"; break;//0x26
        case MSIEMB_ERR_SERIAL_ABSENT    : errMess = "no S/N"       ; break;//0x27//no S/N
        case MSIEMB_ERR_RAM              : errMess = "bad RAM"      ; break;//0x28
        case MSIEMB_ERR_ROM              : errMess = "bad ROM"      ; break;//0x29
        case MSIEMB_ERR_INTERVAL_TIME    : errMess = "bad interval" ; retv = true; ; break;//0x2A
        case MSIEMB_ERR_SN_EXISTS        : errMess = "S/N is ready" ; retv = true; break;//0x2B
        default: errMess = QString("unknown error 0x%1").arg(int(codeAnswr), 0, 16); break;//0x00

        }
        return  retv;
    }
    return  false;
}
//-----------------------------------------------------------------------------------------------------
QVariantHash MsiEMBeeHelper::helloMeter(const QVariantHash &hashMeterConstData)
{
    QVariantHash hash;
    /*
     * bool data7EPt = hashMessage.value("data7EPt", false).toBool();
     * qint32 readLen = hashMessage.value("readLen", 0).toInt();
     * QByteArray endSymb = hashMessage.value("endSymb", QByteArray("\r\n")).toByteArray();
     * QByteArray currNI = hashMeterConstData.value("NI").toByteArray();
     * hashMessage.value("message")
*/

    bool verbouseMode = true;
    bool ok;
    const QByteArray nodeID = hashMeterConstData.value("NI").toByteArray();
    int meterNIint = nodeID.toInt(&ok);
    if(!ok || meterNIint > 0xffff || meterNIint < 0){
        if(verbouseMode)
            qDebug() << "ni out of range " << nodeID.toHex();
        return hash;
    }
//    lastNIlen = nodeID.size();
    const QByteArray dataWrite = crcCalc(nodeID, getDefPasswd(""), "31", "", hashMeterConstData);
    //    lastAddr = dataWrite.mid(2 ,2);

    qint32 readFromPortLen = 17;

    hash.insert("readLen", readFromPortLen);
    hash.insert("endSymb", QByteArray::fromHex("C0"));
    hash.insert("message", dataWrite);
    //C0 48 06 00 FD 00 00 00 00 00 D0 01 00 F4 C0
    return hash;
}
//-----------------------------------------------------------------------------------------------------
QByteArray MsiEMBeeHelper::writeMeterDateTime(const QByteArray &nodeID, const QByteArray &passwd, const QVariantHash &hashTmpData)
{
//    const QDateTime dateTime = QDateTime::currentDateTime().addSecs(2);
//    QByteArray dateTimeArr = dateTime.toString("ssmmhh").toLocal8Bit();
//    const int dayOfWeek = dateTime.date().dayOfWeek();
//    dateTimeArr.append("0" + QByteArray::number( (dayOfWeek > 6) ? 0 : dayOfWeek));
//    dateTimeArr.append(dateTime.toString("ssmmhhddMMyy").toLocal8Bit());

    return crcCalc(nodeID, passwd, "81", getDateTimeBcd(QDateTime::currentDateTime().addSecs(2)), hashTmpData);
}

//-----------------------------------------------------------------------------------------------------

QByteArray MsiEMBeeHelper::getDateTimeBcd(const QDateTime &dt)
{
    return  QByteArray::fromHex(dt.addYears(-2).toString("ssmmhhddMMyy").toLocal8Bit());
}

//-----------------------------------------------------------------------------------------------------

QString MsiEMBeeHelper::prettyMess(const QString &mess, const QString &hexDump)
{
    return  QString("%1 %2%3").arg(QDateTime::currentDateTime().toString("hh:mm:ss")).arg(mess).arg(hexDump );

}

//-----------------------------------------------------------------------------------------------------

QString MsiEMBeeHelper::errWarnKey(int &val, const bool &isErr)
{
    return isErr ? QString("Error_%1").arg(val++) : QString("Warning_%1").arg(val++);
}

//-----------------------------------------------------------------------------------------------------

QString MsiEMBeeHelper::prettyHexDump(const QList<quint8> &list, const QByteArray &command)
{
    QByteArray arr;
    for(int i = 0, iMax = list.size(); i < iMax; i++)
        arr.append(list.at(i));
    arr = arr.toHex();
    QString str;
    for(int i = 0, iMax = arr.size(); i < iMax; i += 2)
        str.append(arr.mid(i,2) + " ");

    if(!str.isEmpty())
        str.chop(1);
    //    if(str.isEmpty())
    //        return "";

    if(!str.isEmpty())
        str.prepend(", D:");

    str.prepend(", C:" + command.toHex().toUpper());
    return  str.toUpper();
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::decodeSerialNumber(const QList<quint8> &meterMess, QVariantHash &hashTmpData)
{
    return decodeAsciiLine(meterMess, "SN", hashTmpData);
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::decodeVersion(const QList<quint8> &meterMess, QVariantHash &hashTmpData)
{
    return decodeAsciiLine(meterMess, "vrsn", hashTmpData);
}
//-----------------------------------------------------------------------------------------------------
bool MsiEMBeeHelper::decodeAsciiLine(const QList<quint8> &meterMess, const QString &key, QVariantHash &hashTmpData)
{
    QString line;
    const bool r = decodeAsciiLine(meterMess, line);
    if(r)
        hashTmpData.insert(key, line);
    return r;
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::decodeAsciiLine(const QList<quint8> &meterMess, QString &line)
{
    return decodeAsciiLine(meterMess, line, 0, meterMess.length());

}
//-----------------------------------------------------------------------------------------------------
bool MsiEMBeeHelper::decodeAsciiLine(const QList<quint8> &meterMess, QString &line, const int &indxFrom, const int &len)
{
    line = MeterPluginHelper::uint8list2line(meterMess, indxFrom, len).simplified().trimmed();
    return !line.isEmpty();
}

//-----------------------------------------------------------------------------------------------------

QDateTime MsiEMBeeHelper::getDateTimeFromMeterMess(const QList<quint8> &meterMess)
{
    QStringList l;
    for(int i = 0, imax = meterMess.size(); i < imax; i++)
        l.append(QString::number(meterMess.at(i), 16));
    if(!l.isEmpty()){
        const int y = l.takeLast().toInt();
        l.append(QString::number(2000 + y));
    }

    const QDateTime dt = QDateTime::fromString(l.join(","), "s,m,h,d,M,yyyy");

    return dt.addYears(2);
}
//-----------------------------------------------------------------------------------------------------
bool MsiEMBeeHelper::decodeDateTime(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{
    QString err, warn, mtd;

    const bool r = MeterPluginHelper::getCorrDateTimeExt(true, hashTmpData, hashConstData, getDateTimeFromMeterMess(meterMess), err, warn, mtd);



    return r;

}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::decodeTotalValues(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{
    const int maxTarff = MeterPluginHelper::getTariffCount(hashConstData);

    bool hasGoodVal = false;
    quint64 summ = 0;
    for(int t = 0, l = 0, lmax = meterMess.size(); l < lmax && t <= MSIEMB_MAX_TARIFF_COUNT; t++, l += 4){
        const QString s = MeterPluginHelper::uint8list2str(meterMess, l, 4);
        bool ok = false;// decodeAsciiLine(meterMess, s, l, 4);
        const quint32 v = (s.isEmpty()) ? 0 : s.toUInt(&ok);
        summ += v;
        if(t > maxTarff || t > MAX_TARIFF_COUNT)
            continue;
        hashTmpData.insert(QString("T%1_A+").arg(QString::number(t + 1)), ok ? s : "-");
        hasGoodVal = (hasGoodVal || ok);
    }

    hashTmpData.insert(QString("T%1_A+").arg(QString::number(0)), hasGoodVal ? QString::number(summ) : "-");

    return  hasGoodVal;
}

//-----------------------------------------------------------------------------------------------------

bool MsiEMBeeHelper::decodeEndOfDayValues(const QList<quint8> &meterMess, const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{
    const int maxTarff = MeterPluginHelper::getTariffCount(hashConstData);
    bool hasGoodVal = false;
    const int dayIndx = hashTmpData.value("dayIndx", 0).toInt();

    if(meterMess.isEmpty())
        return false;
    const QDateTime pollDate = hashConstData.value(QString("pollDate_%1").arg(dayIndx)).toDateTime();

    QStringList listDate = hashTmpData.value("listDate").toStringList();
    listDate.append(QString("%1_23_59").arg(pollDate.toString("yyyy_MM_dd")));
    listDate.removeDuplicates();

    const quint8 dataStatus = meterMess.first();
    const QString pref = QString("%1_23_59").arg(pollDate.toString("yyyy_MM_dd"));

    if(dataStatus == MSIEMB_DATA_STTS_NODATA){

        hashTmpData.insert("listDate", listDate);
        for(int t = 0; t <= maxTarff && t < MAX_TARIFF_COUNT; t++)
            hashTmpData.insert(QString("%1_T%2_A+").arg(pref).arg( QString::number(t)), "-");

        return true;

//        hashTmpData.insert(MeterPluginHelper::errWarnKey(warning_counter, false), MeterPluginHelper::prettyMess(tr("EOD: %1, no data").arg(hashTmpData.value("CE303_lastDateStr4err").toString()), MeterPluginHelper::prettyHexDump(meterMess), false, lastErr));

    }

    quint64 summ = 0;
    for(int t = 0, l = 1, lmax = meterMess.size(); l < lmax && t <= maxTarff && t <= MSIEMB_MAX_TARIFF_COUNT; t++, l += 4){
        const QString s = MeterPluginHelper::uint8list2str(meterMess, l, 4);
        bool ok = false;// decodeAsciiLine(meterMess, s, l, 4);
        const quint32 v = (s.isEmpty()) ? 0 : s.toUInt(&ok);
        summ += v;
        if(t > maxTarff || t > MAX_TARIFF_COUNT)
            continue;

        hashTmpData.insert(QString("%1_T%2_A+").arg(pref).arg(QString::number(t + 1)), ok ? s : "-");
        hasGoodVal = (hasGoodVal || ok);
    }
    hashTmpData.insert(QString("%1_T%2_A+").arg(pref).arg(QString::number(0)), hasGoodVal ? QString::number(summ) : "-");

    hashTmpData.insert("listDate", listDate);

    return  hasGoodVal;

}
//-----------------------------------------------------------------------------------------------------
bool MsiEMBeeHelper::isThisData2msiEmbee(const QByteArray &arr, QByteArray &modemni)
{
    QList<quint8> mas;
    int masSize;

    for(int i = 0, iMax = arr.length(); i < iMax; i++)
        mas.append(arr.at(i));

    while(!mas.isEmpty()){
        if(mas.first() != 0xC0){
            mas.removeFirst();
        }else
            break;
    }

    while(!mas.isEmpty()){
        if(mas.last() != 0xC0)
            mas.removeFirst();
        else
            break;
    }

    masSize = mas.size();
    if(masSize < 9)
        return false;

    mas.removeFirst();
    mas.removeLast();
    masSize -= 2;

    if(!Crc8PolyB5::unByteStaffing(mas, masSize))
        return false;

    if(mas.first() != 0xF5)
        return  false;

    const quint8 messCrc8 = mas.takeLast();
    const quint8 crc8 = Crc8PolyB5::calculateCrc4mess(mas);
    if(messCrc8 != crc8)
        return false;

    QByteArray nodeID = QString("%1").arg(mas.takeFirst(),0,16).toLocal8Bit();
    nodeID = nodeID.rightJustified(2,'0');
    nodeID.prepend(QString("%1").arg(mas.takeFirst(),0,16).toLocal8Bit());
    nodeID = nodeID.rightJustified(4,'0');

    for(int i = 0; i < 6; i++) //addr source + passwd
        mas.removeFirst();


    if(mas.isEmpty())
        return  false;

    QByteArray commandCode("");
    QList<quint8> meterMess;

    commandCode.append(mas.takeFirst());
    if(!mas.isEmpty())
        commandCode.append(mas.takeFirst());

    meterMess = mas;
//    int messageMeterSize = 0;

//     if(meterMess.size() == messageMeterSize){
         modemni = nodeID;
         return true;
//     }
//     return false;
}

//-----------------------------------------------------------------------------------------------------

