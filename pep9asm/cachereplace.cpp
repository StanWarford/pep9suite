#include "cachereplace.h"

AReplacementFactory::AReplacementFactory(quint16 associativity): associativity(associativity)
{

}

quint16 AReplacementFactory::get_associativity() const
{
    return associativity;
}
