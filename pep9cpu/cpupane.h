// File: cpupane.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef CPUPANE_H
#define CPUPANE_H

#include <QWidget>
#include <QGraphicsScene>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

#include "enu.h"
#include "cpugraphicsitems.h"

namespace Ui {
    class CpuPane;
}
class InterfaceMCCPU;
class NewCPUDataSection;
class CpuPane : public QWidget {
    Q_OBJECT
public:
    explicit CpuPane(QWidget *parent = nullptr);
    void init(QSharedPointer<InterfaceMCCPU> cpu, QSharedPointer<NewCPUDataSection> dataSection);
    ~CpuPane() override;

    void highlightOnFocus();
    bool hasFocus();
    void giveFocus();

    void initModel();

    void startDebugging();
    void stopDebugging();

    void setRegister(Enu::ECPUKeywords reg, int value);
    void setRegisterByte(quint8 reg, quint8 value);
    void setStatusBit(Enu::ECPUKeywords bit, bool value);

    void setRegPrecondition(Enu::ECPUKeywords reg, int value);
    void setStatusPrecondition(Enu::ECPUKeywords bit, bool value);

    void clearCpu();
    void clearCpuControlSignals();

    // These are used by the main window in order to allow it to use the
    //  <enter> key to step.
    void clock();

    QSize sizeHint() const override;

protected:
    void changeEvent(QEvent *e) override;
    QSharedPointer<InterfaceMCCPU> cpu;
    QSharedPointer<NewCPUDataSection> dataSection;
    QGraphicsScene *scene;
    CpuGraphicsItems *cpuPaneItems;
    Enu::CPUType type;

private:
    Ui::CpuPane *ui;
    void initRegisters();

protected slots:
    void regTextEdited(QString str);
    void regTextFinishedEditing();
    void zoomFactorChanged(int factor);
    void labelClicked();
    void clockButtonPushed();
    void on_copyToMicrocodePushButton_clicked();
    void ALUTextEdited(QString str);

public slots:
    void onClockChanged();
    void onBusChanged();
    void onRegisterChanged(quint8 which,quint8 oldVal,quint8 newVal);
    void onMemoryRegisterChanged(Enu::EMemoryRegisters,quint8 oldVal,quint8 newVal);
    void onStatusBitChanged(Enu::EStatusBit,bool value);
    void repaintOnScroll(int distance);
    void onSimulationUpdate();
    void onDarkModeChanged(bool darkMode, QString styleSheet);
    // Instead of passing the type it changed to
    // the CPU will querry the cpu it was given for its new type.
    void onCPUTypeChanged();

signals:
    void updateSimulation();
    void stopSimulation();
    void simulationFinished();
    void appendMicrocodeLine(QString line);
    void readByte(int address);
    void writeByte(int address);
    void registerChanged(quint8 reg,quint8 value);
    void statusBitChanged(Enu::EStatusBit,bool value);
};

#endif // CPUPANE_H
