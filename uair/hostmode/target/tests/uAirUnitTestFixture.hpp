#ifndef UAIR_UNIT_TEST_FIXTURE_H__
#define UAIR_UNIT_TEST_FIXTURE_H__

#ifdef UNITTESTS

#include "uAirTestController.hpp"

class uAirUnitTestFixture: public uAirTestController
{
public:
    uAirUnitTestFixture();
    virtual ~uAirUnitTestFixture();

    virtual void init();
};

#endif

#endif
