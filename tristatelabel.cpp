// File: tristatelabel.cpp
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

#include "tristatelabel.h"
#include <QMouseEvent>

TristateLabel::TristateLabel(QWidget *parent, ToggleMode mode) :
    QLabel(parent)
{
    toggleMode = mode;
}

void TristateLabel::setState(int state)
{
    if (state == -1) {
        setText("");
    }
    else if (state == 0) {
        setText("0");
    }
    else if (state == 1) {
        setText("1");
    }
}

bool TristateLabel::toggle()
{
    if (toggleMode == Tristate) {
        if (text() == "") {
            setText("0");
            return 0;
        }
        else if (text() == "0") {
            setText("1");
            return 1;
        }
        else if (text() == "1") {
            setText("");
            return 0; // what should this be?
        }
    }
    else if (toggleMode == ZeroOne) {
        if (text() == "0") {
            setText("1");
            return 1;
        }
        else if (text() == "1") {
            setText("0");
            return 0;
        }
    }
    else if (toggleMode == OneUndefined) {
        if (text() == "") {
            setText("1");
            return 1;
        }
        else if (text() == "1") {
            setText("");
            return 0;
        }
    }
    return 0;
}

void TristateLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton && isEnabled()) {
        emit clicked();
    }
}
