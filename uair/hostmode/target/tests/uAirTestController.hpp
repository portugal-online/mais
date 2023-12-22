#ifndef UAIR_TEST_CONTROLLER_H__
#define UAIR_TEST_CONTROLLER_H__

#ifdef UNITTESTS

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
#include "hal_types.h"
#include <ostream>
#include <sstream>
#include <functional>

#include "models/hs300x.h"
#include "models/vm3011.h"
#include "models/shtc3.h"

extern "C"
{
    extern struct hs300x_model *hs300x;
    extern struct vm3011_model *vm3011;
    extern struct shtc3_model *shtc3;
};

template<typename T>
class Range : public Catch::MatcherBase<T> {
    T m_begin, m_end;
public:
    Range( T begin, T end ) : m_begin( begin ), m_end( end ) {}

    bool match( T const& i ) const override
    {
        return i >= m_begin && i <= m_end;
    }

    virtual std::string describe() const override {
        std::ostringstream ss;
        ss << "is between " << m_begin << " and " << m_end;
        return ss.str();
    }
};

// The builder function
template<typename T>
inline Range<T> IsBetween( T begin, T end ) {
    return Range<T>( begin, end );
}


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
     * @brief Set OAQ.
     *
     * @param base Base value of OAQ.
     * @param random_amplitude Random amplitude.
     * @param force_max Force 1st sample to this value to ensure a known max.
     *
     */
    void setOAQ(float base, float random_amplitude, float force_max=-1);

    /**
     * @brief Set sound level.
     *
     * @param base Base value of sound level.
     * @param random_amplitude Random amplitude.
     * @param force_max Force 1st sample to this value to ensure a known max.
     *
     */
    void setSoundLevel(float base, float random_amplitude, float force_max=-1);


    /**
     * @bried wait for a specified amount of device time
     */
    template< class Rep, class Period >
    void waitFor(const std::chrono::duration<Rep, Period> & duration)
    {
        CCondition<bool> *elapsed = new CCondition<bool>(false);

        unsigned ticks = rtc_engine_get_ticks() + ((std::chrono::microseconds(duration).count() * 1024)/1000000);

        do_log("CONTROLLER", LEVEL_PROGRESS, "","", __LINE__, "Run until %lu (from %lu, %lu)",
               ticks,
               rtc_engine_get_ticks(), std::chrono::microseconds(duration).count());

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
    const std::string &testname() const { return m_testname; }

    void bspPostInit(HAL_StatusTypeDef status) {
        m_bsp_init_cond.set(true);
        m_bsp_init_signal.emit(status);
    }

    //CSignal<HAL_StatusTypeDef> &bspInitialized() { return m_bsp_init_signal; }
    void onBSPInit(std::function<void(void)> );

protected:
    void openLogFiles();

    void setTestName(const std::string &s);

    /**
     * @brief Initialise BSP (core only)
     */
    void initBSPcore();

    /**
     * @brief Initialise BSP (full)
     */
    void initBSPfull();

    static void vm3011_read_callback_wrapper(void *user, struct vm3011_model*model)
    {
        static_cast<uAirTestController*>(user)->VM3011ReadCallback(model);
    }

    void VM3011ReadCallback(struct vm3011_model*model);

private:
    CSignal<HAL_StatusTypeDef> m_bsp_init_signal;
    CCondition<bool> m_bsp_init_cond;
    std::string m_testname;
    FILE *m_logfile;
    std::vector< CSignalID > m_timers;
    std::queue< LoRaUplinkMessage > m_uplink_messages;
    bool m_joinpolicy;

    /* OAQ */
    float m_oaq_base;
    float m_oaq_random;
    float m_oaq_max;
    /* Sound */
    float m_sound_base;
    float m_sound_random;
    float m_sound_max;

};

#endif

#endif
