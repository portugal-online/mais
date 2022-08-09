#include "cap.h"

static double cap_farad = 5.0;
static unsigned cap_mv;
static double cap_coulombs;
static double cap_joules;

void cap_update_voltage(unsigned mv)
{
    cap_mv = mv;

    double mvd = (double)mv / 1000.0;  // In volts

    cap_coulombs = mvd * cap_farad;
    cap_joules = (double)(mvd*mvd) * cap_farad/2.0;
}


double cap_get_charge_coulombs()
{
    return cap_coulombs;
}


double cap_get_energy_joules()
{
    return cap_joules;
}
