/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_keycode_listeners

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/dlist.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keys.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct keycode_listener_cfg {
    size_t bindings_len;
    struct zmk_behavior_binding *bindings;
    size_t layers_len;
    int8_t layers[ZMK_KEYMAP_LAYERS_LEN];
    size_t keycodes_len;
    uint32_t keycodes[];
};

#define TRANSFORMED_BINDINGS(n)                                                                    \
    { LISTIFY(DT_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), n) }

#define KEYCODE_LISTENER_INST(n)                                                                   \
    static struct zmk_behavior_binding                                                             \
        keycode_listener_config_##n##_bindings[DT_PROP_LEN(n, bindings)] =                         \
            TRANSFORMED_BINDINGS(n);                                                               \
                                                                                                   \
    static struct keycode_listener_cfg keycode_listener_cfg_##n = {                                \
        .bindings_len = DT_PROP_LEN(n, bindings),                                                  \
        .bindings = keycode_listener_config_##n##_bindings,                                        \
        .keycodes = DT_PROP(n, keycodes),                                                          \
        .keycodes_len = DT_PROP_LEN(n, keycodes),                                                  \
        .layers = DT_PROP(n, layers),                                                              \
        .layers_len = DT_PROP_LEN(n, layers),                                                      \
    };

DT_INST_FOREACH_CHILD(0, KEYCODE_LISTENER_INST)

#define KEYCODE_LISTENER_ITEM(n) &keycode_listener_cfg_##n,
#define KEYCODE_LISTENER_UTIL_ONE(n) 1 +

static struct keycode_listener_cfg *listeners[] = {DT_INST_FOREACH_CHILD(0, KEYCODE_LISTENER_ITEM)};

#define LISTENERS_LEN (DT_INST_FOREACH_CHILD(0, KEYCODE_LISTENER_UTIL_ONE) 0)

#define TAP_MS DT_INST_PROP(0, tap_ms)
#define WAIT_MS DT_INST_PROP(0, wait_ms)

static bool key_is_equal(struct zmk_keycode_state_changed *ev, uint32_t keycode) {
    return ZMK_HID_USAGE_PAGE(keycode) == ev->usage_page &&
           ZMK_HID_USAGE_ID(keycode) == ev->keycode &&
           SELECT_MODS(keycode) == ev->implicit_modifiers;
}

static bool listener_active_on_layer(struct keycode_listener_cfg *listener, uint8_t layer) {
    if (listener->layers[0] == -1) {
        return true;
    }
    for (int i = 0; i < listener->layers_len; i++) {
        if (listener->layers[i] == layer) {
            return true;
        }
    }
    return false;
}

static int keycode_state_listener(const zmk_event_t *eh) {
    struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    uint8_t highest_active_layer = zmk_keymap_highest_layer_active();

    for (int i = 0; i < LISTENERS_LEN; i++) {
        struct keycode_listener_cfg *cfg = listeners[i];

        if (!listener_active_on_layer(cfg, highest_active_layer)) {
            continue;
        }

        for (int j = 0; j < cfg->keycodes_len; j++) {

            if (key_is_equal(ev, cfg->keycodes[j])) {
                LOG_DBG("invoking listener %d, keycode=%d state=%d", i, ev->keycode, ev->state);

                struct zmk_behavior_binding_event event = {
                    .position = INT32_MAX,
                    .timestamp = k_uptime_get(),
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
                    .source = ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL,
#endif
                };

                if (ev->state) {
                    zmk_behavior_queue_add(&event, cfg->bindings[0], true, TAP_MS);
                    zmk_behavior_queue_add(&event, cfg->bindings[0], false, WAIT_MS);
                } else if (cfg->bindings_len > 1) {
                    zmk_behavior_queue_add(&event, cfg->bindings[1], true, TAP_MS);
                    zmk_behavior_queue_add(&event, cfg->bindings[1], false, WAIT_MS);
                }

                break;
            }
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(keycode_listeners, keycode_state_listener);
ZMK_SUBSCRIPTION(keycode_listeners, zmk_keycode_state_changed);
