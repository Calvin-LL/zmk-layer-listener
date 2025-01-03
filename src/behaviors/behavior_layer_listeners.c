/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_layer_listeners

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/dlist.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/keys.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct layer_listener_cfg {
    struct zmk_behavior_binding enter_binding;
    struct zmk_behavior_binding leave_binding;
    uint8_t layers[ZMK_KEYMAP_LAYERS_LEN];
    size_t layers_len;
};

#define LAYER_LISTENER_INST(n)                                                                     \
    static struct layer_listener_cfg layer_listener_cfg_##n = {                                    \
        .enter_binding = ZMK_KEYMAP_EXTRACT_BINDING(0, n),                                         \
        .leave_binding = ZMK_KEYMAP_EXTRACT_BINDING(1, n),                                         \
        .layers = DT_PROP(n, layers),                                                              \
        .layers_len = DT_PROP_LEN(n, layers),                                                      \
    };

DT_INST_FOREACH_CHILD(0, LAYER_LISTENER_INST)

#define LAYER_LISTENER_ITEM(n) &layer_listener_cfg_##n,
#define LAYER_LISTENER_UTIL_ONE(n) 1 +

static struct layer_listener_cfg *listeners[] = {DT_INST_FOREACH_CHILD(0, LAYER_LISTENER_ITEM)};

#define LISTENERS_LEN (DT_INST_FOREACH_CHILD(0, LAYER_LISTENER_UTIL_ONE) 0)

#define TAP_MS DT_INST_PROP(0, tap_ms)
#define WAIT_MS DT_INST_PROP(0, wait_ms)

static int layer_state_listener(const zmk_event_t *eh) {
    struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);

    for (int i = 0; i < LISTENERS_LEN; i++) {
        const struct layer_listener_cfg *cfg = listeners[i];

        for (int j = 0; j < cfg->layers_len; j++) {
            if (ev->layer == cfg->layers[j]) {
                LOG_DBG("invoking listener %d, layer=%d state=%d", i, ev->layer, ev->state);

                struct zmk_behavior_binding_event event = {
                    .position = INT32_MAX,
                    .timestamp = k_uptime_get(),
                };

                if (ev->state) {
                    zmk_behavior_queue_add(&event, cfg->enter_binding, true, TAP_MS);
                    zmk_behavior_queue_add(&event, cfg->enter_binding, false, WAIT_MS);
                } else {
                    zmk_behavior_queue_add(&event, cfg->leave_binding, true, TAP_MS);
                    zmk_behavior_queue_add(&event, cfg->leave_binding, false, WAIT_MS);
                }

                break;
            }
        }
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_layer_listeners, layer_state_listener);
ZMK_SUBSCRIPTION(behavior_layer_listeners, zmk_layer_state_changed);

#endif
