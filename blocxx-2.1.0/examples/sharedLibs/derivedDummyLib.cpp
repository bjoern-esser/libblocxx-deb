#include "derivedDummyLib.hpp"

DerivedDummyLib::DerivedDummyLib()
{
   m_val = 0;
}
 
DerivedDummyLib::DerivedDummyLib(int val)
{
   m_val = val;
}

DerivedDummyLib::~DerivedDummyLib()
{
}




char *getVersion()
{
   return (char *)"derivedDummyLib:  shared library example:  dummyLib version 1.0";
}

DummyLib *createDummyLibWithVal(int val)
{
   return (DummyLib*) new DerivedDummyLib(val);
}

DummyLib *createDummyLib(void)
{
   return (DummyLib*) new DerivedDummyLib();
}
