// File: executionstatisticswidget.cpp
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2019  J. Stanley Warford & Matthew McRaven, Pepperdine University

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

#include "executionstatisticswidget.h"
#include "ui_executionstatisticswidget.h"
#include "pep.h"

#include <QDebug>

ExecutionStatisticsWidget::ExecutionStatisticsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExecutionStatisticsWidget), model(new QStandardItemModel(this)), containsData(false)
{
    ui->setupUi(this);
    ui->treeView->setModel(model);
    model->setHorizontalHeaderLabels({"Instruction", "Frequency", "Instruction Hit %",
                                      "Data Read Hit %", "Data Write Hit %"});

    // Use the default palette of one of the line editors as a starting point,
    // and set its background color to be entirely transparent.
    QPalette pal = ui->lineEdit_Cycles->palette();
    QColor col = QColor(0,0,0,0);
    pal.setColor(QPalette::Base, col);

    ui->statsLabel->setFont(QFont(Pep::labelFont, Pep::labelFontSize));
    ui->lineEdit_Cycles->setPalette(pal);
    ui->lineEdit_Instructions->setPalette(pal);
}

void ExecutionStatisticsWidget::init(QSharedPointer<InterfaceISACPU> cpu, bool showCycles, bool showCacheStats)
{
    this->cpu = cpu;
    this->showCacheStats = showCacheStats;
    if(!showCycles) {
        ui->label->hide();
        ui->lineEdit_Cycles->hide();
    }
    // Refresh data adjusts visible headers.
    refreshData();
}

ExecutionStatisticsWidget::~ExecutionStatisticsWidget()
{
    delete ui;
}

void ExecutionStatisticsWidget::highlightOnFocus()
{
    if (ui->treeView->hasFocus()) {
        ui->statsLabel->setAutoFillBackground(true);
    }
    else {
        ui->statsLabel->setAutoFillBackground(false);
    }
}

bool ExecutionStatisticsWidget::hasFocus()
{
    return ui->treeView->hasFocus();
}


void ExecutionStatisticsWidget::onClear()
{
    ui->lineEdit_Cycles->clear();
    ui->lineEdit_Instructions->clear();
    model->removeRows(0, model->rowCount());
    // Sort by a non-existent column to prevent the "sorting arrow"
    // from appearing over unsorted data.
    ui->treeView->sortByColumn(-1, Qt::SortOrder::AscendingOrder);
    containsData = false;
}

void ExecutionStatisticsWidget::onSimulationStarted()
{
    // Make sure no statistics are displayed at the start of a run.
    onClear();
}

void ExecutionStatisticsWidget::onSimulationFinished()
{
    containsData = true;
    refreshData();
}

void ExecutionStatisticsWidget::onShowCacheStates(bool newValue)
{
    this->showCacheStats = newValue;
    refreshData();
}

void ExecutionStatisticsWidget::on_includeOSCheckBox_toggled(bool)
{
    // If the CPU currently has data to report, report statistics with new filters.
    if(containsData) {
        refreshData();
    }
}

// POD class to help aggregate statistics.
struct lookup {
    // How many times an instruction was referenced.
    quint32 tally;
    // At what operand specifier does the instruction start.
    quint8 start;
    // How many addressing modes does this instruction have?
    // Options are 0 (unary), 2(non-unary a field), and 8 (non-unary aaa field).
    quint8 addrModes;
    quint32 iread_hit{0}, iread_miss{0};
    quint32 dread_hit{0}, dread_miss{0};
    quint32 dwrite_hit{0}, dwrite_miss{0};
};

