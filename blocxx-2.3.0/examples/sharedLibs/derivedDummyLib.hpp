#ifndef DERIVED_DUMMYLIB_HPP_INCLUDE_GUARD_
#define DERIVED_DUMMYLIB_HPP_INCLUDE_GUARD_

#include "dummyLib.hpp"

// This is the only external stuff to the dummyLib
// All the internal handlings of the dummyLib are in sharedDummyLib.hpp

class DerivedDummyLib : DummyLib
{
   public:
      DerivedDummyLib();
      DerivedDummyLib(int val);
      virtual ~DerivedDummyLib();

      char *getName() {return (char *)"derivedDummyLib";};
};

extern "C"
{

DummyLib *createDummyLibWithVal(int val);
DummyLib *createDummyLib(void);
char* getVersion(void);

} // extern "C"

#endif //DERIVED_DUMMYLIB_HPP_
