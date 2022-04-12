#include "uAirTestController.hpp"

static std::thread app_thread;

extern "C" {
    int app_main(int argc, char **argv);
    void test_exit_main_loop(void);
    void test_BSP_deinit();
    void test_BSP_init();
    void set_speedup(float f);
};

static void start_app()
{
    app_main(0, NULL);
}

uAirTestController::uAirTestController(): m_joinpolicy(true)
{
    LoRaWAN::setNetworkInterface(this);
}

void uAirTestController::startApplication( float speedup )
{
    if (!app_thread.joinable())
    {
        set_speedup(speedup);
        app_thread = std::thread(start_app);
    }
}

bool uAirTestController::stopApplication()
{
    if (app_thread.joinable())
    {
        test_exit_main_loop();

        app_thread.join();

        test_BSP_deinit();
        return true;
    }

    return false;
}

bool uAirTestController::deviceJoined()
{
    return LoRaWAN::hasDeviceJoined();
}

bool uAirTestController::handleUserUplinkMessage(const LoRaUplinkMessage &m)
{
    m_uplink_messages.push(m);
    return false;
}

bool uAirTestController::handleJoin()
{
    return m_joinpolicy;
}

void uAirTestController::setJoinPolicy(bool allow_join)
{
    m_joinpolicy = allow_join;
}

void uAirTestController::onTimerUpdated( std::function<void (uint32_t, uint32_t)> f )
{
    m_timers.push_back (
                        rtc_timer_signal().connect(
                                                   [=](uint32_t old_value,uint32_t new_value)->bool
                                                   {
                                                       f(old_value,new_value); return true;
                                                   }
                                                  )
                       );
}

uAirTestController::~uAirTestController()
{
    LoRaWAN::unsetNetworkInterface();

    for (auto i: m_timers)
    {
        i.disconnect();
    }
    m_timers.clear();

    LoRaWAN::unjoinDevice();

    if (!stopApplication())
        test_BSP_deinit();
}


