#include "htmlhighlightermixin.h"
#include <QTextBlock>
#include <QTextDocument>
#include <QTextDocumentFragment>

/*RestyleableItem::RestyleableItem(const PepColors::Colors& style, QObject *parent) : QObject(parent),
    style(&style)
{

}

RestyleableItem::~RestyleableItem()
{

}

const PepColors::Colors *RestyleableItem::getStyle() const
{
    return style;
}

void RestyleableItem::setStyle(const PepColors::Colors &style)
{
    if( *(this->style) != style) {
        this->style = &style;
        emit styleChanged();
    }
}*/

HTMLHighlighterMixin::HTMLHighlighterMixin(QTextDocument *parent): QSyntaxHighlighter(parent)
{

}

void HTMLHighlighterMixin::asHtml(QString& html, QFont font) const
{
    // Create a new document from all the selected text document.
    QTextCursor cursor(document());
    cursor.select(QTextCursor::Document);
    QTextDocument* tempDocument(new QTextDocument);
    Q_ASSERT(tempDocument);
    QTextCursor tempCursor(tempDocument);

    tempCursor.insertFragment(cursor.selection());
    tempCursor.select(QTextCursor::Document);
    tempCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
    // Set the default foreground for the inserted characters.
    QTextCharFormat textfmt = tempCursor.charFormat();
    textfmt.setForeground(Qt::black);
    textfmt.setFont(font);
    //tempCursor.setCharFormat(textfmt);
    tempCursor.setBlockCharFormat(textfmt);

    // Apply the additional formats set by the syntax highlighter
    QTextBlock start = document()->findBlock(cursor.selectionStart());
    QTextBlock end = document()->findBlock(cursor.selectionEnd());
    end = end.next();
    const int selectionStart = cursor.selectionStart();
    const int endOfDocument = tempDocument->characterCount() - 1;
    for(QTextBlock current = start; current.isValid() && current != end; current = current.next()) {
        const QTextLayout* layout(current.layout());

        foreach(const QTextLayout::FormatRange &range, layout->additionalFormats()) {
            const int start = current.position() + range.start - selectionStart;
            const int end = start + range.length;
            if(end <= 0 || start >= endOfDocument)
                continue;
            tempCursor.setPosition(qMax(start, 0));
            tempCursor.setPosition(qMin(end, endOfDocument), QTextCursor::KeepAnchor);
            textfmt = range.format;
            textfmt.setFont(font);
            tempCursor.setCharFormat(textfmt);
        }
    }

    // Reset the user states since they are not interesting
    for(QTextBlock block = tempDocument->begin(); block.isValid(); block = block.next())
        block.setUserState(-1);

    // Make sure the text appears pre-formatted, and set the background we want.
    tempCursor.select(QTextCursor::Document);
    QTextBlockFormat blockFormat = tempCursor.blockFormat();
    blockFormat.setNonBreakableLines(true);
    blockFormat.setBackground(Qt::white);
    tempCursor.setBlockFormat(blockFormat);

    // Finally retreive the syntax higlighted and formatted html.
    html = tempCursor.selection().toHtml();
    delete tempDocument;
} // asHtml
