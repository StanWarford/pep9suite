// File: memorycellgraphicsitem.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2009  J. Stanley Warford, Pepperdine University

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

#include "memorycellgraphicsitem.h"
#include "pep.h"
#include <QPainter>
#include "amemorydevice.h"
// #include <QDebug>

const int MemoryCellGraphicsItem::boxHeight = 22;
const int MemoryCellGraphicsItem::boxWidth = 50;
const int MemoryCellGraphicsItem::addressWidth = 48;
const int MemoryCellGraphicsItem::symbolWidth = 96;
const int MemoryCellGraphicsItem::bufferWidth = 14;

MemoryCellGraphicsItem::MemoryCellGraphicsItem(const AMemoryDevice *memDevice, int addr, QString sym,
                                               Enu::ESymbolFormat eSymFrmt, int xLoc, int yLoc): memDevice(memDevice), x(xLoc), y(yLoc),
    address(addr), eSymbolFormat(eSymFrmt), boxColor(Qt::black), boxBgColor(Qt::white), textColor(Qt::black), boxTextColor(Qt::black)
{
    if (sym.length() > 0 && sym.at(0).isDigit()) {
        sym = "";
    }
    symbol = sym;
}

QRectF MemoryCellGraphicsItem::boundingRect() const
{
    const int Margin = 4;
    return QRectF(QPointF(x - addressWidth - Margin, y - Margin),
                  QSizeF(addressWidth + bufferWidth * 2 + boxWidth + symbolWidth + Margin * 2, boxHeight + Margin * 2));
}

void MemoryCellGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QPen pen(boxColor);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setBrush(boxBgColor);
    painter->setRenderHint(QPainter::Antialiasing);
    painter->drawRoundedRect(QRectF(x, y, boxWidth, boxHeight), 2, 2, Qt::RelativeSize);

    painter->setPen(textColor);
    painter->setRenderHint(QPainter::TextAntialiasing);
    painter->setFont(QFont(Pep::codeFont, Pep::codeFontSize));

    painter->drawText(QRectF(x - addressWidth - bufferWidth, y, addressWidth, boxHeight), Qt::AlignVCenter | Qt::AlignRight, QString("%1").arg(address, 4, 16, QLatin1Char('0')).toUpper());

    painter->drawText(QRectF(x + bufferWidth + boxWidth, y, symbolWidth, boxHeight), Qt::AlignVCenter | Qt::AlignLeft, QString("%1").arg(symbol));

    painter->setPen(boxTextColor);
    painter->drawText(QRectF(x, y, boxWidth, boxHeight), Qt::AlignCenter, value);
}

void MemoryCellGraphicsItem::updateValue()
{
    quint8 byte;
    quint16 word;
    switch (eSymbolFormat) {
    case Enu::ESymbolFormat::F_1C:
        memDevice->getByte(byte, address);
        value = QString(QChar(byte));
        break;
    case Enu::ESymbolFormat::F_1D:
        memDevice->getByte(byte, address);
        value = QString("%1").arg(QChar(byte));
        break;
    case Enu::ESymbolFormat::F_2D:
        memDevice->getWord(word, address);
        value = QString("%1").arg(word);
        break;
    case Enu::ESymbolFormat::F_1H:
        memDevice->getByte(byte, address);
        value = QString("%1").arg(byte, 2, 16, QLatin1Char('0')).toUpper();
        break;
    case Enu::ESymbolFormat::F_2H:
        memDevice->getWord(word, address);
        value = QString("%1").arg(word, 4, 16, QLatin1Char('0')).toUpper();
        break;
    default:
        value = ""; // Should not occur
        break;
    }
}

int MemoryCellGraphicsItem::getAddress()
{
    return address;
}

int MemoryCellGraphicsItem::getNumBytes()
{
    return cellSize(eSymbolFormat);
}

int cellSize(Enu::ESymbolFormat symbolFormat)
{
    switch (symbolFormat) {
    case Enu::ESymbolFormat::F_1C:
        return 1;
    case Enu::ESymbolFormat::F_1D:
        return 1;
    case Enu::ESymbolFormat::F_2D:
        return 2;
    case Enu::ESymbolFormat::F_1H:
        return 1;
    case Enu::ESymbolFormat::F_2H:
        return 2;
    default:
        // Should not occur
        return 0;
    }
}
