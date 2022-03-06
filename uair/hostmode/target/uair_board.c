#include "stm32wlxx_hal_def.h"
#include <stdlib.h>
#include <stdio.h>
#include "stm32wlxx_hal_i2c_pvt.h"
#include "models/hs300x.h"
#include "models/shtc3.h"
#include "models/zmod4510.h"
#include "models/vm3011.h"
#include "models/hw_rtc.h"
#include "models/hw_interrupts.h"
#include "system_linux.h"


uint32_t i2c1_power = 0;
uint32_t i2c2_power = 0;
uint32_t i2c3_power = 0;

struct hs300x_model *hs300x;
struct shtc3_model *shtc3;;
struct zmod4510_model *zmod4510;
struct vm3011_model *vm3011;

void i2c1_power_control_write(void *user, int val)
{
    i2c1_power = !!val;
    if (!i2c1_power)
        shtc3_powerdown(shtc3);
    else
        shtc3_powerup(shtc3);
}

void i2c2_power_control_write(void *user, int val)
{
    i2c2_power = !!val;
}

void i2c3_power_control_write(void *user, int val)
{
    i2c3_power = !!val;
    if (!i2c3_power) {
        hs300x_powerdown(hs300x);
        zmod4510_powerdown(zmod4510);
    }
    else {
        hs300x_powerup(hs300x);
        zmod4510_powerup(zmod4510);
    }
}

int i2c1_scl_read(void *user)
{
    return i2c1_power;
}

int i2c1_sda_read(void *user)
{
    return i2c1_power;
}

int i2c2_scl_read(void *user)
{
    return i2c2_power;
}

int i2c2_sda_read(void *user)
{
    return i2c2_power;
}

int i2c3_scl_read(void *user)
{
    return i2c3_power;
}

int i2c3_sda_read(void *user)
{
    return i2c3_power;
}

int board_ver_read(void *user)
{
    // R2
    return 0;
}

void gpio_write_ignore(void *user, int value)
{
}

void zmod4510_gpio_reset(void *user, int value)
{
}


void __preinit()
{
    init_interrupts();

    // Register GPIO handlers

    GPIOA->def[4].ops.read = board_ver_read;

    GPIOB->def[2].ops.write = i2c2_power_control_write; // Mic/I2C2
    GPIOA->def[6].ops.write = i2c3_power_control_write; // Sens/I2C3
    GPIOC->def[2].ops.write = i2c1_power_control_write; //

    GPIOA->def[12].ops.read = i2c2_scl_read;   // Mic
    GPIOA->def[12].ops.write = gpio_write_ignore;
    GPIOA->def[11].ops.read = i2c2_sda_read;
    GPIOA->def[11].ops.write = gpio_write_ignore;

    GPIOB->def[8].ops.read = i2c1_scl_read;
    GPIOB->def[8].ops.write = gpio_write_ignore;
    GPIOA->def[10].ops.read = i2c1_sda_read;
    GPIOA->def[10].ops.write = gpio_write_ignore;

    GPIOC->def[0].ops.read = i2c3_scl_read;
    GPIOC->def[0].ops.write = gpio_write_ignore;

    GPIOC->def[1].ops.read = i2c3_sda_read;
    GPIOC->def[1].ops.write = gpio_write_ignore;

    GPIOC->def[13].ops.write = zmod4510_gpio_reset;

    // Debug/Misc pins
    GPIOA->def[7].ops.write = gpio_write_ignore;
    GPIOA->def[8].ops.write = gpio_write_ignore;
    GPIOA->def[9].ops.write = gpio_write_ignore;
    GPIOB->def[13].ops.write = gpio_write_ignore; // Microphone SCK (ZPL)

    hs300x = hs300x_model_new();

    i2c_register_device(I2C3, 0x44, &hs300x_ops, hs300x);

    shtc3 = shtc3_model_new();

    i2c_register_device(I2C1, 0x70, &shtc3_ops, shtc3);

    zmod4510 = zmod4510_model_new();

    i2c_register_device(I2C3, 0x33, &zmod4510_ops, zmod4510);

    vm3011 = vm3011_model_new();

    i2c_register_device(I2C2, 0xC2>>1, &vm3011_ops, vm3011);

    // Set sane defaults
    hs300x_set_temperature(hs300x, 25.43F);
    hs300x_set_humidity(hs300x, 65.30F);

    shtc3_set_temperature(shtc3, 28.21F);
    shtc3_set_humidity(shtc3, 53.20F);

    vm3011_set_gain(vm3011, 31);

    rtc_engine_init();

}


