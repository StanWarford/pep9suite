#pragma once
#include <QtCore>
enum SymbolType
{
    EMPTY,LOCATION,
};
class AbstractSymbolValue
{
public:
	AbstractSymbolValue();
	virtual ~AbstractSymbolValue();
	virtual qint32 getValue() const = 0;
    virtual SymbolType getSymbolType() const = 0;
};

class SymbolValueEmpty :
public AbstractSymbolValue
{
public:
    SymbolValueEmpty();
    virtual ~SymbolValueEmpty();
    
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

class SymbolValueLocation :
public AbstractSymbolValue
{
    quint16 _value;
public:
    explicit SymbolValueLocation(quint16 value);
    virtual ~SymbolValueLocation();
    void setValue(quint16);
    // Inherited via AbstractSymbolValue
    virtual qint32 getValue() const override;
    virtual SymbolType getSymbolType() const override;
};

