#ifndef DUMMYLIB_HPP_INCLUDE_GUARD_
#define DUMMYLIB_HPP_INCLUDE_GUARD_


// This is the only external stuff to the dummyLib
// All the internal handlings of the dummyLib are in sharedDummyLib.hpp

class DummyLib
{
   public:
      DummyLib();
      DummyLib(int val);
      virtual ~DummyLib();

      void setVal(int val);
      int getVal();

      virtual char *getName() = 0;

   protected:
      int m_val;

};

#endif //_DUMMYLIB_HPP_
