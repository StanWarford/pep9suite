// File: disableselectionmodel.cpp
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  J. Stanley Warford & Matthew McRaven, Pepperdine University

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
#include "disableselectionmodel.h"

#include <QObject>
#include <QItemSelectionModel>

DisableSelectionModel::DisableSelectionModel(QAbstractItemModel *model) noexcept: QItemSelectionModel(model), disableSelection(false)
{

}

DisableSelectionModel::DisableSelectionModel(QAbstractItemModel *model, QObject *parent) noexcept: QItemSelectionModel(model, parent),
    disableSelection(false)
{

}

DisableSelectionModel::~DisableSelectionModel()
{

}

void DisableSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if(disableSelection) return;
    QItemSelectionModel::select(selection, command);
}

void DisableSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    if(disableSelection)return;
    QItemSelectionModel::select(index,command);
}

void DisableSelectionModel::forceSelectRow(qint32 row)
{
    // If the row is is sure to be out of range, don't continue with selection logic.
    if(row < 0) {
        return;
    }
    // Select all columns in a row.
    auto start = this->model()->index(row, 0);
    auto end = model()->index(row, model()->columnCount() - 1);
    QItemSelectionModel::select(QItemSelection(start, end), SelectionFlag::ClearAndSelect);
}

void DisableSelectionModel::forceClear()
{
    // Make sure there is no selected item set.
    QItemSelectionModel::clearSelection();
    QItemSelectionModel::clearCurrentIndex();
}

void DisableSelectionModel::onDisableSelection()
{
    disableSelection = true;
}

void DisableSelectionModel::onEnableSelection()
{
    disableSelection = false;
}
