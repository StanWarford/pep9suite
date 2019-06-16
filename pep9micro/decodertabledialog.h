// File: decodertabledialog.h
/*
    Pep9Micro is a complete CPU simulator for the Pep/9 instruction set,
    and is capable of assembling programs to object code, executing
    object code programs, and executing microcode fragments.

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
#ifndef DECODERTABLEDIALOG_H
#define DECODERTABLEDIALOG_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
namespace Ui {
class DecoderTableDialog;
}
class DecoderTableDelegate;

class DecoderTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DecoderTableDialog(QWidget *parent = nullptr);
    ~DecoderTableDialog();
public slots:
    void on_button_close_pressed();
    void on_button_reset_pressed();
    void on_lineEdit_Start_editingFinished();

private:
    Ui::DecoderTableDialog *ui;
    DecoderTableDelegate* delegate;
    QStandardItemModel *model;
    void refreshTable();
};

/*
 * Item delegate that handles input validation of hex constants, and disables editing of address and hex dump columns.
 * Eventually, it can be extended to be signaled to enable or disable editing
 */
class DecoderTableDelegate: public QStyledItemDelegate {
public:
    DecoderTableDelegate(QObject* parent = nullptr);
    virtual ~DecoderTableDelegate() override;
    // See http://doc.qt.io/qt-5/qstyleditemdelegate.html#subclassing-qstyleditemdelegate for explanation on the methods being reimplemented.

    // If the index is editable, create an editor that validates byte hex constants, otherwise return nullptr
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    // Provides editor widget with starting data for editing.
    virtual void setEditorData(QWidget * editor, const QModelIndex & index) const override;
    // Ensure that editor is displayed correctly on the item view
    virtual void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const override;
    // Handle updating data in the model via calling the memorySection
    virtual void setModelData(QWidget *editor,
                                    QAbstractItemModel *model,
                                    const QModelIndex &index) const override;
};
#endif // DECODERTABLEDIALOG_H
