#ifndef CACHECONFIG_H
#define CACHECONFIG_H

#include <QWidget>
#include <QSharedPointer>

class CacheMemory;


namespace Ui {
class CacheConfig;
}
// File: cacheconfig.h
/*
    Pep9 is a virtual machine for writing machine language and assembly
    language programs.

    Copyright (C) 2020  Matthew McRaven & J. Stanley Warford, Pepperdine University

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

/*
 * This UI allows users to view and modify the configuration of a cache.
 */
class CacheConfig : public QWidget
{
    Q_OBJECT

public:
    explicit CacheConfig(QWidget *parent = nullptr);
    ~CacheConfig();
    void init(QSharedPointer<CacheMemory> cache, bool enableCacheChanges);
    // Change if configuration boxes can be changed, or if they are read-only.
    void setReadOnly(bool readOnly);

public slots:
    void onCacheConfigChanged();
    // Handle font changes.
    void onFontChanged(QFont current);
    void onDarkModeChanged(bool darkMode);

private:
    Ui::CacheConfig *ui;
    QSharedPointer<CacheMemory> cache;
    bool valuesChanged = false;
    bool enableCacheChanges = true;
    // Determine if the "Update Configuration" button should be enabled or not
    void updateButtonRefresh();
    // Compute the offset bits from the index, tag bits. Also compute the maximum
    // value that may be placed in either tag or index fields.
    void updateAddressBits();
    // Number of bits needed to represent a memory address.
    static const quint16 memory_bits = 16;

    // Respond to values in configuration being changed.
private slots:
    void on_tagBits_valueChanged(int newValue);
    void on_indexBits_valueChanged(int newValue);
    void on_offsetBits_valueChanged(int newValue);
    void on_associativityNum_valueChanged(int newValue);
    void on_replacementCombo_currentIndexChanged(int);
    void on_writeAllocationCombo_currentIndexChanged(int);
    void on_updateButton_pressed();
};

#endif // CACHECONFIG_H
