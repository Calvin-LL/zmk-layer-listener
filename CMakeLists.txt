if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_LAYER_LISTENERS app PRIVATE src/behaviors/behavior_layer_listeners.c)
endif()
