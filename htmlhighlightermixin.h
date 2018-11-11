#ifndef HTMLHIGHLIGHTERMIXIN_H
#define HTMLHIGHLIGHTERMIXIN_H
#include <QSyntaxHighlighter>
#include <QObject>
#include "pep.h"
#include "colors.h"

/*class RestyleableItem: public QObject {
    Q_OBJECT
protected:
    // Non-owning pointer to a style
    const PepColors::Colors* style;

public:
    RestyleableItem(const PepColors::Colors& style, QObject *parent = 0);
    virtual ~RestyleableItem();
    const PepColors::Colors* getStyle() const;
    void setStyle(const PepColors::Colors& style);

signals:
    void styleChanged();
};*/

class HTMLHighlighterMixin : public QSyntaxHighlighter
{
public:
    explicit HTMLHighlighterMixin(QTextDocument *parent = 0);
    virtual void asHtml(QString& html, QFont font = Pep::codeFont) const;
};

#endif // HTMLHIGHLIGHTERMIXIN_H
