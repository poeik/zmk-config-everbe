# Sources are added to ZMK's `app` target, so ZMK's include dirs apply automatically.
target_sources_ifdef(CONFIG_ZMK_OS_LAYER app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/os_layer.c)
