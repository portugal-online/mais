#include "tests/uAirSystemTestFixture.hpp"
#include "tests/uAirUplinkMessage.hpp"
#include <iostream>

TEST_CASE_METHOD(uAirSystemTestFixture, "UAIR system tests - network", "[SYS][SYS/Network]")
{

    onBSPInit([this]
              {
                  setOAQ( 35.0, 2.0, 40.0 ); // 35.0 +- 2.0(random), abs max 40.0
                  setSoundLevel( 8.0, 2.0, 16.0 ); // 8 +- 2, abs max 16
              }
             );

    startApplication( 200.0 ); // 200x speedup

    /* Wait at least 10 seconds. The device must have joined network. */

    waitFor(std::chrono::seconds(10));

    CHECK( deviceJoined() );

    /* Wait 75+5 minutes. After this time the device must have sent one payload */

    waitFor(std::chrono::minutes(75+5));

    CHECK( uplinkMessages().size() == 1 );

    if (uplinkMessages().empty())
        return;

    LoRaUplinkMessage m = getUplinkMessage();

    std::cout<<"Message: "<<m<<std::endl;

    uAirUplinkMessage *upm = uAirUplinkMessage::create(m);

    if (upm->type() == 0) {
        upm->dump(std::cout);

        uAirUplinkMessageType0 *up = static_cast<uAirUplinkMessageType0*>(upm);

        CHECK( up->OAQValid() );

        if (up->OAQValid()) {
            CHECK( up->maxOAQ() ==  40 );
            CHECK( up->averageOAQ() == 35 );
        }

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
            CHECK( up->maximumSoundLevel() == 16);
            CHECK_THAT( up->averageSoundLevel(), Range<int>(8-2, 8+2));
        }
    }
}

struct hs_error_t
{
    unsigned cycle;
};

static hs_error_t hs_error;

i2c_status_t hs300x_receive_handler(struct hs300x_model *, void *userdata, uint8_t *pData, uint16_t Size)
{
    hs_error_t *e = (hs_error_t*)userdata;
    e->cycle++;
    if ((e->cycle & 7) == 7)
        return HAL_I2C_ERROR_AF;
    return HAL_OK;
}

i2c_status_t hs300x_transmit_handler(struct hs300x_model *, void *userdata, const uint8_t *pData, uint16_t Size)
{
    return HAL_OK;
}

TEST_CASE_METHOD(uAirSystemTestFixture, "UAIR system tests - resilience external temp", "[SYS][SYS/Resilience][SYS/Resilence/ExternalTemp]")
{
    hs_error.cycle = 0;

    // Set up HS300x hooks after BSP initialises

    onBSPInit([this]
              {
                  hs300x_set_receive_hook(hs300x, hs300x_receive_handler, &hs_error);
                  hs300x_set_transmit_hook(hs300x, hs300x_transmit_handler, &hs_error);
                  setSoundLevel( 8.0, 2.0, 16.0 ); // 8 +- 2, abs max 16
              }
             );

    startApplication( 200.0 ); // 200x speedup

    waitFor(std::chrono::seconds(10));

    CHECK( deviceJoined() );

    waitFor(std::chrono::minutes(75+5));

    CHECK( uplinkMessages().size() == 1 );

    if (!uplinkMessages().empty()) {

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
                CHECK( up->maximumSoundLevel() == 16);
                CHECK_THAT( up->averageSoundLevel(), Range<uint8_t>(8-2, 8+2));
            }
        }
    }

    hs300x_set_receive_hook(hs300x, NULL, NULL);
    hs300x_set_transmit_hook(hs300x, NULL, NULL);
}
