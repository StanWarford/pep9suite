// File: rotatedheaderview.h
/*
    Pep9CPU is a CPU simulator for executing microcode sequences to
    implement instructions in the instruction set of the Pep/9 computer.

    Copyright (C) 2018  J. Stanley Warford, Pepperdine University

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
#ifndef ROTATEDHEADERVIEW_H
#define ROTATEDHEADERVIEW_H

#include <QHeaderView>
#include <QWidget>
#include <QObject>
/*
 * This class paints a header view with text rotated 90 degrees counter clockwise.
 */
class RotatedHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    RotatedHeaderView(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);
    virtual ~RotatedHeaderView() override {}
protected:
    void paintSection(QPainter* painter,
                  const QRect& rect,
                  int logicalIndex) const override;
   QSize sectionSizeFromContents(int logicalIndex) const override;
};

#endif // ROTATEDHEADERVIEW_H
