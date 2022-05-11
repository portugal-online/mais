#ifdef UNITTESTS

#include "uAirTestController.hpp"
#include "hlog.h"
#include <regex>
#include "hal_types.h"

#define TAG "CONTROLLER"

static std::thread app_thread;

extern "C" {
    int app_main(int argc, char **argv);
    void test_exit_main_loop(void);
    void test_BSP_deinit();
    void test_BSP_init(int skip_shield);
    void set_speedup(float f);
    FILE *uart2_get_filedes();
    void uart2_set_filedes(FILE *f);
    void set_bsp_postinit_hook(void (*f)(void *, HAL_StatusTypeDef), void *user);
};

static void start_app()
{
    app_main(0, NULL);
}

uAirTestController::uAirTestController(): m_logfile(NULL), m_joinpolicy(true), m_oaq_max(-1.0), m_sound_max(-1.0)
{
    LoRaWAN::setNetworkInterface(this);
    setTestName(Catch::getResultCapture().getCurrentTestName());
    openLogFiles();
}

void uAirTestController::setTestName(const std::string &s)
{
    std::regex spc("\\s+");
    std::regex slash("\\\\");
    std::string new_s = std::regex_replace(s, spc, "_");
    new_s = std::string("UAIR_TEST_") + std::regex_replace(new_s, slash, "_");
    m_testname = new_s;
}

static void bsp_postinit_wrapper(void *user, HAL_StatusTypeDef r)
{
    static_cast<uAirTestController*>(user)->bspPostInit(r);
}

void uAirTestController::startApplication( float speedup )
{
    if (!app_thread.joinable())
    {
        set_speedup(speedup);

        set_bsp_postinit_hook( &bsp_postinit_wrapper, this);
        m_bsp_init_cond.reset(false);
        app_thread = std::thread(start_app);

        // Wait for BSP post init
        m_bsp_init_cond.wait(true);
    }
}

bool uAirTestController::stopApplication()
{
    if (app_thread.joinable())
    {
        test_exit_main_loop();

        app_thread.join();
        HLOG(TAG, "De-initalizing BSP");

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

LoRaUplinkMessage uAirTestController::getUplinkMessage()
{
    LoRaUplinkMessage r = m_uplink_messages.front();
    m_uplink_messages.pop();
    return r;
}

void uAirTestController::setOAQ(float base, float random_amplitude, float max)
{
    m_oaq_base = base;
    m_oaq_random = random_amplitude;
    m_oaq_max = max;
    OAQ::setOAQInterface(this);
}

void uAirTestController::setSoundLevel(float base, float random_amplitude, float max)
{
    m_sound_base = base;
    m_sound_random = random_amplitude;
    m_sound_max = max;
    vm3011_set_read_callback(vm3011, &uAirTestController::vm3011_read_callback_wrapper, this);
}

void uAirTestController::VM3011ReadCallback(struct vm3011_model*model)
{
    int val;
    if (m_sound_max<0.0)
    {
        int r = getrand(m_sound_random);
        val = m_sound_base + r;
    }
    else
    {
        val = m_sound_max;
        m_sound_max  = -1;
    }

    if (val<0)
        val = 0;
    if (val>31)
        val = 31;
    vm3011_set_gain(model, 31 - val);
}


float uAirTestController::getrand(float amplitude)
{
    int rval = (random() & 0xFFFF) - 0x8000;
    return (amplitude*rval)/32768;
}

uint16_t uAirTestController::getFAST_AQI()
{
    int val;
    if (m_oaq_max < 0.0)
    {
        int r = getrand(m_oaq_random);
        val = m_oaq_base + r;
    }
    else
    {
        val = m_oaq_max;
        m_oaq_max = -1.0;
    }

    if (val<0)
        val=0;

    if (val>500)
        val = 500;

    return val;
}

uint16_t uAirTestController::getEPA_AQI()
{
    return m_oaq_base;
}

float uAirTestController::getO3ppb()
{
    return (m_oaq_base * 6.0) + (6.0*getrand(m_oaq_random));
}

uAirTestController::~uAirTestController()
{
    HLOG(TAG, "Shutting down controller");
    LoRaWAN::unsetNetworkInterface();
    OAQ::unsetOAQInterface();

    vm3011_set_read_callback(vm3011, NULL, NULL);

    for (auto i: m_timers)
    {
        i.disconnect();
    }
    m_timers.clear();

    LoRaWAN::unjoinDevice();

    if (!stopApplication()) {
        HLOG(TAG, "De-initalizing BSP");
        test_BSP_deinit();
    }
    if (m_logfile) {
        set_host_log_file(stdout);
        uart2_set_filedes(stdout);
        fclose(m_logfile);
        m_logfile = NULL;
    }
    HLOG(TAG,"Controller shut down");
}

void uAirTestController::initBSPcore()
{
    test_BSP_init(true);
}

void uAirTestController::initBSPfull()
{
    test_BSP_init(false);
}


void uAirTestController::openLogFiles()
{
    if ((uart2_get_filedes() == NULL) || (uart2_get_filedes() == stdout))
    {
        // Try remapping to log file
        std::string logfilename = testname() + ".log";
        FILE *out = fopen(logfilename.c_str(), "w+");
        if (NULL!=out)
        {
            HLOG("CONTROLLER", "Logging to %s", logfilename.c_str());
            uart2_set_filedes(out);
            set_host_log_file(out);
            m_logfile = out;
        }
        else
        {
            HERROR("CONTROLLER", "Cannot open log file %s", logfilename.c_str());
        }
    } else {
        HERROR("CONTROLLER", "Not remapping log file=%p %p %p", uart2_get_filedes(), stdout, stderr);
    }
}

//CSignal<HAL_StatusTypeDef> &bspInitialized() { return m_bsp_init_signal; }
void uAirTestController::onBSPInit(std::function<void(void)> f)
{
    m_bsp_init_signal.connect([=](HAL_StatusTypeDef status)->bool { f(); return false; } );
}


#endif
