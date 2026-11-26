/*
 * OS layer switching based on where keystrokes currently go.
 *
 * 1. The Windows marker layer is on while the selected output is BLE profile
 *    CONFIG_ZMK_OS_LAYER_WIN_PROFILE, and off otherwise (other profiles, USB).
 *    Re-evaluated on every endpoint / BLE profile change, and once after boot.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define WIN_PROFILE CONFIG_ZMK_OS_LAYER_WIN_PROFILE
#define WIN_LAYER CONFIG_ZMK_OS_LAYER_WIN_LAYER

/* --- 1. layer follows the selected output --- */

static bool windows_selected(void) {
    struct zmk_endpoint_instance ep = zmk_endpoint_get_selected();
    switch (ep.transport) {
    case ZMK_TRANSPORT_BLE:
        return ep.ble.profile_index == WIN_PROFILE;
    case ZMK_TRANSPORT_USB:
        return false;
    default: /* nothing connected yet: go by the selected BLE profile */
        return zmk_ble_active_profile_index() == WIN_PROFILE;
    }
}

static void apply_os_layer(void) {
    bool want = windows_selected();
    if (want == zmk_keymap_layer_active(WIN_LAYER)) {
        return;
    }
    LOG_INF("os_layer: Windows layer %s", want ? "on" : "off");
    if (want) {
        zmk_keymap_layer_activate(WIN_LAYER, true);
    } else {
        zmk_keymap_layer_deactivate(WIN_LAYER, true);
    }
}

static int os_layer_listener(const zmk_event_t *eh) {
    apply_os_layer();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(os_layer, os_layer_listener);
ZMK_SUBSCRIPTION(os_layer, zmk_ble_active_profile_changed);
ZMK_SUBSCRIPTION(os_layer, zmk_endpoint_changed);

static void boot_apply_cb(struct k_work *work) { apply_os_layer(); }
static K_WORK_DELAYABLE_DEFINE(boot_apply_work, boot_apply_cb);

static int os_layer_init(void) {
    k_work_schedule(&boot_apply_work, K_SECONDS(2));
    return 0;
}
SYS_INIT(os_layer_init, APPLICATION, 99);
