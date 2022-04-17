#include "tests/uAirSystemTestFixture.hpp"
#include <iostream>

TEST_CASE_METHOD(uAirSystemTestFixture, "UAIR system tests - network", "[SYS][SYS/Network]")
{
    setOAQ( 35.0, 12.0 ); // 35.0 +- 12.0(random)

    startApplication( 200.0 ); // 200x speedup

    /* Wait at least 10 seconds. The device must have joined network. */

    waitFor(std::chrono::seconds(10));

    CHECK( deviceJoined() );

    /* Wait 75+5 minutes. After this time the device must have sent one payload */

    waitFor(std::chrono::minutes(75+5));

    CHECK( uplinkMessages().size() == 1 );

    LoRaUplinkMessage m = getUplinkMessage();

    std::cout<<"Message: "<<m;
}

