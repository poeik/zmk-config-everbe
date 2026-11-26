/*
 * OS layer switching based on where keystrokes currently go.
 *
 * 1. The Windows marker layer is on while the selected output is BLE profile
 *    CONFIG_ZMK_OS_LAYER_WIN_PROFILE, and off otherwise (other profiles, USB).
 *    Re-evaluated on every endpoint / BLE profile change, and once after boot.
 *
 * 2. When a bonded host connects while the active profile's host is away
 *    (asleep/off), the keyboard switches the active profile to that host.
 *    To avoid races when several hosts reconnect after a wake/reboot, the switch
 *    is only done after a delay, and never during a grace period after boot.
 *    Note: ZMK stops advertising while the active host is connected, so other
 *    hosts can only connect (and trigger a switch) while the active one is away.
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/conn.h>
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

/* --- 2. switch to a host that connects while the active host is away --- */

#if IS_ENABLED(CONFIG_ZMK_OS_LAYER_AUTO_SWITCH)

#define SWITCH_DELAY_MS (CONFIG_ZMK_OS_LAYER_AUTO_SWITCH_DELAY_S * 1000)
#define BOOT_GRACE_MS (CONFIG_ZMK_OS_LAYER_AUTO_SWITCH_BOOT_GRACE_S * 1000)

static bt_addr_le_t pending_addr;

static void auto_switch_cb(struct k_work *work) {
    if (zmk_ble_active_profile_is_connected()) {
        return; /* active host is there, never steal focus from it */
    }
    for (uint8_t i = 0; i < ZMK_BLE_PROFILE_COUNT; i++) {
        if (bt_addr_le_cmp(zmk_ble_profile_address(i), &pending_addr) == 0) {
            if (i != zmk_ble_active_profile_index()) {
                LOG_INF("os_layer: host of profile %d connected, active host away, switching", i);
                zmk_ble_prof_select(i);
            }
            return;
        }
    }
}
static K_WORK_DELAYABLE_DEFINE(auto_switch_work, auto_switch_cb);

static void on_connected(struct bt_conn *conn, uint8_t err) {
    struct bt_conn_info info;
    if (err || bt_conn_get_info(conn, &info) != 0 || info.role != BT_CONN_ROLE_PERIPHERAL) {
        return; /* ignore failures and the split-half link (keyboard is central there) */
    }
    bt_addr_le_copy(&pending_addr, bt_conn_get_dst(conn));

    /* Give the active host time to come back (e.g. after a wake), and never decide
     * during the boot grace period, when all hosts reconnect in random order. */
    int64_t delay = SWITCH_DELAY_MS;
    int64_t until_grace_end = BOOT_GRACE_MS - k_uptime_get();
    if (until_grace_end > delay) {
        delay = until_grace_end;
    }
    k_work_reschedule(&auto_switch_work, K_MSEC(delay));
}

BT_CONN_CB_DEFINE(os_layer_conn_cb) = {
    .connected = on_connected,
};

#endif /* CONFIG_ZMK_OS_LAYER_AUTO_SWITCH */