void ExecutionStatisticsWidget::fillModel(const QVector<quint32> histogram, const std::optional<CacheHitrates> rates)
{
    // Make sure the model has no existing items.
    // Model will make sure to delete any extra items
    model->removeRows(0, model->rowCount());

    Enu::EMnemonic mnemon;
    QMap<Enu::EMnemonic, lookup> mnemonicMap;

    // Verify that the histogram contains enough entries to be read.
    if(histogram.length() < 256) {
        qWarning() << "Histogram is not long enough";
        return;
    }
    // For every opcode from 00..FF
    for(int it = 0; it < 256; it++) {
        // Convert the int to an actual instruction
        mnemon = Pep::decodeMnemonic[it];
        // If the instruction exists, update the data in place
        if(mnemonicMap.contains(mnemon)) {
            mnemonicMap[mnemon].tally += histogram[it];
            // If the item already exists, then it must be a non-unary instruction.
            // Therefore, the first loop iteration was an addressing mode,
            // and we must adjust the address mode counter to compensate.
            if(mnemonicMap[mnemon].addrModes == 0) mnemonicMap[mnemon].addrModes += 1;
            // Each time we match a mnemonic, we have another addressing mode for the same instruction.
            mnemonicMap[mnemon].addrModes += 1;
        }
        // Otherwsie create a new data object.
        else {
            lookup entry;
            entry.start = it;
            entry.tally = histogram[it];
            // Assume a mnemonic is unary until otherwise proven.
            entry.addrModes = 0;
            mnemonicMap[mnemon] = entry;
        }

        // Now that the mnemonic lookup entry has been created, fill in cache stats if available.
        if(rates.has_value()) {
            mnemonicMap[mnemon].iread_hit   += (*rates).instructions[it].read_hit;
            mnemonicMap[mnemon].iread_miss  += (*rates).instructions[it].read_miss;
            mnemonicMap[mnemon].dread_hit   += (*rates).data[it].read_hit;
            mnemonicMap[mnemon].dread_miss  += (*rates).data[it].read_miss;
            mnemonicMap[mnemon].dwrite_hit  += (*rates).data[it].write_hit;
            mnemonicMap[mnemon].dwrite_miss += (*rates).data[it].write_miss;
        }
    }
    // Metaobjects to help convert enums to QStrings.
    static QMetaEnum mnemonicMetaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EMnemonic"));
    static QMetaEnum addrMetaenum = Enu::staticMetaObject.enumerator(Enu::staticMetaObject.indexOfEnumerator("EAddrMode"));

    // Iteratre through the key, value pairs in the map.
    for(auto kvPair = mnemonicMap.keyValueBegin(); kvPair != mnemonicMap.keyValueEnd(); ++kvPair) {
        Enu::EMnemonic mnemon = (*kvPair).first;
        auto tuple = (*kvPair).second;

        // If the instruction was not used, do not insert its entry.
        if(tuple.tally == 0 ) continue;
        // Non-unary traps all have 8 available addressing modes, regardless of what the Pep mnemonic maps indicate.
        else if(Pep::isTrapMap[mnemon] && !Pep::isUnaryMap[mnemon]) tuple.addrModes=8;

        // Create entries for the mnemonic name
        QStandardItem* instrName = new QStandardItem(QString(mnemonicMetaenum.valueToKey((int)mnemon)).toLower());

        QStandardItem* instrCount = new QStandardItem();
        QStandardItem* mnemonIReadHits = new QStandardItem();
        QStandardItem* mnemonDReadHits = new QStandardItem();
        QStandardItem* mnemonDWriteHits = new QStandardItem();

        // Make a variant from an int type to ensure that sorting works correctly.
        instrCount->setData(QVariant(tuple.tally), Qt::DisplayRole);

        if(rates.has_value()) {
            if(tuple.iread_hit + tuple.iread_miss > 0) {
                mnemonIReadHits->setData(tuple.iread_hit / ((float)tuple.iread_hit + tuple.iread_miss),
                                         Qt::DisplayRole);
            }
            if(tuple.dread_hit + tuple.dread_miss > 0) {
                mnemonDReadHits->setData(tuple.dread_hit / ((float)tuple.dread_hit + tuple.dread_miss),
                                         Qt::DisplayRole);
            }
            if(tuple.dwrite_hit + tuple.dwrite_miss > 0) {
                mnemonDWriteHits->setData(tuple.dwrite_hit / ((float)tuple.dwrite_hit + tuple.dwrite_miss),
                                         Qt::DisplayRole);
            }
        }

        model->insertRow(model->rowCount(), {instrName, instrCount,
                                             mnemonIReadHits, mnemonDReadHits, mnemonDWriteHits});

        for(int offset = 0;  offset < tuple.addrModes; offset++) {
            // If the addressing mode was not used, do not insert the entry.
            if(histogram[tuple.start + offset] == 0) continue;
            // Otherwise, for every opcode between the start and number of addressing modes,
            // figure out the addressing mode associated with the instruction.
            Enu::EAddrMode addr;
            // Trap instructions don't typically have associated addressing modes, since they are considered unary.
            // Instead, manually generate the address via knowing the bitmask
            if(Pep::isTrapMap[mnemon]) {
                addr = static_cast<Enu::EAddrMode>(1<<offset);
            }
            // If the instruction is not a trap, we can trust
            else addr = Pep::decodeAddrMode[tuple.start + offset];
            QStandardItem* addrName = new QStandardItem(QString(addrMetaenum.valueToKey((int) addr)).toLower());
            QStandardItem* addrCount = new QStandardItem();
            QStandardItem* iReadHits = new QStandardItem();
            QStandardItem* dReadHits = new QStandardItem();
            QStandardItem* dWriteHits = new QStandardItem();
            if(rates.has_value()) {
                auto instr = (*rates).instructions[tuple.start + offset];

                // Record instruction read %, if any reads occured for this instruction,
                if(instr.read_hit + instr.read_miss > 0) {
                    iReadHits->setData(instr.read_hit / ((float)instr.read_hit + instr.read_miss),
                                       Qt::DisplayRole);
                    //qDebug() << tuple.start+offset << instr.read_hit << instr.read_miss;
                } else {
                    iReadHits->setData({}, Qt::DisplayRole);
                }


                auto data = (*rates).data[tuple.start + offset];

                // Record data read %, if any reads occured for this instruction.
                if(data.read_hit + data.read_miss>0) {
                    dReadHits->setData(data.read_hit / ((float)data.read_hit + data.read_miss),
                                       Qt::DisplayRole);
                    //qDebug() << tuple.start+offset << data.read_hit << data.read_miss;
                }
                else {
                    dReadHits->setData({}, Qt::DisplayRole);
                }

                // Record data write %, if any writes occured for this instruction.
                if(data.write_hit + data.write_miss>0) {
                    dWriteHits->setData(data.write_hit / ((float)data.write_hit + data.write_miss),
                                        Qt::DisplayRole);
                    //qDebug() << tuple.start+offset << data.write_hit << data.write_miss;
                } else {
                    dWriteHits->setData({}, Qt::DisplayRole);
                }


            }
            addrCount->setData(QVariant(histogram[tuple.start + offset]), Qt::DisplayRole);

            instrName->appendRow({addrName, addrCount, iReadHits, dReadHits, dWriteHits});
        }
    }

    bool hide = true;
    if(rates.has_value() && showCacheStats) {
        hide = false;
    }

    for(int col=2; col<model->columnCount(); col++) {
        ui->treeView->setColumnHidden(col, hide);
    }

}

void ExecutionStatisticsWidget::refreshData()
{
    bool includeOS = ui->includeOSCheckBox->checkState() == Qt::CheckState::Checked;
    // Use locale so that strings have commas in them.
    ui->lineEdit_Cycles->setText(QLocale::system().toString(cpu->getCycleCount(includeOS)));
    ui->lineEdit_Instructions->setText(QLocale::system().toString(cpu->getInstructionCount(includeOS)));
    std::optional<CacheHitrates> rates = std::nullopt;
    if(showCacheStats && cpu->hasCacheStats()) {
        rates = cpu->getCacheHitRates(includeOS);
    }
    fillModel(cpu->getInstructionHistogram(includeOS), rates);
}
