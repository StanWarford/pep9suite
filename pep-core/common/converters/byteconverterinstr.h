// File: byteconverterinstr.h
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
#ifndef BYTECONVERTERINSTR_H
#define BYTECONVERTERINSTR_H

#include <QWidget>

namespace Ui {
    class ByteConverterInstr;
}

/*
 * The byte converter class set are used to show how a byte value is represented
 * in different forms (e.g. bin, hec, character, etc.).
 *
 * This particular byte converter renders the Pep/9 instruction specifier
 * associated with a particualr byte value.
 */
class ByteConverterInstr : public QWidget {
    Q_OBJECT
    Q_DISABLE_COPY(ByteConverterInstr)

public:
    ByteConverterInstr(QWidget *parent = nullptr);
    ~ByteConverterInstr() override;

    void setValue(int data);

protected:
    void changeEvent(QEvent *e) override;

private:
    Ui::ByteConverterInstr *ui;
};

#endif // BYTECONVERTERINSTR_H
