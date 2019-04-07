// File: disableselectionmodel.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  J. Stanley Warford, Pepperdine University

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
#ifndef DISABLESELECTIONMODEL_H
#define DISABLESELECTIONMODEL_H

#include <QObject>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
/*
 * DisableSelectionModel prevents users from changing the selected row in an item model.
 * To prevent a user from changing the selection, call onDisableSelection().
 * To allow a user to change the selection, call onEnableSelection().
 * To programatically change the selected row, call forceSelectRow(row).
 */
class DisableSelectionModel : public QItemSelectionModel
{
    bool disableSelection;
    Q_OBJECT
public:
    DisableSelectionModel(QAbstractItemModel *model = Q_NULLPTR) noexcept;
    DisableSelectionModel(QAbstractItemModel *model, QObject *parent = Q_NULLPTR) noexcept;
    virtual ~DisableSelectionModel() override;
    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override;
    void select(const QModelIndex &index, SelectionFlags command) override;
    void forceSelectRow(uint row);
public slots:
    void onDisableSelection();
    void onEnableSelection();
};

#endif // DISABLESELECTIONMODEL_H
