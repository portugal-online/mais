#include "uAirUnitTestFixture.hpp"

#ifdef UNITTESTS

uAirUnitTestFixture::uAirUnitTestFixture()
{
    init();
}

uAirUnitTestFixture::~uAirUnitTestFixture()
{
}

void uAirUnitTestFixture::init()
{
    initBSPcore();
}


#endif