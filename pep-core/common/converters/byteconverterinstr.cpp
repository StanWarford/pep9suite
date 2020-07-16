// File: byteconverterinstr.cpp
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.

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

#include "byteconverterinstr.h"
#include "ui_byteconverterinstr.h"

#include "pep/apepversion.h"

ByteConverterInstr::ByteConverterInstr(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ByteConverterInstr), pep_version(nullptr)
{
    ui->setupUi(this);
    // The label is really a text editor.
    // However, nested QLabels aren't picking up on style changes in parent classes,
    // so they do not render text correctly in Mac OS dark mode.
    // By making a read-only transparent QLineEdit, we should get the correct
    // text styling with minimal extra effort on our part.
    QPalette pal = ui->label->palette();
    pal.setColor(QPalette::Window, QColor(0,0,0,0));
    pal.setColor(QPalette::Window, QColor(0,0,0,0));
    pal.setColor(QPalette::Base, QColor(0,0,0,0));
    pal.setColor(QPalette::Window, QColor(0,0,0,0));
    ui->label->setPalette(pal);
    ui->label->setFrame(false);
    ui->label->setAttribute(Qt::WA_TranslucentBackground);
    ui->label->setReadOnly(true);
}

void ByteConverterInstr::init(QSharedPointer<const APepVersion> pep_version)
{
    this->pep_version = pep_version;
}

ByteConverterInstr::~ByteConverterInstr()
{
    delete ui;
}

void ByteConverterInstr::setValue(int data)
{
    auto str = " " + pep_version->getAsmMnemonic(data) % (pep_version->isInstructionUnary(data)? QString() : (", " % pep_version->getAsmAddr(data)));
    ui->label->setText(str);
}

void ByteConverterInstr::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

