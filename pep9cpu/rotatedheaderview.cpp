// File: rotatedheaderview.cpp
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
#include "rotatedheaderview.h"
#include <QPainter>
#include <math.h>
#include "pep.h"
RotatedHeaderView::RotatedHeaderView(Qt::Orientation orientation, QWidget *parent): QHeaderView(orientation, parent)
{

}

void RotatedHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    // Rotate 90 degrees counter clockwise
    qreal angle = -90;
    qint32 newx = -rect.bottom();
    qint32 newy = rect.x();
    QRect nRect = QRect(newx, newy, rect.height(), rect.width());
    painter->rotate(angle);
    painter->setFont(QFont(Pep::codeFont,Pep::codeFontSize));
    // Ask parent for brush to color text.
    painter->setBrush(QBrush(parentWidget()->palette().windowText()));
    painter->drawText(nRect, this->model()->headerData(logicalIndex, Qt::Horizontal).toString());
    painter->restore();
}

QSize RotatedHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QSize old = QHeaderView::sectionSizeFromContents(logicalIndex);
    // The original header view makes the text a little too wide,
    // so shave off a few pixels from the width to save vertical space.
    return QSize(old.height(), old.width() - 3);
}


