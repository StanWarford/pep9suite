#include "updatechecker.h"
//#include <unistd.h>
UpdateChecker::UpdateChecker(QObject *parent): QObject(parent)
{

}

UpdateChecker::~UpdateChecker()
{

}

void UpdateChecker::beginUpdateCheck()
{
    //sleep(2);
    //emit this->updateInformation(5);
}
