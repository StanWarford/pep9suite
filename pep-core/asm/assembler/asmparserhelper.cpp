#include "asmparserhelper.h"

bool startsWithHexPrefix(QString str)
{
    if (str.length() < 2) return false;
    if (str[0] != '0') return false;
    if (str[1] == 'x' || str[1] == 'X') return true;
    return false;
}

int charStringToInt(QString str)
{
    str.remove(0, 1); // Remove the leftmost single quote.
    str.chop(1); // Remove the rightmost single quote.
    int value;
    unquotedStringToInt(str, value);
    return value;
}

int string2ArgumentToInt(QString str)
{
    int valueA, valueB;
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    unquotedStringToInt(str, valueA);
    if (str.length() == 0) {
        return valueA;
    }
    else {
        unquotedStringToInt(str, valueB);
        valueA = 256 * valueA + valueB;
        if (valueA < 0) {
            valueA += 65536; // Stored as two-byte unsigned.
        }
        return valueA;
    }
}

void unquotedStringToInt(QString &str, int &value)
{
    QString s;
    if (str.startsWith("\\x") || str.startsWith("\\X")) {
        str.remove(0, 2); // Remove the leading \x or \X
        s = str.left(2);
        str.remove(0, 2); // Get the two-char hex number
        bool ok;
        value = s.toInt(&ok, 16);
    }
    else if (str.startsWith("\\")) {
        str.remove(0, 1); // Remove the leading bash
        s = str.left(1);
        str.remove(0,1);
        if (s == "b") { // backspace
            value = 8;
        }
        else if (s == "f") { // form feed
            value = 12;
        }
        else if (s == "n") { // line feed (new line)
            value = 10;
        }
        else if (s == "r") { // carriage return
            value = 13;
        }
        else if (s == "t") { // horizontal tab
            value = 9;
        }
        else if (s == "v") { // vertical tab
            value = 11;
        }
        else {
            value = QChar(s[0]).toLatin1();
        }
    }
    else {
        s = str.left(1);
        str.remove(0, 1);
        value = QChar(s[0]).toLatin1();
    }
    value += value < 0 ? 256 : 0;
}

int byteStringLength(QString str)
{
    str.remove(0, 1); // Remove the leftmost double quote.
    str.chop(1); // Remove the rightmost double quote.
    int length = 0;
    while (str.length() > 0) {
        if (str.startsWith("\\x") || str.startsWith("\\X")) {
            str.remove(0, 4); // Remove the \xFF
        }
        else if (str.startsWith("\\")) {
            str.remove(0, 2); // Remove the quoted character
        }
        else {
            str.remove(0, 1); // Remove the single character
        }
        length++;
    }
    return length;
}
