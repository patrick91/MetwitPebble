#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "config.h"
#include "http.h"

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }

#define TIME_FRAME (GRect(0, 6, 144, 168-6))
#define DATE_FRAME (GRect(0, 62, 144, 168-62))

#define WEATHER_KEY_CONDITION 0
#define WEATHER_KEY_TEMPERATURE 1

#define WEATHER_KEY_LATITUDE 0
#define WEATHER_KEY_LONGITUDE 1
#define WEATHER_KEY_UNIT_SYSTEM 2

#define WEATHER_HTTP_COOKIE 1949327671

#define REQUEST_WEATHER_URL "http://192.168.0.6:5000/weather"

PBL_APP_INFO(MY_UUID,
             "Metwit", "A.Stagi-P.Arminio",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer time_layer;
TextLayer date_layer;

int my_latitude = -1, my_longitude = -1;

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t);
void handle_init(AppContextRef ctx);
void on_failure(int32_t cookie, int http_status, void* context);
void on_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context);
void on_location(float latitude, float longitude, float altitude, float accuracy, void* context);
void on_reconnect(void* context);
void request_weather();

void reset_location();
int is_located();

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT
    },
    .messaging_info = {
      .buffer_sizes = {
        .inbound = 124,
        .outbound = 124,
      }
    }
  };
  app_event_loop(params, &handlers);
}

void handle_init(AppContextRef ctx) {

  window_init(&window, "Metwit");
  window_stack_push(&window, true);
  window_set_background_color(&window, GColorBlack);

  //Init time layer
  text_layer_init(&time_layer, window.layer.frame);
  text_layer_set_text(&time_layer, "00:00");
  text_layer_set_font(&time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_color(&time_layer, GColorWhite);
  text_layer_set_background_color(&time_layer, GColorClear);
  text_layer_set_text_alignment(&time_layer, GTextAlignmentCenter);
  layer_set_frame(&time_layer.layer, TIME_FRAME);
  layer_add_child(&window.layer, &time_layer.layer);

  //Init date layer
  text_layer_init(&date_layer, window.layer.frame);
  text_layer_set_text(&date_layer, "XXX, XXX 00");
  text_layer_set_font(&date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_color(&date_layer, GColorWhite);
  text_layer_set_background_color(&date_layer, GColorClear);
  text_layer_set_text_alignment(&date_layer, GTextAlignmentCenter);
  layer_set_frame(&date_layer.layer, DATE_FRAME);
  layer_add_child(&window.layer, &date_layer.layer);

  http_register_callbacks((HTTPCallbacks){.failure=on_failure,.success=on_success,.reconnect=on_reconnect,.location=on_location}, (void*)ctx);

  PblTm tm;
  PebbleTickEvent t;

  get_time(&tm);
  t.tick_time = &tm;
  t.units_changed = SECOND_UNIT | MINUTE_UNIT | HOUR_UNIT | DAY_UNIT;
    
  handle_minute_tick(ctx, &t);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {

  static char date_text[] = "XXX, XXX 00";
  static char hour_text[] = "00:00";

  if (t->units_changed & DAY_UNIT) {
    string_format_time(date_text, sizeof(date_text),
                       "%a, %b %d",
                       t->tick_time);
    text_layer_set_text(&date_layer, date_text);
  }

  if (clock_is_24h_style()) {
    string_format_time(hour_text, sizeof(hour_text), "%H:%M", t->tick_time);
  } else {
    string_format_time(hour_text, sizeof(hour_text), "%I:%M", t->tick_time);
  }

  text_layer_set_text(&time_layer, hour_text);

  if(!is_located() || (t->tick_time->tm_min % 30) == 0) {
    http_location_request();
  }

}

void on_failure(int32_t cookie, int http_status, void* context) {
  //HANDLE FAILURE
  reset_location();
}

void on_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
  Tuple* weather_tuple = dict_find(received, WEATHER_KEY_CONDITION);
  if(weather_tuple) {
    //HANDLE WEATHER CONDITION RECEIVE
  }
  Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE);
  if(temperature_tuple) {
    //HANDLE TEMPERATURE RECEIVE
  }
  
}

void on_location(float latitude, float longitude, float altitude, float accuracy, void* context) {
  //HANDLE LOCATION
  my_latitude = latitude * 10000;
  my_longitude = longitude * 10000;
  request_weather();
}

void on_reconnect(void* context) {
  reset_location();
  request_weather();
}

void request_weather() {
  if(!is_located()) {
    http_location_request();
    return;
  }
  // Build the HTTP request
  DictionaryIterator *body;
  HTTPResult result = http_out_get(REQUEST_WEATHER_URL, WEATHER_HTTP_COOKIE, &body);
  dict_write_int32(body, WEATHER_KEY_LATITUDE, my_latitude);
  dict_write_int32(body, WEATHER_KEY_LONGITUDE, my_longitude);
  dict_write_cstring(body, WEATHER_KEY_UNIT_SYSTEM, UNIT_SYSTEM);
  if(result != HTTP_OK) {
    //HANDLE FAILURE
    return;
  }
  // Send it.
  if(http_out_send() != HTTP_OK) {
    //HANDLE FAILURE
    return;
  }
}

void reset_location() {
  my_latitude = -1;
  my_longitude = -1;
}

int is_located() {
  return my_longitude != -1 ? 1 : 0;
}
