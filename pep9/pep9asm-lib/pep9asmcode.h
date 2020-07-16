#ifndef PEP9ASMCODE_H
#define PEP9ASMCODE_H
#include "assembler/asmcode.h"

#include "isadefs.h"
// Concrete code classes
class UnaryInstruction: public AsmCode
{
public:
    UnaryInstruction() = default;
    ~UnaryInstruction() override = default;
    UnaryInstruction(const UnaryInstruction& other);
    UnaryInstruction& operator=(UnaryInstruction other);
    AsmCode *cloneAsmCode() const override;


    void appendObjectCode(QList<int> &objectCode) const override;
//    NO LONGER WITH PEP/9. FOR RET1, RET2, ..., RET7
//    bool processSymbolTraceTags(int &sourceLine, QString &errorString);

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasBreakpoint() const override;
    void setBreakpoint(bool b) override;
    bool isCode() const  override { return true;}

    Pep9::ISA::EMnemonic getMnemonic() const;
    void setMnemonic(Pep9::ISA::EMnemonic);

    bool tracksTraceTags() const override;

    friend void swap(UnaryInstruction& first, UnaryInstruction& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.breakpoint, second.breakpoint);
        swap(first.mnemonic, second.mnemonic);
    }
private:
    Pep9::ISA::EMnemonic mnemonic;
    bool breakpoint = false;
};

class NonUnaryInstruction: public AsmCode
{
public:
    NonUnaryInstruction() = default;
     ~NonUnaryInstruction() override = default;
    NonUnaryInstruction(const NonUnaryInstruction& other);
    NonUnaryInstruction& operator=(NonUnaryInstruction other);
    AsmCode *cloneAsmCode() const override;
    // ~NonUnaryInstruction() { delete argument; }
    void appendObjectCode(QList<int> &objectCode) const override;

    // AsmCode interface
    QString getAssemblerListing() const override;
    QString getAssemblerSource() const override;
    quint16 objectCodeLength() const override;

    bool hasBreakpoint() const override;
    void setBreakpoint(bool b) override;
    bool isCode() const  override { return true;}

    Pep9::ISA::EMnemonic getMnemonic() const;
    void setMnemonic(Pep9::ISA::EMnemonic);

    Pep9::ISA::EAddrMode getAddressingMode() const;
    void setAddressingMode(Pep9::ISA::EAddrMode);

    bool hasSymbolicOperand() const override;
    QSharedPointer<const SymbolEntry> getSymbolicOperand() const override;
    QSharedPointer<AsmArgument> getArgument() const;
    void setArgument(QSharedPointer<AsmArgument>);

    bool tracksTraceTags() const override;

    friend void swap(NonUnaryInstruction& first, NonUnaryInstruction& second)
    {
        using std::swap;
        swap(static_cast<AsmCode&>(first), static_cast<AsmCode&>(second));
        swap(first.mnemonic, second.mnemonic);
        swap(first.addressingMode, second.addressingMode);
        swap(first.argument, second.argument);
        swap(first.breakpoint, second.breakpoint);
    }
private:
    Pep9::ISA::EMnemonic mnemonic;
    Pep9::ISA::EAddrMode addressingMode;
    QSharedPointer<AsmArgument> argument = nullptr;
    bool breakpoint = false;
};


#endif // PEP9ASMCODE_H
