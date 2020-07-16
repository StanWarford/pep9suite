// File: decodertabledialog.cpp
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
#include "decodertabledialog.h"
#include "ui_decodertabledialog.h"

#include "isacpu.h"

DecoderTableDialog::DecoderTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DecoderTableDialog), model(new QStandardItemModel(this))
{
    ui->setupUi(this);

    // Force this window to stay on top, so that users may not accidentally change
    // mnemonics and keywords while a microprogram is running.
    setWindowFlags(Qt::WindowStaysOnTopHint);
    delegate = new DecoderTableDelegate(ui->tableView);

    // Set up model so it has the right number of rows & columns.
    model->insertColumns(0,3);
    model->insertRows(0,256);
    // Then fill in the hexidecimal value of the instruction specifiers
    for(int it = 0; it <= 255; it++) {
        model->setData(model->index(it,0), QString("%1").arg(it,2,16,QChar('0')).toUpper());
    }

    // Install model, delegates.
    ui->tableView->setModel(model);
    ui->tableView->setItemDelegate(delegate);
    ui->lineEdit_Start->setValidator(new QRegExpValidator(QRegExp("[A-zÀ-ÖØ-öø-ÿ][0-9A-zÀ-ÖØ-öø-ÿ]*"), ui->lineEdit_Start));
    refreshTable();

}

DecoderTableDialog::~DecoderTableDialog()
{
    delete ui;
}

void DecoderTableDialog::on_button_close_pressed()
{
    this->close();
}

void DecoderTableDialog::on_button_reset_pressed()
{
    Pep9::ISA::initMicroDecoderTables();
    refreshTable();
}

void DecoderTableDialog::on_lineEdit_Start_editingFinished()
{
    Pep9::ISA::defaultStartSymbol = ui->lineEdit_Start->text();
}

void DecoderTableDialog::refreshTable()
{
    ui->lineEdit_Start->setText(Pep9::ISA::defaultStartSymbol);
    for(int it = 0; it <= 255; it++) {
        model->setData(model->index(it, 1), Pep9::ISA::instSpecToMicrocodeInstrSymbol[it]);
        model->setData(model->index(it, 2), Pep9::ISA::instSpecToMicrocodeAddrSymbol[it]);
    }
}

DecoderTableDelegate::DecoderTableDelegate(QObject *parent): QStyledItemDelegate (parent)
{

}

DecoderTableDelegate::~DecoderTableDelegate()
{

}

QWidget *DecoderTableDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // The first and last columns are not user editable, so do not create an editor.
    if(index.column() == 0) return nullptr;
    // Otherwise, defer to QStyledItemDelegate's implementation, which returns a LineEdit
    QLineEdit *line = qobject_cast<QLineEdit*>(QStyledItemDelegate::createEditor(parent, option, index));
    // Apply a validator, so that a user cannot input anything other than a one byte hexadecimal constant
    line->setValidator(new QRegExpValidator(QRegExp("[A-zÀ-ÖØ-öø-ÿ][0-9A-zÀ-ÖØ-öø-ÿ]*"), line));
    return line;
}

void DecoderTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // The default value in the line edit should be the text currently in that cell.
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *line = static_cast<QLineEdit*>(editor);
    line->setText(value);
}

void DecoderTableDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex& /*index*/) const
{
    // Pass geometry information to the editor.
    editor->setGeometry(option.rect);
}

void DecoderTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    // Get text from editor and convert it to a integer.
    QLineEdit *line = static_cast<QLineEdit*>(editor);
    QString strValue = line->text();
    model->setData(index, strValue);
    if(index.column() == 1) {
         Pep9::ISA::instSpecToMicrocodeInstrSymbol[index.row()] = index.data().toString();
    }
    else if(index.column() == 2){
        Pep9::ISA::instSpecToMicrocodeAddrSymbol[index.row()] = index.data().toString();
    }
}
