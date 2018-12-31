#ifndef AMEMORYDEVICE_H
#define AMEMORYDEVICE_H

#include <QObject>
#include <QSet>
/*
 * This class provides a unified interface for memory devices (like RAM, or a cache).
 * It provides concrete methods for singaling errors & error messages,
 * reporting on addresses whose contents changed over a period of time,
 * and reading / writing to memory.
 *
 * One matter of note is the differentiation between read/get and write/set.
 * Read and write are able to trigger side effects (such as creating output in memory-mapped io,
 * or loading a newpage due to a cache miss). Write might also trigger errors,
 * like trying to write to the read-only storage for the operating system.
 *
 * Get and set do not trigger side effects. Get simply reports the value at an address,
 * and set will modify the value. Some chips (like the ConstCHip) do not support any modification, even
 * through set, but some chips like the ROMChip will error if written to, but will succede if set.
 *
 * Both set / write will trigger the changed(...) signal
 *
 * Therefore, programmers should use get / set when interacting with the memory model from the UI,
 * and the logical model operating on memory should use get / set.
 *
 */
class AMemoryDevice : public QObject
{
    Q_OBJECT
protected:
    QSet<quint16> bytesWritten, bytesSet;
    mutable bool error;
    mutable QString errorMessage;
public:
    explicit AMemoryDevice(QObject *parent = nullptr) noexcept;

    // Returns true if a fatal error affected memory.
    bool hadError() const noexcept;
    // If there was an error, return the diagnostic message.
    QString getErrorMessage() const noexcept;
    // Return the number of bytes of memory this unit has access to.
    // Is larger that used a 32 size so that no cleverness is needed to
    // representa range of 0 to 2^16 bytes of memory.
    virtual quint32 size() const noexcept = 0;

    void clearErrors();

    // Returns the set of bytes the have been written / set.
    // since the last clear.
    const QSet<quint16> getBytesWritten() const noexcept;
    const QSet<quint16> getBytesSet() const noexcept;
    // Call after all components have (synchronously) had a chance
    // to access these fields. The set of written / set bytes will
    // continue to grow until explicitly reset.
    void clearBytesWritten() noexcept;
    void clearBytesSet() noexcept;

public slots:
    // Clear the contents of memory. All addresses from 0 to size will be set to 0.
    virtual void clearMemory() = 0;
    virtual void onCycleStarted() = 0;
    virtual void onCycleFinished() = 0;

    // Read / Write functions that may trap for IO or generate errors from writing to readonly storage.
    virtual bool readByte(quint16 address, quint8& output) const = 0;
    virtual bool writeByte(quint16 address, quint8 value) = 0;
    // Read / Write of words as two read / write byte operations and bitmath.
    virtual bool readWord(quint16 address, quint16& output) const;
    virtual bool writeWord(quint16 address, quint16 value);

    // Get / Set functions that are guarenteed not to trap for IO and will not error.
    virtual bool getByte(quint16 address, quint8& output) const = 0;
    virtual bool setByte(quint16 address, quint8 value) = 0;
    // Get / Set of words as two get / set byte operations and bitmath.
    virtual bool getWord(quint16 address, quint16& output) const;
    virtual bool setWord(quint16 address, quint16 value);

signals:
    // Signal that a memory address has been written / set.
    void changed(quint16 address, quint8 newValue);


};

#endif // AMEMORYDEVICE_H
