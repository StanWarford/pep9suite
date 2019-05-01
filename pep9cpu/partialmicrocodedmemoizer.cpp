#include "partialmicrocodedmemoizer.h"
#include "partialmicrocodedcpu.h"
#include "pep.h"
#include "amemorydevice.h"
#include "cpudata.h"
#include "registerfile.h"
#include <assert.h>
#include <QString>
#include <QtCore>
#include <QDebug>
const QString stackFrameEnter("%1\n===CALL===\n");
const QString stackFrameLeave("%1\n===RET====\n");
const QString trapEnter("%1\n===TRAP===\n");
const QString trapLeave("%1\n===RETR===\n");


PartialMicrocodedMemoizer::PartialMicrocodedMemoizer(PartialMicrocodedCPU& item): cpu(item)
{
}

void PartialMicrocodedMemoizer::clear()
{
}

void PartialMicrocodedMemoizer::storeStateInstrEnd()
{

}

void PartialMicrocodedMemoizer::storeStateInstrStart()
{

}

QString PartialMicrocodedMemoizer::memoize()
{
    QString build, AX, NZVC;
    const RegisterFile& file = cpu.data->getRegisterBank();
    AX = QString(" A=%1, X=%2, SP=%3, ")

            .arg(formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::A)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::X)),
                 formatNum(file.readRegisterWordCurrent(Enu::CPURegisters::SP)));
    NZVC = QString(" SNZVC=") % QString("%1").arg(QString::number(file.readStatusBitsCurrent(), 2), 5, '0');
    build = AX;
    build += NZVC;
    return build;
}

QString PartialMicrocodedMemoizer::finalStatistics()
{
    QString output = "";
    return output;
}

//Properly formats a number as a 4 char hex
QString PartialMicrocodedMemoizer::formatNum(quint16 number)
{
    return QString("%1").arg(QString::number(number,16),4,'0').toUpper();
}

//Properly format a number as 2 char hex
QString PartialMicrocodedMemoizer::formatNum(quint8 number)
{
    return QString("%1").arg(QString::number(number,16),2,'0').toUpper();
}
