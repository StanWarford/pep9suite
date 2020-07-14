#ifndef ASMPARSERHELPER_H
#define ASMPARSERHELPER_H

#include <QString>
bool startsWithHexPrefix(QString str);
// Post: Returns true if str starts with the characters 0x or 0X. Otherwise returns false.

int charStringToInt(QString str);
// Pre: str is enclosed in single quotes.
// Post: Returns the ASCII integer value of the character accounting for \ quoted characters.

int string2ArgumentToInt(QString str);
// Pre: str is enclosed in double quotes and contains at most two possibly quoted characters.
// Post: Returns the two-byte ASCII integer value for the string.

void unquotedStringToInt(QString &str, int &value);
// Pre: str is a character or string stripped of its single or double quotes.
// Post: The sequence of characters representing the first possibly \ quoted character
// is stripped from the beginning of str.
// Post: value is the ASCII integer value of the first possibly \ quoted character.

int byteStringLength(QString str);
// Pre: str is a double quoted string.
// Post: Returns the byte length of str accounting for possibly \ quoted characters.
#endif // ASMPARSERHELPER_H
