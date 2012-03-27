#include <blocxx/BLOCXX_config.h>
#include "dummyLib.hpp"

DummyLib::DummyLib()
{
   m_val = 0;
}

DummyLib::DummyLib(int val)
{
   m_val = val;
}

DummyLib::~DummyLib()
{
}

void DummyLib::setVal(int val)
{
   m_val = val;
}

int DummyLib::getVal()
{
   return m_val;
}

char *getVersion()
{
   return (char *)"dummyLib base class: shared library example:  dummyLib version 1.0";
}
