#include "tests/uAirSystemTestFixture.hpp"
#include "tests/uAirUplinkMessage.hpp"
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

    std::cout<<"Message: "<<m<<std::endl;

    uAirUplinkMessage *upm = uAirUplinkMessage::create(m);
    if (upm->type() == 0) {
        upm->dump(std::cout);
        uAirUplinkMessageType0 *up = static_cast<uAirUplinkMessageType0*>(upm);


        CHECK( up->OAQValid() );

        CHECK( up->externalTHValid() );
        if (up->externalTHValid())
        {
            CHECK( up->averageExternalTemperature() == 25.50 );
            CHECK( up->averageExternalHumidity() == 65 );
        }

        CHECK( up->internalTHValid() );
        if ( up->internalTHValid() )
        {
            CHECK( up->maximumInternalTemperature() == 28.25 );
            CHECK( up->maximumInternalHumidity() == 53 );
        }

        CHECK( up->microphoneValid() );
        if (up->microphoneValid() )
        {
            CHECK( up->maximumSoundLevel() == 0);
            CHECK( up->averageSoundLevel() == 0);
        }
    }
}

