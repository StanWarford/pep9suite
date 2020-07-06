// File: asmcpupane.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.
    
    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#ifndef ASMCPUPANE_H
#define ASMCPUPANE_H

#include <QWidget>

#include "pep/enu.h"

namespace Ui {
    class AsmCpuPane;
}
class ACPUModel;
class InterfaceISACPU;
class AsmCpuPane : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(AsmCpuPane)
public:
    explicit AsmCpuPane(QWidget *parent = nullptr);
    virtual ~AsmCpuPane() override;
    // Must capture a pointer to ACPUModel to read register values.
    // Even though both pointers are probably to the same CPU, must also capture
    // InterfaceISACPU to have access to decoded operand value.
    void init(QSharedPointer<ACPUModel> acpu, QSharedPointer<InterfaceISACPU> isacpu);

    void updateCpu();
    // Post: Updates CPU pane labels

    void clearCpu();
    // Post: The CPU pane labels are blanked and the CPU registers are cleared

    void setDebugState(bool b);
    // Post: if b is true, checkboxes are set to disabled, and vice versa

    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool hasFocus();
    // Post: Returns if the single step button has focus

public slots:
    void onSimulationUpdate();

private:
    Ui::AsmCpuPane *ui;
    QSharedPointer<ACPUModel> acpu;
    QSharedPointer<InterfaceISACPU> isacpu;
    bool inSimulation;

};

#endif // ASMCPUPANE_H
