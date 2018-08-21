#include "crc8polyb5.h"
#include <QDebug>
#include <QBitArray>

#define DEF_FIRST_BYTE  0xFD
#define DEF_SECOND_BYTE 0x00

//----------------------------------------------------------------------------------------------

QByteArray Crc8PolyB5::crcCalc(const QByteArray &nodeID, const QByteArray &passwd, const QByteArray &dataArr)
{
    return  crcCalcExt(0x48, nodeID, passwd, QByteArray(), dataArr);

}

//----------------------------------------------------------------------------------------------

QByteArray Crc8PolyB5::crcCalcMSI(const QByteArray &nodeID, const QByteArray &passwd, const QByteArray &commandh, const QByteArray &dataArr)
{
    return  crcCalcExt(0x0, nodeID, commandh, passwd.rightJustified(8,'0'), dataArr);

}

//----------------------------------------------------------------------------------------------

QByteArray Crc8PolyB5::crcCalcExt(const quint8 &optByte, const QByteArray &nodeID, const QByteArray &argsLeftH, const QByteArray &argsRightH, const QByteArray &dataArr)
{
    //{0x00, 0x48, 0x00, 0x00, 0xFD, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD2, 0x01, 0x16, 0x00, 0x00, 0x00, 0x00};
    // read meter type      {0x00, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD0, 0x01, 0x00, 0x00, 0x00};
    QList<quint8> mas;

    mas.append(optByte);

    if((dataArr.isEmpty() && argsLeftH.isEmpty()) || nodeID.isEmpty())
        return QByteArray();

    bool okNodeID;
    const quint16 nodeIDint = nodeID.toUShort(&okNodeID);
    if(!okNodeID)
        return QByteArray();


    QByteArray nodeIDhex;
    nodeIDhex.setNum(nodeIDint, 16); //7
    nodeIDhex = nodeIDhex.rightJustified(4,'0').toUpper(); //0007
    //    nodeIDhex = nodeIDhex.right(2) + nodeIDhex.left(2);//0700

//    QByteArray lastAddr;
    mas.append(quint8(nodeIDhex.right(2).toUShort(nullptr,16)));
//    lastAddr.append(mas.last());
    mas.append(quint8(nodeIDhex.left(2).toUShort(nullptr,16)));
//    lastAddr.append(mas.last());


    if(optByte == 0){
        mas.append(0xFE);
        mas.append(0xFF);
    }else{
        if(nodeIDint != 253)  //source addr
            mas.append(0xfd);
        else
            mas.append(0xfe);
        mas.append(0x00);
    }


    if(!addArgs2mas(mas, argsLeftH) || !addArgs2mas(mas, argsRightH))
        return QByteArray();

//    quint32 passwdUint = argsLeft.toUInt(&okNodeID);
//    if(!okNodeID)
//        return QByteArray();

//    QByteArray pravylnyjPasswd;
//    pravylnyjPasswd.setNum(passwdUint,16);
//    pravylnyjPasswd = pravylnyjPasswd.rightJustified(8,'0');

//    for(int i = 3; i >= 0; i--)
//        mas.append((quint8)pravylnyjPasswd.mid(i * 2, 2).toUShort(nullptr,16));



    for(int i = 0, iMax = dataArr.length(); i < iMax; i++)
        mas.append(quint8(dataArr.at(i)));

    mas.append(calculateCrc4mess(mas));

    for(int i = 0, iMax = mas.size(); i < iMax; i++){
        if(mas.at(i) == 0xDB){
            mas.replace(i, 0xDD);
            mas.insert(i, 0xDB);
            iMax++;
        }
    }

    while(mas.contains(0xC0)){
        int index = mas.indexOf(0xC0);
        mas.replace(index, 0xDC);
        mas.insert(index, 0xDB);
    }

    mas.prepend(0xC0);
    mas.append(0xC0);

    QByteArray dataWrite = "";
    for(int i = 0, iMax = mas.size(); i < iMax; i++)
        dataWrite.append(mas.at(i));
    return dataWrite;
}

//----------------------------------------------------------------------------------------------

