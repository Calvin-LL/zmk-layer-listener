if ((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  target_sources_ifdef(CONFIG_ZMK_LAYER_LISTENERS app PRIVATE src/layer_listeners.c)
  target_sources_ifdef(CONFIG_ZMK_KEYCODE_LISTENERS app PRIVATE src/keycode_listeners.c)
endif()
