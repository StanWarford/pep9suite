#pragma once
#include "symboltable.h"
#include <qstring.h>
enum DefStates
{
    SINGLE, MULTIPLE, UNDEFINED
};
class SymbolEntry
{
private:
	SymbolTable::SymbolID _symbolID;
	QString _name;
	SymbolTable::AbstractSymbolValuePtr _value;
	DefStates _state;
public:
	SymbolEntry(SymbolTable::SymbolID symbolID, QString name);
	SymbolEntry(SymbolTable::SymbolID symbolID, QString name, SymbolTable::AbstractSymbolValuePtr value);
	~SymbolEntry();
	std::shared_ptr<SymbolTable> getParentTable() const;
	void setValue(SymbolTable::AbstractSymbolValuePtr value);
	QString getName() const;
	bool isDefined() const;
	bool isUndefined() const;
	bool isMultiplyDefined() const;
    void setMultiplyDefined();
	SymbolTable::SymbolID getSymbolID() const;
	qint32 getValue() const;
	SymbolTable::AbstractSymbolValuePtr getRawValue();



};

