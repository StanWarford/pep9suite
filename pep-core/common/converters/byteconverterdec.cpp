// File: byteconverterdec.cpp
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

#include "byteconverterdec.h"
#include "ui_byteconverterdec.h"

ByteConverterDec::ByteConverterDec(QWidget *parent) :
        QWidget(parent),
        m_ui(new Ui::ByteConverterDec)
{
    m_ui->setupUi(this);
    // Regular expression to validate 0-255
    QRegExp decRx("^(25[0-5])|(2[0-4][0-9])|([0-1][0-9][0-9])|([0-9]{0,2})$");
    decValidator = new QRegExpValidator(decRx, this);
    m_ui->lineEdit->setValidator(decValidator);
    // Forward the textEdited() signal from m_ui->lineEdit up to the main window
    QObject::connect(m_ui->lineEdit, &QLineEdit::textEdited, this,
                     &ByteConverterDec::textEdited);
}

ByteConverterDec::~ByteConverterDec()
{
    delete m_ui;
}

void ByteConverterDec::setValue(int value)
{
    m_ui->lineEdit->setText(QString("%1").arg(value, 0, 10));
}
