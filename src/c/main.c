#include <pebble.h>
#include "src/c/pebble2window.h"

static void init()
{
  show_pebble2window();
}

static void deinit()
{
  hide_pebble2window();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}