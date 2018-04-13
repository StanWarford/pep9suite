// File: byteconverterinstr.h
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
#ifndef BYTECONVERTERINSTR_H
#define BYTECONVERTERINSTR_H

#include <QWidget>

namespace Ui {
    class ByteConverterInstr;
}

class ByteConverterInstr : public QWidget {
    Q_OBJECT
public:
    ByteConverterInstr(QWidget *parent = 0);
    ~ByteConverterInstr();

//    void setValue(int data);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::ByteConverterInstr *ui;
};

#endif // BYTECONVERTERINSTR_H
