#ifndef UAIR_MODULE_TEST_FIXTURE_H__
#define UAIR_MODULE_TEST_FIXTURE_H__

#ifdef UNITTESTS

#include "uAirTestController.hpp"

class uAirModuleTestFixture: public uAirTestController
{
public:
    uAirModuleTestFixture();
    virtual ~uAirModuleTestFixture();

    virtual void init();
};

#endif

#endif
