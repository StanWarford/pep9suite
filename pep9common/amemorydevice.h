#ifndef AMEMORYDEVICE_H
#define AMEMORYDEVICE_H

#include <QObject>

class AMemoryDevice : public QObject
{
    Q_OBJECT
public:
    explicit AMemoryDevice(QObject *parent = nullptr);

signals:

public slots:
};

#endif // AMEMORYDEVICE_H