quint8 Crc8PolyB5::calculateCrc4mess(const QList<quint8> &mas)
{
    const quint8 crc8tab[256] = {
        0x00, 0xb5, 0xdf, 0x6a, 0x0b, 0xbe, 0xd4, 0x61, 0x16, 0xa3, 0xc9, 0x7c, 0x1d, 0xa8, 0xc2, 0x77,
        0x2c, 0x99, 0xf3, 0x46, 0x27, 0x92, 0xf8, 0x4d, 0x3a, 0x8f, 0xe5, 0x50, 0x31, 0x84, 0xee, 0x5b,
        0x58, 0xed, 0x87, 0x32, 0x53, 0xe6, 0x8c, 0x39, 0x4e, 0xfb, 0x91, 0x24, 0x45, 0xf0, 0x9a, 0x2f,
        0x74, 0xc1, 0xab, 0x1e, 0x7f, 0xca, 0xa0, 0x15, 0x62, 0xd7, 0xbd, 0x08, 0x69, 0xdc, 0xb6, 0x03,
        0xb0, 0x05, 0x6f, 0xda, 0xbb, 0x0e, 0x64, 0xd1, 0xa6, 0x13, 0x79, 0xcc, 0xad, 0x18, 0x72, 0xc7,
        0x9c, 0x29, 0x43, 0xf6, 0x97, 0x22, 0x48, 0xfd, 0x8a, 0x3f, 0x55, 0xe0, 0x81, 0x34, 0x5e, 0xeb,
        0xe8, 0x5d, 0x37, 0x82, 0xe3, 0x56, 0x3c, 0x89, 0xfe, 0x4b, 0x21, 0x94, 0xf5, 0x40, 0x2a, 0x9f,
        0xc4, 0x71, 0x1b, 0xae, 0xcf, 0x7a, 0x10, 0xa5, 0xd2, 0x67, 0x0d, 0xb8, 0xd9, 0x6c, 0x06, 0xb3,
        0xd5, 0x60, 0x0a, 0xbf, 0xde, 0x6b, 0x01, 0xb4, 0xc3, 0x76, 0x1c, 0xa9, 0xc8, 0x7d, 0x17, 0xa2,
        0xf9, 0x4c, 0x26, 0x93, 0xf2, 0x47, 0x2d, 0x98, 0xef, 0x5a, 0x30, 0x85, 0xe4, 0x51, 0x3b, 0x8e,
        0x8d, 0x38, 0x52, 0xe7, 0x86, 0x33, 0x59, 0xec, 0x9b, 0x2e, 0x44, 0xf1, 0x90, 0x25, 0x4f, 0xfa,
        0xa1, 0x14, 0x7e, 0xcb, 0xaa, 0x1f, 0x75, 0xc0, 0xb7, 0x02, 0x68, 0xdd, 0xbc, 0x09, 0x63, 0xd6,
        0x65, 0xd0, 0xba, 0x0f, 0x6e, 0xdb, 0xb1, 0x04, 0x73, 0xc6, 0xac, 0x19, 0x78, 0xcd, 0xa7, 0x12,
        0x49, 0xfc, 0x96, 0x23, 0x42, 0xf7, 0x9d, 0x28, 0x5f, 0xea, 0x80, 0x35, 0x54, 0xe1, 0x8b, 0x3e,
        0x3d, 0x88, 0xe2, 0x57, 0x36, 0x83, 0xe9, 0x5c, 0x2b, 0x9e, 0xf4, 0x41, 0x20, 0x95, 0xff, 0x4a,
        0x11, 0xa4, 0xce, 0x7b, 0x1a, 0xaf, 0xc5, 0x70, 0x07, 0xb2, 0xd8, 0x6d, 0x0c, 0xb9, 0xd3, 0x66
    };
    quint8  crc8 = 0;
    for(int i = 0, iMax = mas.size(); i < iMax; i++)
        crc8 = crc8tab[crc8 ^ mas.at(i)];
    return  crc8;
}

//----------------------------------------------------------------------------------------------

