#include "disableselectionmodel.h"
#include <qdebug.h>
#include <QObject>
#include <QItemSelectionModel>

DisableSelectionModel::DisableSelectionModel(QAbstractItemModel *model):QItemSelectionModel(model),disableSelection(false)
{

}

DisableSelectionModel::DisableSelectionModel(QAbstractItemModel *model, QObject *parent):QItemSelectionModel(model,parent),disableSelection(false)
{

}

DisableSelectionModel::~DisableSelectionModel()
{

}

void DisableSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
    if(disableSelection)return;
    QItemSelectionModel::select(selection,command);
}

void DisableSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    if(disableSelection)return;
    QItemSelectionModel::select(index,command);
}

void DisableSelectionModel::forceSelectRow(int row)
{
    auto start = this->model()->index(row,0);
    auto end = model()->index(row,model()->columnCount()-1);
    clearSelection();
    QItemSelectionModel::select(QItemSelection(start,end),SelectionFlag::SelectCurrent);
}

void DisableSelectionModel::onBeginSimulation()
{
    disableSelection=true;
}

void DisableSelectionModel::onEndSimulation()
{
    disableSelection=false;
}
