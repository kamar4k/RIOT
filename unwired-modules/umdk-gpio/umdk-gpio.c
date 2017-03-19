/*
 * Copyright (C) 2016 cr0s
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file
 * @brief
 * @author      cr0s
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "periph/gpio.h"

#include "board.h"
#include "unwds-common.h"

#include "umdk-gpio.h"

static bool set(int num, bool one)
{
    gpio_t gpio = unwds_gpio_pin(num);

    if (gpio == 0) {
        return false;
    }

    gpio_init(gpio, GPIO_OUT);

    if (one) {
        gpio_set(gpio);
    }
    else {
        gpio_clear(gpio);
    }

    return true;
}

static bool get(int num)
{
    gpio_t gpio = unwds_gpio_pin(num);

    gpio_init(gpio, GPIO_IN);

    return gpio_read(gpio);
}

static bool toggle(int num)
{
    gpio_t gpio = unwds_gpio_pin(num);

    if (gpio == 0) {
        return false;
    }

    gpio_toggle(gpio);

    return true;
}


void umdk_gpio_init(uint32_t *non_gpio_pin_map, uwnds_cb_t *event_callback)
{
    (void) non_gpio_pin_map;
    (void) event_callback;
}

static inline void do_reply(module_data_t *reply, umdk_gpio_reply_t reply_code)
{
    reply->length = UMDK_GPIO_DATA_LEN + 1;
    reply->data[0] = UNWDS_GPIO_MODULE_ID;
    reply->data[1] = reply_code;
}

static bool check_pin(module_data_t *reply, int pin)
{
    /* Is pin is occupied by other module */
    if (unwds_is_pin_occupied(pin)) {
        do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
        return false;
    }

    /* Gpio pin not in range */
    if (pin < 0 || unwds_gpio_pin(pin) == 0) {
        do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
        return false;
    }

    return true;
}

static bool gpio_cmd(module_data_t *cmd, module_data_t *reply, bool with_reply)
{
    if (cmd->length != UMDK_GPIO_DATA_LEN) {
        if (with_reply) {
            do_reply(reply, UMDK_GPIO_REPLY_ERR_FORMAT);
        }
        return false;
    }

    uint8_t value = cmd->data[0];
    uint8_t pin = value & UMDK_GPIO_PIN_MASK;
    umdk_gpio_action_t act = (value & UMDK_GPIO_ACT_MASK) >> UMDK_GPIO_ACT_SHIFT;

    if (!check_pin(reply, pin)) {
        return false;
    }

    switch (act) {
        case UMDK_GPIO_GET:
            if (with_reply) {
                if (get(pin)) {
                    do_reply(reply, UMDK_GPIO_REPLY_OK_1);
                } else {
                    do_reply(reply, UMDK_GPIO_REPLY_OK_0);
                }
            }

            break;

        case UMDK_GPIO_SET_0:
        case UMDK_GPIO_SET_1:
            if (set(pin, act == UMDK_GPIO_SET_1)) {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_OK);
                }
                else
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
                }
            }
            break;

        case UMDK_GPIO_TOGGLE:
            if (toggle(pin)) {
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_OK);
                }
                else
                if (with_reply) {
                    do_reply(reply, UMDK_GPIO_REPLY_ERR_PIN);
                }
            }

            break;
    }

    return true;
}

bool umdk_gpio_broadcast(module_data_t *cmd, module_data_t *reply) {
	return gpio_cmd(cmd, reply, false); /* Don't reply on broadcasts */
}

bool umdk_gpio_cmd(module_data_t *cmd, module_data_t *reply)
{
    return gpio_cmd(cmd, reply, true);
}

#ifdef __cplusplus
}
#endif