// File: aboutpep.h
/*
    The Pep/9 suite of applications (Pep9, Pep9CPU, Pep9Micro) are
    simulators for the Pep/9 virtual machine, and allow users to
    create, simulate, and debug across various levels of abstraction.
    
    Copyright (C) 2010  J. Stanley Warford, Pepperdine University

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

#ifndef ABOUTPEP_H
#define ABOUTPEP_H

#include <QDialog>

namespace Ui {
    class AboutPep;
}

/*
 * Class that serves a typical "about this application" html text document,
 * along with a custom 150x150px icon.
 *
 * This replaces the myriad of old about dialogs, and should reduce
 * the number of
 */
class AboutPep : public QDialog {
    Q_OBJECT
public:
    // Icons will be scaled to 150x150 px.
    AboutPep(QString aboutText, QPixmap icon, QWidget *parent = nullptr);
    ~AboutPep();

private:
    Ui::AboutPep *ui;
    QString str;
    QPixmap icon;
};

#endif // ABOUTPEP_H
