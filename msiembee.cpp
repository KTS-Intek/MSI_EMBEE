#include "msiembee.h"

#include "shared/myucmmeterstypes.h"
#include "shared/definedpollcodes.h"
#include "msiembeehelper.h"
#include "shared/meterpluginhelper.h"
#include "shared/ucmetereventcodes.h"

#include <QDateTime>


QString MsiEMBee::getMeterType(){ return "MSI-EMBEE"; }

quint16 MsiEMBee::getPluginVersion(){ return PLG_VER_RELEASE; }

QString MsiEMBee::getMeterAddrsAndPsswdRegExp(){ return QString("^(0|[1-9][0-9]{3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"
                                                                "^(0|[1-9][0-9]{3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5])$"); }

quint8 MsiEMBee::getPasswdType(){ return  UCM_PSWRD_TEXT; }

QString MsiEMBee::getVersion(){ return QString("MSI EMBEE v0.1 26.06.2018"); }

QByteArray MsiEMBee::getDefPasswd(){ return MsiEMBeeHelper::getDefPasswd(""); }

QString MsiEMBee::getSupportedMeterList(){ return "MSI EMBEE v0.1"; }

quint8 MsiEMBee::getMaxTariffNumber(const QString &version){ Q_UNUSED(version); return 4; }

QStringList MsiEMBee::getEnrgList4thisMeter(const quint8 &pollCode, const QString &version){ return MsiEMBeeHelper::getEnrgList4thisMeter(pollCode, version);  }

quint8 MsiEMBee::getMeterTypeTag(){ return  UC_METER_ELECTRICITY; }

//----------------------------------------------------------------------------------------

Mess2meterRezult MsiEMBee::mess2meter(const Mess2meterArgs &pairAboutMeter)
{
    const QVariantHash hashConstData = pairAboutMeter.hashConstData;
    QVariantHash hashTmpData = pairAboutMeter.hashTmpData;
    QVariantHash hashMessage;
    bool verbouseMode = hashConstData.value("verbouseMode").toBool();
    quint8 pollCode = quint8(hashConstData.value("pollCode").toUInt());

    if(!MsiEMBeeHelper::isPollCodeSupported(pollCode, hashTmpData))
        return Mess2meterRezult(hashMessage,hashTmpData);

    if(hashTmpData.value("vrsn").toString().isEmpty() && !hashConstData.value("vrsn").toString().isEmpty() && getSupportedMeterList().split(",").contains(hashConstData.value("vrsn").toString(), Qt::CaseInsensitive))
        hashTmpData.insert("vrsn", hashConstData.value("vrsn").toString());


    if(verbouseMode)
        qDebug() << hashTmpData.value("vrsn").toString() ;


    if(hashTmpData.value("corrDateTime", false).toBool()){
        if(verbouseMode)
            qDebug() << "CE102 write dateTime";
        hashMessage.insert("message_0", MsiEMBeeHelper::writeMeterDateTime(hashConstData.value("NI", QByteArray()).toByteArray() , hashConstData.value("passwd", QByteArray()).toByteArray(), hashTmpData));
        hashMessage.insert("endSymb", QByteArray::fromHex("C0"));
        return Mess2meterRezult(hashMessage,hashTmpData);
    }

    if(pollCode >= POLL_CODE_READ_METER_LOGBOOK && pollCode <= POLL_CODE_READ_END_MONTH && pollCode % 20 == 0){
        quint32 step = hashTmpData.value("step", 0).toUInt();
        QByteArray commandh;

        if(step < 6){
            if(step == 0 && !hashTmpData.value("vrsn").toString().isEmpty())
                step = 1;

            if(step == 1 && !hashTmpData.value("SN").toString().isEmpty())
                step = 2;


            switch (step) {
            case 1  : commandh = "30"; break;//read S/N
            case 2  : commandh = "32"; break;//read DT

            default : commandh = "31"; break;//version

            }
            hashMessage.insert("message_0", MsiEMBeeHelper::crcCalc(hashConstData.value("NI").toByteArray(), hashConstData.value("passwd").toByteArray(), commandh, "", hashTmpData));
            hashTmpData.insert("step", step);

            hashMessage.insert("endSymb", QByteArray::fromHex("C0"));
            return Mess2meterRezult(hashMessage,hashTmpData);

        }
        QByteArray dataarr;
        switch (pollCode) {
        case POLL_CODE_READ_TOTAL   :{ commandh = "43"; break;}
        case POLL_CODE_READ_END_DAY :{
            const int dayIndx = hashTmpData.value("dayIndx", 0).toInt();
            const QDateTime dt = hashConstData.value(QString("pollDate_%1").arg(dayIndx)).toDateTime();
            if(dayIndx >= 0 && dt.isValid()){
                commandh = "44";
                dataarr = MsiEMBeeHelper::getDateTimeBcd(dt).right(3);
            }
            break;}
        }
        hashMessage.insert("message_0", MsiEMBeeHelper::crcCalc(hashConstData.value("NI").toByteArray(), hashConstData.value("passwd").toByteArray(), commandh, dataarr, hashTmpData));

    }
    if(hashMessage.isEmpty())
        hashTmpData.insert("step", 0xFFFF);
    else
        hashMessage.insert("endSymb", QByteArray::fromHex("C0"));

    return Mess2meterRezult(hashMessage,hashTmpData);

}

