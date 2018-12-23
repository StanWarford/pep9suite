#ifndef MAINMEMORY_H
#define MAINMEMORY_H

#include <QMap>
#include <QObject>
#include <QSharedPointer>
#include <QVector>

#include "amemorydevice.h"
class AMemoryChip;
class NilChip;
class MainMemory : public AMemoryDevice
{
    Q_OBJECT
    QSharedPointer<NilChip> endChip;
    QMap<quint16, QString> inputBuffer;
    QVector<AMemoryChip*> addressToChip;
    QMap<quint16, QSharedPointer<AMemoryChip>> memoryChipMap;
public:
    MainMemory(QObject* parent = nullptr);

    // AMemoryDevice interface
public:
    quint16 size() const override;
    void insertChip(QSharedPointer<AMemoryChip> chip, quint16 address);
    QSharedPointer<AMemoryChip> chipAt(quint16 address);
    QSharedPointer<const AMemoryChip> chipAt(quint16 address) const;
    // Remove the chip containing address and return it
    QSharedPointer<AMemoryChip> removeChip(quint16 address);
    QVector<QSharedPointer<AMemoryChip>> removeAllChips();

public slots:
    void clearMemory() override;
    void onCycleStarted() override;
    void onCycleFinished() override;

    bool readByte(quint8 &output, quint16 address) const override;
    bool writeByte(quint16 address, quint8 value) override;
    bool getByte(quint8 &output, quint16 address) const override;
    bool setByte(quint16 address, quint8 value) override;

    void onInputReceived(quint16 address, quint8 input);
    void onInputReceived(quint16 address, QChar input);
    void onInputReceived(quint16 address, QString input);
    void onInputCanceled(quint16 address);

signals:
    void inputRequested(quint16 address);
    void outputWritten(quint16 address, quint8 value);

protected slots:
    void onChipInputRequested(quint16 address);
    void onChipOutputWritten(quint16 address, quint8 value);

private:
    void calculateAddressToChip();
};

#endif // MAINMEMORY_H
