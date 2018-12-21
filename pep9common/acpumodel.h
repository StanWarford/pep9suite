#ifndef ACPUMODEL_H
#define ACPUMODEL_H

#include <QObject>
class AMemoryDevice;
class ACPUModel: public QObject
{
    Q_OBJECT
public:
    ACPUModel(AMemoryDevice* MemorySection, QObject* parent=nullptr);
    virtual ~ACPUModel();
protected:
    AMemoryDevice* memory;
};

#endif // ACPUMODEL_H