//----------------------------------------------------------------------------------------

QVariantHash MsiEMBee::decodeMeterData(const DecodeMeterMess &threeHash)
{
    const QVariantHash hashConstData = threeHash.hashConstData;
    const QVariantHash hashRead = threeHash.hashRead;
    QVariantHash hashTmpData = threeHash.hashTmpData;

    int error_counter = qMax(0, hashTmpData.value("error_counter", 0).toInt());

//    int warning_counter = qMax(0, hashTmpData.value("warning_counter", 0).toInt());
    const bool verbouseMode = hashConstData.value("verbouseMode").toBool();


    hashTmpData.insert("messFail", true);
    quint8 pollCode = quint8(hashConstData.value("pollCode").toUInt());


    QByteArray commandCode;
    QList<quint8> meterMess;
    QString errMess;

    const bool isDateTimeCorrectionNow = hashTmpData.value("corrDateTime", false).toBool();

    if(!MsiEMBeeHelper::messageIsValid(hashConstData.value("NI").toByteArray(), hashRead.value("readArr_0").toByteArray(), verbouseMode, commandCode, errMess, meterMess)){
        if(verbouseMode)
            qDebug() << "if(!messageIsValid(nodeID, readArr, commandCode, meterMess)){";
        hashTmpData.insert(MsiEMBeeHelper::errWarnKey(error_counter, true), MsiEMBeeHelper::prettyMess(errMess, QString(", " + hashRead.value("readArr_0").toByteArray().toHex())));

        if(isDateTimeCorrectionNow){
            hashTmpData.insert(MeterPluginHelper::nextMatildaEvntName(hashTmpData), MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_NOT_CORR, QDateTime::currentDateTimeUtc(),
                                                                              tr("Correct date: fail") ));
            hashTmpData.insert(MsiEMBeeHelper::errWarnKey(error_counter, true), MsiEMBeeHelper::prettyMess(tr("Correct date: fail"), ""));
        }
        hashTmpData.insert("error_counter", error_counter);

        return hashTmpData;
    }



    if(isDateTimeCorrectionNow){

        hashTmpData.insert("step", 0);


        hashTmpData.insert("corrDateTime", false);
        hashTmpData.insert("messFail", false);
        hashTmpData.insert(MeterPluginHelper::nextMatildaEvntName(hashTmpData), MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_CORR_DONE, QDateTime::currentDateTimeUtc(),
                                                                          tr("Meter new date %1 UTC%2%3")
                                                                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
                                                                          .arg( (QDateTime::currentDateTime().offsetFromUtc() >= 0) ? "+" : "" )
                                                                          .arg(QString::number(QDateTime::currentDateTime().offsetFromUtc()))) );


        return hashTmpData;
    }



    if(pollCode >= POLL_CODE_READ_METER_LOGBOOK && pollCode <= POLL_CODE_READ_END_MONTH && pollCode % 20 == 0){
        quint32 step = hashTmpData.value("step", 0).toUInt();



        bool goodRez = false;
        QByteArray commandh;
        if(step < 6){
            switch (step) {
            case 1  : goodRez = MsiEMBeeHelper::decodeSerialNumber(meterMess, hashTmpData)              ; commandh = "30"; break;//read S/N
            case 2  : goodRez = MsiEMBeeHelper::decodeDateTime(meterMess, hashConstData, hashTmpData)   ; commandh = "32"; break;//read DT
            default : step = 0; goodRez = MsiEMBeeHelper::decodeVersion(meterMess, hashTmpData)         ; commandh = "31"; break;//version
            }
        }else{
            switch (pollCode) {
            case POLL_CODE_READ_TOTAL:{
                commandh = "43";
                goodRez = MsiEMBeeHelper::decodeTotalValues(meterMess, hashConstData, hashTmpData);
                if(goodRez)
                    step = 0xFFFF - 1;
                break;}
            case POLL_CODE_READ_END_DAY:{
                commandh = "44";
                goodRez = MsiEMBeeHelper::decodeEndOfDayValues(meterMess, hashConstData, hashTmpData);
                if(goodRez){
                    const int dayIndx = hashTmpData.value("dayIndx", 0).toInt() + 1;
                     hashTmpData.insert("dayIndx", dayIndx);

                    if(!hashConstData.value(QString("pollDate_%1").arg(dayIndx)).toDateTime().isValid())
                       step = 0xFFFF;

                }
                break;}
            }
        }
        if(goodRez)
            commandh = QByteArray::fromHex(commandh);

        if(goodRez && commandCode == commandh && step != 0xFFFF)
            step++;
        if(step == 3)
            step = 6;//go to the next unit
        hashTmpData.insert("step", step);
        hashTmpData.insert("messFail", !goodRez);

    }

    return hashTmpData;


}

