#include "tests/uAirTestController.hpp"
#include <iostream>

TEST_CASE("UAIR system tests - network", "[SYS][SYS/Network]")
{
    uAirTestController controller;

    controller.setOAQ( 35.0, 12.0 ); // 35.0 +- 12.0(random)

    controller.startApplication( 200.0 ); // 200x speedup

    /* Wait at least 10 seconds. The device must have joined network. */

    controller.waitFor(std::chrono::seconds(10));

    CHECK( controller.deviceJoined() );

    /* Wait 75+5 minutes. After this time the device must have sent one payload */

    controller.waitFor(std::chrono::minutes(75+5));

    CHECK( controller.uplinkMessages().size() == 1 );

    LoRaUplinkMessage m = controller.getUplinkMessage();

    std::cout<<"Message: "<<m;
}

