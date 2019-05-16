#ifndef ASSEMBLERPANE_H
#define ASSEMBLERPANE_H

#include "enu.h"
#include <QWidget>
#include <QSettings>
#include <asmprogrammanager.h>
namespace Ui {
class AssemblerPane;
}


class AssemblerPane : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AssemblerPane)
public:
    explicit AssemblerPane(QWidget *parent = nullptr);
    virtual ~AssemblerPane() override;
    void init(AsmProgramManager* manager);

    void newProject();
    void loadSourceFile(QString fileName, QString code);
    void loadObjectFile(QString fileName, QString code);
    void formatAssemblerCode();
    void removeErrorMessages();

    // Return the fully qualified file path associated with a pane.
    QFileInfo getFileName(Enu::EPane which) const;
    // Associate a pane with a file.
    void setFileName(Enu::EPane which, QFileInfo fileName);
    // Set the file names of the object & listing panes from the file name of the source code pane.
    void setFilesFromSource();

    QString getPaneContents(Enu::EPane which) const;
    void setPanesFromProgram(const AsmProgramManager::AsmOutput &assemblerOutput);
    void clearPane(Enu::EPane which);

    QSharedPointer<AsmProgramManager::AsmOutput> getAssemblerOutput();
    bool isModified(Enu::EPane which) const;
    void setModified(Enu::EPane which, bool val);

    void assembleAsOS(bool forceBurnAt0xFFFF = false);
    void assembleAsProgram();

    void rebuildHighlightingRules();
    void highlightOnFocus();
    // Post: Highlights the label based on the label window color saved in the UI file

    bool isUndoable();
    // Returns the undoability of the text edit

    bool isRedoable();
    // Returns the redoability of the text edit

    void undo();
    // Post: the last action in the text edit is undone

    void redo();
    // Post: the last undo in the text edit is redone

    void cut();
    // Post: selected text in the text edit is cut to the clipboard

    void copy();
    // Post: selected text in the text edit is copied to the clipboard

    void paste();
    // Post: selected text in the clipboard is pasted to the text edit

    int enabledButtons() const;
    void writeSettings(QSettings& settings);
    void readSettings(QSettings& settings);

signals:
    void undoAvailable(bool);
    void redoAvailable(bool);

public slots:
    void onFontChanged(QFont font);
    void onDarkModeChanged(bool darkMode);
    // Forwards event to AsmSourceTextEdit
    void onRemoveAllBreakpoints();
    // Forwards event to AsmSourceTextEdit
    void onBreakpointAdded(quint16 address);
    // Forwards event to AsmSourceTextEdit
    void onBreakpointRemoved(quint16 address);

private slots:
    void doubleClickedCodeLabel(Enu::EPane which);
    void onChildUndoAvailable(bool);
    void onChildRedoAvailable(bool);

private:
    Ui::AssemblerPane *ui;
    AsmProgramManager* manager;
    QSharedPointer<AsmProgramManager::AsmOutput> output;
};

#endif // ASSEMBLERPANE_H