//----------------------------------------------------------------------------------------

QVariantHash MsiEMBee::helloMeter(const QVariantHash &hashMeterConstData)
{
    return  MsiEMBeeHelper::helloMeter(hashMeterConstData);
}

//----------------------------------------------------------------------------------------

QString MsiEMBee::meterTypeFromMessage(const QByteArray &readArr)
{
    QByteArray commandCode;
    QList<quint8> meterMess;
    QString errMess;

    const bool verbouseMode = true;

    if(MsiEMBeeHelper::messageIsValid("", readArr, verbouseMode, commandCode, errMess, meterMess)){
        return getMeterType();
    }
    return "";


}

//----------------------------------------------------------------------------------------

QVariantHash MsiEMBee::isItYour(const QByteArray &arr)
{
    QByteArray modemni;
    if(MsiEMBeeHelper::isThisData2msiEmbee(arr, modemni) && !modemni.isEmpty()){
        QVariantHash hash;
        hash.insert("nodeID", modemni);
        hash.insert("endSymb", QByteArray::fromHex("C0"));
        hash.insert("readLen", 9);
        return hash;

    }
    return QVariantHash();

}

//----------------------------------------------------------------------------------------

QVariantHash MsiEMBee::isItYourRead(const QByteArray &arr)
{
    QByteArray commandCode;
    QList<quint8> meterMess;
    QString errMess;

    const bool verbouseMode = true;

    if(MsiEMBeeHelper::messageIsValid("FFFF", arr, verbouseMode, commandCode, errMess, meterMess)){
        QVariantHash hash;
        hash.insert("Tak", true);
        return hash;
    }
    return QVariantHash();
}

//----------------------------------------------------------------------------------------

QByteArray MsiEMBee::niChanged(const QByteArray &arr)
{
    Q_UNUSED(arr);
    return QByteArray();
}

//----------------------------------------------------------------------------------------

QVariantHash MsiEMBee::meterSn2NI(const QString &meterSn)
{
    /*
     * hard - без варіантів (жорстко), від старого до нового ф-ту
     * altr - альтернатива для стандартного варіанту
     * <keys> : <QStringList>
     */
    QVariantHash h;

    QStringList listNI;
    bool ok;
    meterSn.right(4).toInt(&ok);
    if(ok)
        listNI.append(QString::number(meterSn.right(4).toInt()));

    meterSn.right(5).toInt(&ok);
    if(ok && meterSn.right(4).toInt() != meterSn.right(5).toInt() && meterSn.right(5).toInt() < 0xFFFF )
        listNI.append(QString::number(meterSn.right(5).toInt()));


    if(!listNI.isEmpty())
        h.insert("altr", listNI);

    return h;
}

//----------------------------------------------------------------------------------------

