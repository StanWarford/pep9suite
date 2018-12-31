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


