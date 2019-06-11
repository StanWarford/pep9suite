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