Mess2meterRezult MsiEMBee::messParamPamPam(const Mess2meterArgs &pairAboutMeter)
{
    const QVariantHash hashConstData = pairAboutMeter.hashConstData;
    QVariantHash hashTmpData = pairAboutMeter.hashTmpData;
    QVariantHash hashMessage;
//    const bool verbouseMode = hashConstData.value("verbouseMode").toBool();

    quint8 pollCode = quint8(hashConstData.value("pollCode").toUInt());
    if(hashTmpData.value("vrsn").toString().isEmpty() && !hashConstData.value("vrsn").toString().isEmpty() && getSupportedMeterList().split(",").contains(hashConstData.value("vrsn").toString(), Qt::CaseInsensitive))
        hashTmpData.insert("vrsn", hashConstData.value("vrsn").toString());

    switch(pollCode){



    case POLL_CODE_WRITE_DATE_TIME:{

        if(!hashTmpData.value("MSI_dateTimeDone").toBool()){
            hashTmpData.insert("corrDateTime", true);
            hashMessage.insert("message_0", MsiEMBeeHelper::writeMeterDateTime(hashConstData.value("NI", QByteArray()).toByteArray() , hashConstData.value("passwd", QByteArray()).toByteArray(), hashTmpData));
            break;}
        }
    case POLL_CODE_READ_DATE_TIME_DST : hashMessage.insert("message_0", MsiEMBeeHelper::crcCalc(hashConstData.value("NI").toByteArray(), hashConstData.value("passwd").toByteArray(), "32", "", hashTmpData)); break;


    default: hashTmpData.insert("notsup", true); hashTmpData.insert("step", 0xFFFF); break;
    }

    if(!hashMessage.isEmpty())
        hashMessage.insert("endSymb", QByteArray::fromHex("C0"));

    return Mess2meterRezult(hashMessage,hashTmpData);


}
//----------------------------------------------------------------------------------------
QVariantHash MsiEMBee::decodeParamPamPam(const DecodeMeterMess &threeHash)
{
    const QVariantHash hashConstData = threeHash.hashConstData;
    const QVariantHash hashRead = threeHash.hashRead;

    QVariantHash hashTmpData = threeHash.hashTmpData;
    int error_counter = qMax(0, hashTmpData.value("error_counter", 0).toInt());

    hashTmpData.insert("messFail", true);
    quint8 pollCode = quint8(hashConstData.value("pollCode").toUInt());

    QByteArray commandCode;
    QList<quint8> meterMess;
    QString errMess;
    const bool verbouseMode = hashConstData.value("verbouseMode").toBool();

    const bool isDateTimeCorrectionNow = hashTmpData.value("corrDateTime", false).toBool();

    if(!MsiEMBeeHelper::messageIsValid(hashConstData.value("NI").toByteArray(), hashRead.value("readArr_0").toByteArray(), verbouseMode, commandCode, errMess, meterMess)){
        if(verbouseMode)
            qDebug() << "if(!messageIsValid(nodeID, readArr, commandCode, meterMess)){";
        hashTmpData.insert(MsiEMBeeHelper::errWarnKey(error_counter, true), MsiEMBeeHelper::prettyMess(errMess, QString(", " + hashRead.value("readArr_0").toByteArray().toHex())));

        if(isDateTimeCorrectionNow){
            hashTmpData.insert(MeterPluginHelper::nextMatildaEvntName(hashTmpData), MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_NOT_CORR, QDateTime::currentDateTimeUtc(),
                                                                              tr("Correct date: fail") ));
            hashTmpData.insert(MsiEMBeeHelper::errWarnKey(error_counter, true), MsiEMBeeHelper::prettyMess(tr("Correct date: fail"), ""));
        }
        hashTmpData.insert("error_counter", error_counter);

        return hashTmpData;
    }

    switch (pollCode) {
     case POLL_CODE_WRITE_DATE_TIME:{
         if(!hashTmpData.value("MSI_dateTimeDone").toBool()){
             hashTmpData.insert("messFail", false);
             hashTmpData.insert("MSI_dateTimeDone", true);
             break;
         }
        }
    case POLL_CODE_READ_DATE_TIME_DST :
        if(MsiEMBeeHelper::decodeDateTime(meterMess, hashConstData, hashTmpData)){

            hashTmpData.insert("SummerTime", QString("3,0,%1,1")//Місяць,День тижня,Година,Переведення на г годин
                               .arg(QString::number(3))
                               );

            hashTmpData.insert("NormalTime", QString("10,0,%1,1")
                               .arg(QString::number(3 + 1))
                               );
            hashTmpData.insert("DST_Profile",  false);

            hashTmpData.insert("messFail", false);


            hashTmpData.insert("step", 0xFFFF);
        }
    }


    return hashTmpData;


}

QVariantHash MsiEMBee::how2logout(const QVariantHash &hashConstData, const QVariantHash &hashTmpData){    Q_UNUSED(hashConstData);    Q_UNUSED(hashTmpData);    return QVariantHash();}

QVariantHash MsiEMBee::getDoNotSleep(const quint8 &minutes){    Q_UNUSED(minutes);    return QVariantHash();}

QVariantHash MsiEMBee::getGoToSleep(const quint8 &seconds){    Q_UNUSED(seconds);    return QVariantHash();}
