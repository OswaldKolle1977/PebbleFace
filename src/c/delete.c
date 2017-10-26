#include <pebble.h>
#include "delete.h"

// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY
static Window *s_window;
static GFont s_res_roboto_condensed_21;
static TextLayer *ltHerzfrequenz;

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // ltHerzfrequenz
  ltHerzfrequenz = text_layer_create(GRect(90, 146, 34, 22));
  text_layer_set_background_color(ltHerzfrequenz, GColorClear);
  text_layer_set_text_color(ltHerzfrequenz, GColorWhite);
  text_layer_set_text(ltHerzfrequenz, "222");
  text_layer_set_text_alignment(ltHerzfrequenz, GTextAlignmentRight);
  text_layer_set_font(ltHerzfrequenz, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)ltHerzfrequenz);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  text_layer_destroy(ltHerzfrequenz);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

void show_delete(void) {
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  window_stack_push(s_window, true);
}

void hide_delete(void) {
  window_stack_remove(s_window, true);
}
