#include "assemblerpane.h"
#include "ui_assemblerpane.h"

#include "assembler/asmprogram.h"

AssemblerPane::AssemblerPane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AssemblerPane), manager(nullptr), output(nullptr)
{
    ui->setupUi(this);
    //Connect assembler pane widgets
    connect(ui->sourcePane, &AsmSourceCodePane::labelDoubleClicked, this, &AssemblerPane::doubleClickedCodeLabel);
    connect(ui->objectPane, &AsmObjectCodePane::labelDoubleClicked, this, &AssemblerPane::doubleClickedCodeLabel);
    connect(ui->listingPane, &AsmProgramListingPane::labelDoubleClicked, this, &AssemblerPane::doubleClickedCodeLabel);

    connect(ui->sourcePane, &AsmSourceCodePane::undoAvailable, this, &AssemblerPane::onChildUndoAvailable);
    connect(ui->objectPane, &AsmObjectCodePane::undoAvailable, this, &AssemblerPane::onChildUndoAvailable);
    connect(ui->sourcePane, &AsmSourceCodePane::redoAvailable, this, &AssemblerPane::onChildRedoAvailable);
    connect(ui->objectPane, &AsmObjectCodePane::redoAvailable, this, &AssemblerPane::onChildRedoAvailable);
}

AssemblerPane::~AssemblerPane()
{
    delete ui;
}

void AssemblerPane::init(AsmProgramManager *manager)
{
    this->manager = manager;
}

void AssemblerPane::newProject()
{
    ui->sourcePane->setFocus();
    ui->sourcePane->clearSourceCode();
    ui->sourcePane->setCurrentFile("");
    ui->objectPane->clearObjectCode();
    ui->objectPane->setCurrentFile("");
    ui->listingPane->clearAssemblerListing();
    ui->listingPane->setCurrentFile("");
    // Set output to nullptr, as a new project does
    // not have any output until assembled.
    output.clear();
}

void AssemblerPane::loadSourceFile(QString fileName, QString code)
{
    newProject();
    ui->sourcePane->setSourceCodePaneText(code);
    ui->sourcePane->setCurrentFile(fileName);
    ui->sourcePane->setFocus();
}

void AssemblerPane::loadObjectFile(QString fileName, QString code)
{
    newProject();
    ui->objectPane->setObjectCodePaneText(code);
    ui->objectPane->setCurrentFile(fileName);
    ui->objectPane->setFocus();
}

void AssemblerPane::formatAssemblerCode()
{
    auto tempOutput = manager->assembleProgram(getPaneContents(Enu::EPane::ESource));
    if(!tempOutput->success) {
        ui->sourcePane->appendMessagesInSourceCodePane(tempOutput->errors);
        return;
    }
    else {
      output = tempOutput;
    }
    QString code = output->prog->getFormattedSourceCode();
    ui->sourcePane->setSourceCodePaneText(code);
}

void AssemblerPane::removeErrorMessages()
{
    ui->sourcePane->removeErrorMessages();
}

QFileInfo AssemblerPane::getFileName(Enu::EPane which) const
{
    QFileInfo fInfo;
    switch(which) {
    case Enu::EPane::ESource:
        fInfo = QFileInfo(ui->sourcePane->getCurrentFile());
        break;
    case Enu::EPane::EObject:
        fInfo = QFileInfo(ui->objectPane->getCurrentFile());
        break;
    case Enu::EPane::EListing:
        fInfo = QFileInfo(ui->listingPane->getCurrentFile());
        break;
    default:
        // Can't access file name of other panes from this class.
        break;
    }
    return fInfo;
}

void AssemblerPane::setFileName(Enu::EPane which, QFileInfo fileName)
{
    switch(which) {
    case Enu::EPane::ESource:
        ui->sourcePane->setCurrentFile(fileName.absoluteFilePath());
        break;
    case Enu::EPane::EObject:
        ui->objectPane->setCurrentFile(fileName.absoluteFilePath());
        break;
    case Enu::EPane::EListing:
        ui->listingPane->setCurrentFile(fileName.absoluteFilePath());
        break;
    default:
        // Can't set file of other panes from this class.
        break;
    }
}

void AssemblerPane::setFilesFromSource()
{
    // If source code pane has a file, set the object code and listing to have the same file name.
    // Otherwise, set the filenames to empty.
    QFileInfo fileInfo(ui->sourcePane->getCurrentFile());
    QString object, listing;
    // If there is no file name, then empty file name of listing and object code panes.
    if(fileInfo.fileName().isEmpty()){
        object = "";
        listing = "";
    }
    else {
        object = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName() + ".pepo");
        listing = fileInfo.absoluteDir().absoluteFilePath(fileInfo.baseName() + ".pepl");
    }
    ui->objectPane->setCurrentFile(object);
    ui->listingPane->setCurrentFile(listing);
}

