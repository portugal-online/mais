#ifndef UAIR_TEST_H__
#define UAIR_TEST_H__

#include <limits>
#include <catch2/catch.hpp>
#include "models/network/lorawan.hpp"
#include <thread>
#include <chrono>
#include <signal.h>
#include "csignal.hpp"
#include "ccondition.hpp"
#include "models/hw_rtc.h"
#include "models/OAQ.hpp"

using namespace std::chrono_literals;

struct uAirTestController: public NetworkInterface, public OAQInterface
{
    uAirTestController();
    virtual ~uAirTestController();
    /**
     * @brief Start the uAir application for system tests
     */
    void startApplication(float speed=1.0F);
    /**
     * @brief Stop uAir application for system tests
     */
    bool stopApplication();

    /**
     * @brief Check if device has joined
     */
    bool deviceJoined();

    /**
     * @brief Set join policy
     */
    void setJoinPolicy(bool allow_join);

    /**
     * @brief Set OAQ
     */
    void setOAQ(float base, float random_amplitude);

    /**
     * @bried wait for a specified amount of device time
     */
    template< class Rep, class Period >
    void waitFor(const std::chrono::duration<Rep, Period> & duration)
    {
        CCondition<bool> *elapsed = new CCondition<bool>(false);

        auto ticks = rtc_engine_get_ticks() + ((std::chrono::microseconds(duration).count() * 1024)/1000000);

        auto timer = rtc_timer_signal().connect(
                                                [elapsed, ticks](uint32_t old_value,uint32_t new_value)->bool
                                                {
                                                    //printf("Ticks %d %d %d\n", old_value, new_value, ticks);
                                                    if ((new_value >= ticks ) && ( old_value<ticks))
                                                    {
                                                        elapsed->set(true);
                                                        return false;
                                                    }
                                                    return true;
                                                }
                                               );
        elapsed->wait( true );
        timer.disconnect();
        delete (elapsed);


    }

    const std::queue< LoRaUplinkMessage > &uplinkMessages() const
    {
        return m_uplink_messages;
    }

    LoRaUplinkMessage getUplinkMessage();


    void onTimerUpdated( std::function<void (uint32_t, uint32_t)> );



    /* LoRaWAN network interface */

    virtual bool handleUserUplinkMessage(const LoRaUplinkMessage &);

    /* Handle join. Return true if join accepted, false otherwise */
    virtual bool handleJoin();

    virtual uint16_t getFAST_AQI();
    virtual uint16_t getEPA_AQI();
    virtual float getO3ppb();

    float getrand(float amplitude);

private:
    /* OAQ */
    float oaq_base;
    float oaq_random;

    std::vector< CSignalID > m_timers;
    std::queue< LoRaUplinkMessage > m_uplink_messages;
    bool m_joinpolicy;
};

#endif