bool Crc8PolyB5::addArgs2mas(QList<quint8> &mas, const QByteArray &argsh)
{
    if(argsh.isEmpty())
        return true;

    const QByteArray args = QByteArray::fromHex(argsh);
    for(int i = 0, imax = args.length(); i < imax; i++)
        mas.append(quint8(args.at(i)));
    return true;
}

//----------------------------------------------------------------------------------------------

bool Crc8PolyB5::messageIsValid(const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, QList<quint8> &meterMess)
{
    quint8 codeAnswr;
    return  messageIsValidExt(false, nodeID, readArr, verbouseMode, commandCode, codeAnswr, meterMess);
}
//----------------------------------------------------------------------------------------------
bool Crc8PolyB5::messageIsValidMci(const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, quint8 &codeAnswr, QList<quint8> &meterMess)
{
    return  messageIsValidExt(true, nodeID, readArr, verbouseMode, commandCode, codeAnswr, meterMess);
}

//----------------------------------------------------------------------------------------------

bool Crc8PolyB5::messageIsValidExt(const bool &isMci, const QByteArray &nodeID, const QByteArray &readArr, const bool &verbouseMode, QByteArray &commandCode, quint8 &codeAnswr, QList<quint8> &meterMess)
{
    //readArr = C0 48 FD 00 07 00 57 01 20 59 47 10 02 21 01 14 FC C0
    // C0 54 FD 00 06 00 50 31 02 00 4C 00 0C 09 60 00
    bool okNodeID;
    const quint16 nodeIDint = quint16(nodeID.toUInt(&okNodeID));
    if(!okNodeID && !nodeID.isEmpty())
        return false;
    if(nodeIDint < 1 && !nodeID.isEmpty())
        return false;
    QByteArray nodeIDhex;
    nodeIDhex.setNum(nodeIDint, 16); //7
    nodeIDhex = nodeIDhex.rightJustified(4,'0').toUpper(); //0007
    //    nodeIDhex = nodeIDhex.right(2) + nodeIDhex.left(2);//0700

    QList<quint8> mas;
    int masSize;

    for(int i = 0, iMax = readArr.length(); i < iMax; i++)
        mas.append(quint8(readArr.at(i)));

    //mas = C0 48 FD 00 07 00 57 01 20 59 47 10 02 21 01 14 FC C0
    //C0 54 FD 00 06 00 50 31 02 00 4C 00 0C 09 60 00

    while(!mas.isEmpty()){
        if(mas.first() != 0xC0)
            mas.removeFirst();
        else
            break;
    }

    if(mas.isEmpty()){
        if(verbouseMode)
            qDebug() << "mas empty";
        return false;
    }
    while(!mas.isEmpty()){
        if(mas.last() != 0xC0)
            mas.removeLast();
        else
            break;
    }
    if(mas.isEmpty()){
        if(verbouseMode)
            qDebug() << "mas empty";
        return false;
    }
    masSize = mas.size();
    const int minimumSize = isMci ? 10 : 11;
    if(masSize < minimumSize){
        if(verbouseMode)
            qDebug() << "masSize=" << masSize << mas << readArr;
        return false;
    }


    if(mas.first() != 0xC0 || mas.last() != 0xC0){
        if(verbouseMode)
            qDebug() << "mas.first() != 0xC0 || mas.last() != 0xC0 " << masSize << mas << readArr;
        return false;
    }

    mas.removeFirst();
    mas.removeLast();
    masSize -= 2;
    //mas = 48 FD 00 07 00 57 01 20 59 47 10 02 21 01 14 FC
    //54 FD 00 06 00 50 31 02 00 4C 00 0C 09 60 00

    if(!unByteStaffing(mas, masSize)){
        if(verbouseMode)
            qDebug() << "byte db=" << mas;
        return  false;
    }


    const bool isCrc8 = (mas.first() == 0x48 || isMci);
    if(isCrc8){ //isCrc8
        quint8 messCrc8 = mas.takeLast();
        const quint8 crc8tab[256] = {
            0x00, 0xb5, 0xdf, 0x6a, 0x0b, 0xbe, 0xd4, 0x61, 0x16, 0xa3, 0xc9, 0x7c, 0x1d, 0xa8, 0xc2, 0x77,
            0x2c, 0x99, 0xf3, 0x46, 0x27, 0x92, 0xf8, 0x4d, 0x3a, 0x8f, 0xe5, 0x50, 0x31, 0x84, 0xee, 0x5b,
            0x58, 0xed, 0x87, 0x32, 0x53, 0xe6, 0x8c, 0x39, 0x4e, 0xfb, 0x91, 0x24, 0x45, 0xf0, 0x9a, 0x2f,
            0x74, 0xc1, 0xab, 0x1e, 0x7f, 0xca, 0xa0, 0x15, 0x62, 0xd7, 0xbd, 0x08, 0x69, 0xdc, 0xb6, 0x03,
            0xb0, 0x05, 0x6f, 0xda, 0xbb, 0x0e, 0x64, 0xd1, 0xa6, 0x13, 0x79, 0xcc, 0xad, 0x18, 0x72, 0xc7,
            0x9c, 0x29, 0x43, 0xf6, 0x97, 0x22, 0x48, 0xfd, 0x8a, 0x3f, 0x55, 0xe0, 0x81, 0x34, 0x5e, 0xeb,
            0xe8, 0x5d, 0x37, 0x82, 0xe3, 0x56, 0x3c, 0x89, 0xfe, 0x4b, 0x21, 0x94, 0xf5, 0x40, 0x2a, 0x9f,
            0xc4, 0x71, 0x1b, 0xae, 0xcf, 0x7a, 0x10, 0xa5, 0xd2, 0x67, 0x0d, 0xb8, 0xd9, 0x6c, 0x06, 0xb3,
            0xd5, 0x60, 0x0a, 0xbf, 0xde, 0x6b, 0x01, 0xb4, 0xc3, 0x76, 0x1c, 0xa9, 0xc8, 0x7d, 0x17, 0xa2,
            0xf9, 0x4c, 0x26, 0x93, 0xf2, 0x47, 0x2d, 0x98, 0xef, 0x5a, 0x30, 0x85, 0xe4, 0x51, 0x3b, 0x8e,
            0x8d, 0x38, 0x52, 0xe7, 0x86, 0x33, 0x59, 0xec, 0x9b, 0x2e, 0x44, 0xf1, 0x90, 0x25, 0x4f, 0xfa,
            0xa1, 0x14, 0x7e, 0xcb, 0xaa, 0x1f, 0x75, 0xc0, 0xb7, 0x02, 0x68, 0xdd, 0xbc, 0x09, 0x63, 0xd6,
            0x65, 0xd0, 0xba, 0x0f, 0x6e, 0xdb, 0xb1, 0x04, 0x73, 0xc6, 0xac, 0x19, 0x78, 0xcd, 0xa7, 0x12,
            0x49, 0xfc, 0x96, 0x23, 0x42, 0xf7, 0x9d, 0x28, 0x5f, 0xea, 0x80, 0x35, 0x54, 0xe1, 0x8b, 0x3e,
            0x3d, 0x88, 0xe2, 0x57, 0x36, 0x83, 0xe9, 0x5c, 0x2b, 0x9e, 0xf4, 0x41, 0x20, 0x95, 0xff, 0x4a,
            0x11, 0xa4, 0xce, 0x7b, 0x1a, 0xaf, 0xc5, 0x70, 0x07, 0xb2, 0xd8, 0x6d, 0x0c, 0xb9, 0xd3, 0x66
        };
        quint8  crc8 = 0;
        for(int i = 0, iMax = mas.size(); i < iMax; i++)
            crc8 = crc8tab[crc8 ^ mas.at(i)];

        if(crc8 != messCrc8){
            if(verbouseMode)
                qDebug() << "crc8 != messCrc8 " << messCrc8 << crc8;
            return false;
        }
    }else{
        if(verbouseMode)
            qDebug() << "unknown crc " << mas.first();
        return false;

    }
    const quint8 optByte = mas.takeFirst();
    //mas = FD 00 07 00 57 01 20 59 47 10 02 21 01 14
    //FD 00 06 00 50 31 02 00 4C 00 0C 09 60 00

    if(isMci){
        if(mas.takeFirst() != 0xfe || mas.takeFirst() != 0xFF){
            if(verbouseMode)
                qDebug() << "dest addr != 0xfe 0xFF";
            return false;
        }
    }else{
        if(nodeIDint != 253){  //source addr
            if(mas.takeFirst() != 0xfd || mas.takeFirst() != 0x0){
                if(verbouseMode)
                    qDebug() << "dest addr != 0xfd 0x0";
                return false;
            }
        }else{
            if(mas.takeFirst() != 0xfe || mas.takeFirst() != 0x0){
                if(verbouseMode)
                    qDebug() << "dest addr != 0xfe 0x0";
                return false;
            }
        }
    }
    //mas = 07 00 57 01 20 59 47 10 02 21 01 14
    //06 00 50 31 02 00 4C 00 0C 09 60 00

    if(nodeID.isEmpty()){//ignore some NI
        mas.removeFirst();
        mas.removeFirst();
    }else{
        if(mas.takeFirst() != quint8(nodeIDhex.right(2).toUInt(nullptr,16)) || mas.takeFirst() != quint8(nodeIDhex.left(2).toUInt(nullptr,16))){
            if(verbouseMode)
                qDebug() << "nodeID != " << nodeIDhex;

            return false;
        }
    }
    //mas = 57 01 20 59 47 10 02 21 01 14
    //mas = 50 31 02 00 4C 00 0C 09 60 00

    int messageMeterSize = 0;
    if(isMci){
        messageMeterSize = int(optByte);
    }else{
        if(isCrc8){
            QByteArray servArr;
            servArr.append(mas.takeFirst());

            QBitArray bitArr(8);

            for(int i = 0, iMax = 1; i < iMax; ++i){
                for(int b = 0; b < 8; ++b)
                    bitArr.setBit(i * 8 + b, servArr.at(i) & (1 << b));
            }
            if(bitArr.at(7) != false){
                if(verbouseMode)
                    qDebug() << "bitArr.at(0) != false " << bitArr;
                return false;
            }

            if(bitArr.at(6) != true || bitArr.at(5) != false || bitArr.at(4) != true){
                if(verbouseMode)
                    qDebug() << "bitArr.at(6) != true || bitArr.at(5) != false || bitArr.at(4) != true" << bitArr;

                if(!(bitArr.at(6) == true && bitArr.at(5) == true && bitArr.at(4) == true))
                    return false;
                //else has err
            }

            for(int i = 0, j = 1; i < 4; i++, j *= 2){
                if(bitArr.at(i))
                    messageMeterSize += j;
            }

        }
    }
    //mas = 01 20 59 47 10 02 21 01 14
    //mas = 02 00 4C 00 0C 09 60 00

    commandCode.clear();
    meterMess.clear();

    if(mas.isEmpty()){
        if(verbouseMode)
            qDebug() << "if(!mas.isEmpty())" << mas;
        return false;
    }


    commandCode.append(mas.takeFirst());
    if(isMci){
        codeAnswr = mas.takeFirst();
    }else{
        if(!mas.isEmpty()){
            commandCode.append(mas.takeFirst());
        }
    }

    //mas = 59 47 10 02 21 01 14
    //mas = 4C 00 0C 09 60 00
    meterMess = mas;

    return (meterMess.size() == messageMeterSize);
}
//----------------------------------------------------------------------------------------------
bool Crc8PolyB5::unByteStaffing(QList<quint8> &mas, int &masSize)
{
    for(int i = 0, j = 1; i < masSize; i++, j++){ //48 db dd 00 07 00      48 db dc 00 07 00
        if(mas.at(i) == 0xDB && j < masSize){     //48 db 00 07 00         48 c0 00 07 00
            if(mas.at(j) == 0xDD){
                mas.removeAt(j);
                masSize--;
            }else{
                if(mas.at(j) == 0xDC){
                    mas.replace(i, 0xC0);
                    mas.removeAt(j);
                    masSize--;
                }else{

                    return false;
                }
            }
        }
    }
    return true;
}

//----------------------------------------------------------------------------------------------
