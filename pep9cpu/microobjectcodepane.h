// File: objectcodepane.h
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
#ifndef OBJECTCODEPANE_H
#define OBJECTCODEPANE_H

#include <QWidget>
#include <QStandardItemModel>
#include "disableselectionmodel.h"
class InterfaceMCCPU;
class MicrocodeProgram;
class SymbolTable;
class RotatedHeaderView;
namespace Ui {
    class MicroObjectCodePane;
}

class MicroObjectCodePane : public QWidget {
    Q_OBJECT
public:
    explicit MicroObjectCodePane(QWidget *parent = nullptr);
    virtual ~MicroObjectCodePane() override;

    // Must be called after the class is constructed but before it is used, else the program will crash
    void init(QSharedPointer<InterfaceMCCPU> cpu, bool showCtrlSectionSignals);
    void setShowCtrlSectionSignals(bool showCtrlSectionSignals);

    void initCPUModelState();

    void highlightOnFocus();

    void setObjectCode();
    void setObjectCode(QSharedPointer<MicrocodeProgram> prog, QSharedPointer<SymbolTable> symbolTable);

    void highlightCurrentInstruction();
    void clearSimulationView();

    void copy();
    void assignHeaders();
signals:
    void beginSimulation();
    void endSimulation();
public slots:
    void onSimulationStarted();
    void onSimulationFinished();
    void onDarkModeChanged(bool);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::MicroObjectCodePane *ui;
    QSharedPointer<InterfaceMCCPU> cpu;
    QSharedPointer<MicrocodeProgram> program;
    QSharedPointer<SymbolTable> symTable;
    quint32 rowCount;
    RotatedHeaderView* rotatedHeaderView;
    DisableSelectionModel* selectionModel;
    QStandardItemModel* model;
    bool inSimulation, showCtrlSectionSignals;

};

#endif // OBJECTCODEPANE_H
