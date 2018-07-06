#ifndef MEMORYSECTION_H
#define MEMORYSECTION_H

#include <QObject>
#include <QVector>
enum IOMode
{
    BATCH, INTERACTIVE
};
class MemorySection : public QObject
{
    Q_OBJECT
public:
    static MemorySection* getInstance();
    //Fetch Values from Memory
    quint8 getMemoryByte(quint16 address, bool useIOPorts) const;
    quint16 getMemoryWord(quint16 address, bool useIOPorts) const; //Uses the same even / odd conventions as Pep9
    const QVector<quint8> getMemory() const; //Returns a shared copy of the memory vector
    bool hadError() const;
    const QString getErrorMessage();

    void setMemoryByte(quint16 address, quint8 value);
    void setMemoryWord(quint16 address, quint16 value);
    void clearMemory() noexcept;
    void clearErrors() noexcept;

    void loadObjectCode(quint16 address, QVector<quint8> values);

signals:
    void memoryChanged(quint16 address,quint8 oldVal, quint8 newVal); //Thrown whenever a memory address is changed
    void charWrittenToOutput(QChar character);
    void charRequestedFromInput() const;

public slots:
    void onSetMemoryByte(quint16 address,quint8 val);
    void onSetMemoryWord(quint16 address,quint16 val); //This doesn't enforce aligned memory access
    void onClearMemory() noexcept;
#pragma message("TODO: Make memory section report on bytes changed")
    void onInstructionFinished() noexcept;

    void onMemorySizeChanged(quint16 maxBytes);
    void onIPortChanged(quint16 newIPort);
    void onOPortChanged(quint16 newIPort);
    void onAppendInBuffer(const QString& newData);
    void onCancelWaiting();

private:
    explicit MemorySection(QObject *parent = nullptr);
    static MemorySection *instance;

    mutable bool waiting;
    mutable bool hadMemoryError = false;
    mutable QString errorMessage = "";
    mutable QString inBuffer;

    quint16 maxBytes, iPort, oPort;
    QVector<quint8> memory;
    //QSet<quint16> modifiedAddresses,inProgressAddresses;
    void initializeMemory();
};

#endif // MEMORYSECTION_H
