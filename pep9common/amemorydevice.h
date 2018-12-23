#ifndef AMEMORYDEVICE_H
#define AMEMORYDEVICE_H

#include <QObject>
#include <QSet>
class AMemoryDevice : public QObject
{
    Q_OBJECT
protected:
    QSet<quint16> bytesWritten, bytesSet;
    mutable bool error;
    mutable QString errorMessage;
public:
    explicit AMemoryDevice(QObject *parent = nullptr);

    bool hadError() const;
    QString getErrorMessage() const;
    virtual quint16 size() const = 0;

    void clearErrors();

    const QSet<quint16> getBytesWritten() const;
    const QSet<quint16> getBytesSet() const;
    void clearBytesWritten();
    void clearBytesSet();

public slots:
    virtual void clearMemory() = 0;
    virtual void onCycleStarted() = 0;
    virtual void onCycleFinished() = 0;

    // Read / Write functions that may generate signals or trap for IO.
    virtual bool readByte(quint8& output, quint16 address) const = 0;
    virtual bool writeByte(quint16 address, quint8 value) = 0;
    // Read / Write of words as two read / write byte operations and bitmath
    virtual bool readWord(quint16& output, quint16 address) const;
    virtual bool writeWord(quint16 address, quint16 value);

    // Get / Set functions that are guarenteed to not generate signals or trap for IO.
    virtual bool getByte(quint8& output, quint16 address) const = 0;
    virtual bool setByte(quint16 address, quint8 value) = 0;
    // Get / Set of words as two get / set byte operations and bitmath
    virtual bool getWord(quint16& output, quint16 address) const;
    virtual bool setWord(quint16 address, quint16 value);

signals:
    void changed(quint16 address, quint8 newValue, quint8 oldValue);

};

#endif // AMEMORYDEVICE_H