QString AssemblerPane::getPaneContents(Enu::EPane which) const
{
    switch(which) {
    case Enu::EPane::ESource:
        return ui->sourcePane->toPlainText();
    case Enu::EPane::EObject:
        return ui->objectPane->toPlainText();
    case Enu::EPane::EListing:
        return ui->listingPane->toPlainText();
    default:
        // Other panes may not be read from this class.
        return QString();
    }
}

void AssemblerPane::setPanesFromProgram(const AsmProgramManager::AsmOutput &assemblerOutput)
{
    ui->sourcePane->displayAssemblerOutput(assemblerOutput);
    ui->objectPane->setObjectCode(assemblerOutput.prog->getObjectCode());
    ui->listingPane->setAssemblerListing(assemblerOutput.prog,
                                         assemblerOutput.prog->getSymbolTable());
}

void AssemblerPane::clearPane(Enu::EPane which)
{
    switch(which) {
    case Enu::EPane::ESource:
        ui->sourcePane->clearSourceCode();
        break;
    case Enu::EPane::EObject:
        ui->objectPane->clearObjectCode();
        break;
    case Enu::EPane::EListing:
        ui->listingPane->clearAssemblerListing();
        break;
    default:
        // Can't clear other panes from this class.
        break;
    }

}

QSharedPointer<AsmProgramManager::AsmOutput> AssemblerPane::getAssemblerOutput()
{
    return output;
}

bool AssemblerPane::isModified(Enu::EPane which) const
{
    switch(which) {
    case Enu::EPane::ESource:
        return ui->sourcePane->isModified();
    case Enu::EPane::EObject:
        return ui->objectPane->isModified();
    case Enu::EPane::EListing:
        return ui->listingPane->isModified();
    default:
        // Can't check if any other panes are modified from this class.
       return false;
    }
}

void AssemblerPane::setModified(Enu::EPane which, bool val)
{
    switch(which) {
    case Enu::EPane::ESource:
        ui->sourcePane->setModified(val);
        break;
    case Enu::EPane::EObject:
        ui->objectPane->setModified(val);
        break;
    case Enu::EPane::EListing:
        // The assembler listing can't be modified.
        break;
    default:
        // Can't modify any other panes from this class.
        break;
    }
}

void AssemblerPane::assembleAsOS(bool forceBurnAt0xFFFF)
{
    // Clean up any global state from previous compilation attempts
    removeErrorMessages();
    output = manager->assembleOS(ui->sourcePane->toPlainText(), forceBurnAt0xFFFF);
    if(output->success) {
        setPanesFromProgram(*output);
    }
    else {
        ui->sourcePane->displayAssemblerOutput(*output);
        clearPane(Enu::EPane::EObject);
        clearPane(Enu::EPane::EListing);
    }
}

void AssemblerPane::assembleAsProgram()
{
    // Clean up any global state from previous compilation attempts
    removeErrorMessages();
    output = manager->assembleProgram(ui->sourcePane->toPlainText());
    if(output->success) {
        setPanesFromProgram(*output);
    }
    else {
        ui->sourcePane->displayAssemblerOutput(*output);
        clearPane(Enu::EPane::EObject);
        clearPane(Enu::EPane::EListing);
    }
}

void AssemblerPane::rebuildHighlightingRules()
{
    ui->sourcePane->rebuildHighlightingRules();
    ui->listingPane->rebuildHighlightingRules();
}

void AssemblerPane::highlightOnFocus()
{
    ui->sourcePane->highlightOnFocus();
    ui->objectPane->highlightOnFocus();
    ui->listingPane->highlightOnFocus();
}

bool AssemblerPane::isUndoable()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        return ui->sourcePane->isUndoable();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        return ui->objectPane->isUndoable();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return false;
    }
    else {
        return false;
    }
}

bool AssemblerPane::isRedoable()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        return ui->sourcePane->isRedoable();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        return ui->objectPane->isRedoable();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return false;
    }
    else {
        return false;
    }
}

void AssemblerPane::undo()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        ui->sourcePane->undo();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        ui->objectPane->undo();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return;
    }
}

void AssemblerPane::redo()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        ui->sourcePane->redo();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        ui->objectPane->redo();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return;
    }
}

void AssemblerPane::cut()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        ui->sourcePane->cut();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        ui->objectPane->cut();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return;
    }
}

void AssemblerPane::copy()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        ui->sourcePane->copy();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        ui->objectPane->copy();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        ui->listingPane->copy();
    }
}

