#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x34, 0xDC, 0xC7, 0xE2, 0x31, 0x20, 0x42, 0x2A, 0x84, 0x4F, 0x73, 0x51, 0x7D, 0xA4, 0x6E, 0x0E }
PBL_APP_INFO(MY_UUID,
             "Template App", "Your Company",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window window;


void handle_init(AppContextRef ctx) {

  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init
  };
  app_event_loop(params, &handlers);
}
