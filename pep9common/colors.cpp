#include "colors.h"
#include <QColor>
const PepColors::Colors PepColors::initDark()
{
    auto retVal =  PepColors::Colors();
    retVal.comment = QColor(Qt::green).lighter();
    retVal.leftOfExpression = QColor("lightsteelblue");
    retVal.rightOfExpression = QColor(Qt::red).lighter();
    retVal.symbolHighlight = QColor("firebrick");
    retVal.conditionalHighlight = QColor("orange");
    retVal.branchFunctionHighlight = QColor("lightblue");
    retVal.warningHighlight = QColor("lightsteelblue");
    retVal.errorHighlight = QColor("orangered");
    retVal.altTextHighlight = Qt::black;

    retVal.seqCircuitColor= QColor(0x3B3630).lighter(200); //Used to be 370 0x3B3630
    retVal.combCircuitRed = QColor(0xDF5A49);
    retVal.combCircuitGreen = QColor(0x008A1C);
    retVal.combCircuitBlue = QColor(0x506082);
    retVal.combCircuitYellow = QColor(0xE0B010);

    retVal.muxCircuitRed = retVal.combCircuitRed.darker(110); // A sightly lighter shade of combCircuitRed that is a better background for text
    retVal.muxCircuitBlue = retVal.combCircuitBlue.darker(110); // A sightly lighter shade of combCircuitBlue that is a better background for text
    retVal.muxCircuitYellow = retVal.combCircuitYellow.darker(110); // A sightly lighter shade of combCircuitYellow that is a better background for text
    retVal.muxCircuitGreen = retVal.combCircuitGreen.darker(110); // A sightly lighter shade of combCircuitGreen that is a better background for text

    retVal.aluColor = retVal.combCircuitBlue;
    retVal.aluOutline = retVal.combCircuitBlue.lighter(140);

    retVal.arrowColorOn = QColor(0xeeeeee); //Used to be Qt::white
    retVal.arrowColorOff = QColor(Qt::gray); //Used to be Qt:gray
    retVal.backgroundFill= QColor(0x31363b);
    retVal.arrowImageOn=(":/images/arrowhead_dark.png");
    retVal.arrowImageOff=(":/images/arrowhead_gray.png");

    retVal.memoryHighlightSP = QColor("Magenta");
    retVal.memoryHighlightPC = QColor("skyblue");
    retVal.memoryHighlightChanged = QColor("orangered");
    // Back & Text colors are those of QT Creator's text editor's line numbers & line number background in QT Creator Dark theme.
    retVal.lineAreaBackground = QColor(64, 66, 68);
    retVal.lineAreaText = QColor(190, 192, 194);
    retVal.lineAreaHighlight = QColor("dodgerblue");

    return retVal;
}

const PepColors::Colors PepColors::initLight()
{
    auto retVal =  PepColors::Colors();
    retVal.comment = Qt::darkGreen;
    retVal.leftOfExpression = Qt::darkBlue;
    retVal.rightOfExpression = Qt::darkMagenta;
    retVal.symbolHighlight =QColor("firebrick");
    retVal.conditionalHighlight = QColor("orange");
    retVal.branchFunctionHighlight = QColor("red");
    retVal.warningHighlight = Qt::darkBlue;
    retVal.errorHighlight = Qt::red;
    retVal.altTextHighlight = Qt::white;

    retVal.seqCircuitColor= QColor(0x3B3630).lighter(370);
    retVal.combCircuitRed = QColor(0xD92405).lighter(140);
    retVal.combCircuitBlue = QColor(0x3563EB).lighter(120);
    retVal.combCircuitYellow = QColor(0xEAC124).lighter(120);
    retVal.combCircuitGreen = QColor(0x739211).lighter(130);

    retVal.muxCircuitRed = retVal.combCircuitRed.lighter(140); // A sightly lighter shade of combCircuitRed that is a better background for text
    retVal.muxCircuitBlue = retVal.combCircuitBlue.lighter(140); // A sightly lighter shade of combCircuitBlue that is a better background for text
    retVal.muxCircuitYellow = retVal.combCircuitYellow.lighter(140); // A sightly lighter shade of combCircuitYellow that is a better background for text
    retVal.muxCircuitGreen = retVal.combCircuitGreen.lighter(140); // A sightly lighter shade of combCircuitGreen that is a better background for text

    retVal.aluColor = retVal.combCircuitBlue.lighter(140);
    retVal.aluOutline = retVal.combCircuitBlue;

    retVal.arrowColorOn = Qt::black;
    retVal.arrowColorOff = Qt::gray;
    retVal.backgroundFill= QColor(Qt::white);
    retVal.arrowImageOn=(":/images/arrowhead.png");
    retVal.arrowImageOff=(":/images/arrowhead_gray.png");

    retVal.memoryHighlightSP = Qt::darkMagenta;
    retVal.memoryHighlightPC = Qt::blue;
    retVal.memoryHighlightChanged = Qt::red;
    // Back & Text colors are those of QT Creator's text editor's line numbers & line number background in default theme.
    retVal.lineAreaBackground = QColor(240,240,240);
    retVal.lineAreaText = QColor(150,150,150);
    retVal.lineAreaHighlight = QColor("lightskyblue");

    return retVal;
}

QColor PepColors::invert(QColor input)
{
    return QColor(255-input.red(),255-input.green(),255-input.blue(),input.alpha());
}