void AssemblerPane::paste()
{
    if(ui->sourcePane->isAncestorOf(focusWidget())) {
        ui->sourcePane->paste();
    }
    else if(ui->objectPane->isAncestorOf(focusWidget())) {
        ui->objectPane->paste();
    }
    else if(ui->listingPane->isAncestorOf(focusWidget())) {
        return;
    }
}

int AssemblerPane::enabledButtons() const
{
    int which = 0;
    if (ui->sourcePane->hasFocus()) {
        which = Enu::EditButton::COPY | Enu::EditButton::CUT | Enu::EditButton::PASTE;
        which |= Enu::EditButton::UNDO * ui->sourcePane->isUndoable() | Enu::EditButton::REDO * ui->sourcePane->isRedoable();
    }
    else if (ui->objectPane->hasFocus()) {
        which = Enu::EditButton::COPY | Enu::EditButton::CUT | Enu::EditButton::PASTE;
        which |= Enu::EditButton::UNDO * ui->objectPane->isUndoable() | Enu::EditButton::REDO * ui->objectPane->isRedoable();
    }
    else if (ui->listingPane->hasFocus()) {
        which = Enu::EditButton::COPY;
    }
    return which;
}

void AssemblerPane::writeSettings(QSettings &settings)
{
    settings.beginGroup("AssemblerPane");
    settings.beginWriteArray("codePaneSplit", 3);
    QList<int> temp = ui->splitter->sizes();
    for(int it = 0; it < 3; it++) {
        settings.setArrayIndex(it);
        settings.setValue("size", temp[it]);
    }
    settings.endArray();
    ui->sourcePane->writeSettings(settings);
    // ui->objectPane->writeSettings(settings); // Unimplemented
    // ui->listingPane->writeSettings(settings); // Unimplemented
    settings.endGroup();
}

void AssemblerPane::readSettings(QSettings &settings)
{
    // Restore last used split in assembly code pane
    settings.beginGroup("AssemblerPane");
    QVariant val = settings.beginReadArray("codePaneSplit");
    QList<int> sizes;
    for(int it = 0; it < ui->splitter->sizes().length(); it++) {
        settings.setArrayIndex(it);
        sizes.append(settings.value("size", 1).toInt());
    }
    ui->splitter->setSizes(sizes);
    settings.endArray();
    ui->sourcePane->readSettings(settings);
    // ui->objectPane->writeSettings(settings); // Unimplemented
    // ui->listingPane->writeSettings(settings); // Unimplemented
    settings.endGroup();
}

void AssemblerPane::onFontChanged(QFont font)
{
    ui->sourcePane->onFontChanged(font);
    ui->objectPane->onFontChanged(font);
    ui->listingPane->onFontChanged(font);
}

void AssemblerPane::onDarkModeChanged(bool darkMode)
{
    ui->sourcePane->onDarkModeChanged(darkMode);
    ui->listingPane->onDarkModeChanged(darkMode);
}

void AssemblerPane::onRemoveAllBreakpoints()
{
    ui->sourcePane->onRemoveAllBreakpoints();
}

void AssemblerPane::onBreakpointAdded(quint16 address)
{
    ui->sourcePane->onBreakpointAdded(address);
}

void AssemblerPane::onBreakpointRemoved(quint16 address)
{
    ui->sourcePane->onBreakpointRemoved(address);
}

void AssemblerPane::doubleClickedCodeLabel(Enu::EPane which)
{
    QList<int> list, defaultList = {1,1,1};
    QList<int> old = ui->splitter->sizes();
    auto max = std::minmax_element(old.begin(), old.end());
    static const int largeSize = 3000, smallSize = 1;
    bool sameSize = *max.second - *max.first <5;
    switch(which)
    {
    // Give the selected pane the majority of the screen space.
    case Enu::EPane::ESource:
        if(old[0] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(largeSize);
            list.append(smallSize);
            list.append(smallSize);
        }
        break;
    case Enu::EPane::EObject:
        if(old[1] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(smallSize);
            list.append(largeSize);
            list.append(smallSize);
        }
        break;
    case Enu::EPane::EListing:
        if(old[2] == *max.second && !sameSize) {
            list = defaultList;
        }
        else {
            list.append(smallSize);
            list.append(smallSize);
            list.append(largeSize);
        }
        break;
    default:
        // Provided a default - even though it should never occur -
        // to silence compiler warnings.
        break;
    }
    ui->splitter->setSizes(list);
}

void AssemblerPane::onChildUndoAvailable(bool b)
{
    emit undoAvailable(b);
}

void AssemblerPane::onChildRedoAvailable(bool b)
{
    emit redoAvailable(b);
}
