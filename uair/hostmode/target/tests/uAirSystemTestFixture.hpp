#ifndef UAIR_SYSTEM_TEST_FIXTURE_H__
#define UAIR_SYSTEM_TEST_FIXTURE_H__

#ifdef UNITTESTS

#include "uAirTestController.hpp"

class uAirSystemTestFixture: public uAirTestController
{
public:
    uAirSystemTestFixture();
    virtual ~uAirSystemTestFixture();

    virtual void init();
};

#endif

#endif
