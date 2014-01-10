#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "config.h"
#include "http.h"
#include "util.h"

#define MY_UUID { 0x91, 0x41, 0xB6, 0x28, 0xBC, 0x89, 0x49, 0x8E, 0xB1, 0x47, 0x04, 0x9F, 0x49, 0xC0, 0x99, 0xAD }

#define TIME_FRAME (GRect(0, 6, 144, 168-6))
#define DATE_FRAME (GRect(0, 62, 144, 168-62))
#define ICON_FRAME (GRect(0, 100, 60, 60))
#define TEMPERATURE_FRAME (GRect(60, 105, 144, 168-62))

#define WEATHER_KEY_RESULT_CODE 0
#define WEATHER_KEY_CONDITION 1
#define WEATHER_KEY_TEMPERATURE 2

#define WEATHER_KEY_LATITUDE 0
#define WEATHER_KEY_LONGITUDE 1
#define WEATHER_KEY_UNIT_SYSTEM 2

#define WEATHER_HTTP_COOKIE 1949327671

#define REQUEST_WEATHER_URL "http://glacial-island-2381.herokuapp.com/weather"

PBL_APP_INFO(MY_UUID,
             "Metwit", "A.Stagi-P.Arminio",
             1, 0, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
TextLayer time_layer;
TextLayer date_layer;
TextLayer temperature_layer;
BmpContainer icon_weather;
Layer icon_w_layer;

uint8_t STATUS_RESOURCES[] = {
  RESOURCE_ID_ICON_UNKNOWN,
  RESOURCE_ID_ICON_SUNNY,
  RESOURCE_ID_ICON_RAINY,
  RESOURCE_ID_ICON_STORMY,
  RESOURCE_ID_ICON_SNOWY,
  RESOURCE_ID_ICON_PCLOUDY,
  RESOURCE_ID_ICON_CLOUDY,
  RESOURCE_ID_ICON_HAILING,
  RESOURCE_ID_ICON_HEAVYSEAS,
  RESOURCE_ID_ICON_CALMSEAS,
  RESOURCE_ID_ICON_FOGGY,
  RESOURCE_ID_ICON_SNOWFLURRY,
  RESOURCE_ID_ICON_WINDY,
  RESOURCE_ID_ICON_MOON,
  RESOURCE_ID_ICON_PMOONCLOUDY,
};

int my_latitude = -1, my_longitude = -1;
int8_t current_weather_status = -1;

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t);
void handle_init(AppContextRef ctx);
void on_failure(int32_t cookie, int http_status, void* context);
void on_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context);
void on_location(float latitude, float longitude, float altitude, float accuracy, void* context);
void on_reconnect(void* context);
void request_weather();
int8_t need_to_refresh_icon(uint8_t status);
void reset_location();
int8_t is_located();

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

  resource_init_current_app(&APP_RESOURCES);

  //Init time layer
  text_layer_init(&time_layer, window.layer.frame);
  text_layer_set_text(&time_layer, "00:00");
  text_layer_set_font(&time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_49)));
  text_layer_set_text_color(&time_layer, GColorWhite);
  text_layer_set_background_color(&time_layer, GColorClear);
  text_layer_set_text_alignment(&time_layer, GTextAlignmentCenter);
  layer_set_frame(&time_layer.layer, TIME_FRAME);
  layer_add_child(&window.layer, &time_layer.layer);

  //Init date layer
  text_layer_init(&date_layer, window.layer.frame);
  text_layer_set_text(&date_layer, "XXX, XXX 00");
  text_layer_set_font(&date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_23)));
  text_layer_set_text_color(&date_layer, GColorWhite);
  text_layer_set_background_color(&date_layer, GColorClear);
  text_layer_set_text_alignment(&date_layer, GTextAlignmentCenter);
  layer_set_frame(&date_layer.layer, DATE_FRAME);
  layer_add_child(&window.layer, &date_layer.layer);

  //Draw horizontal line
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(0, 100), GPoint(100, 100));

  //Init temperature layer
  text_layer_init(&temperature_layer, window.layer.frame);
  text_layer_set_text(&temperature_layer, "...");
  text_layer_set_font(&temperature_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_35)));
  text_layer_set_text_color(&temperature_layer, GColorWhite);
  text_layer_set_background_color(&temperature_layer, GColorClear);
  layer_set_frame(&temperature_layer.layer, TEMPERATURE_FRAME);
  layer_add_child(&window.layer, &temperature_layer.layer);

  //Init weather icon
  layer_add_child(&window.layer, &icon_w_layer);

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
  } else {
    http_time_request();
  }

}

void on_failure(int32_t cookie, int http_status, void* context) {
  //HANDLE FAILURE
  reset_location();
}

void on_success(int32_t cookie, int http_status, DictionaryIterator* received, void* context) {
  static char temperature_text[] = "      ";

  Tuple* rescode_tuple = dict_find(received, WEATHER_KEY_RESULT_CODE);
  if(rescode_tuple) {
    int rescode = rescode_tuple->value->int8;
    if(rescode) {
      reset_location();
      return;
    }
  }
  Tuple* weather_tuple = dict_find(received, WEATHER_KEY_CONDITION);
  if(weather_tuple) {
    //HANDLE WEATHER CONDITION RECEIVE
    uint8_t weather_status = weather_tuple->value->int8;
    if(need_to_refresh_icon(weather_status)) {
      if(current_weather_status != -1) {
        layer_remove_from_parent(&icon_weather.layer.layer);
        bmp_deinit_container(&icon_weather);
      }
      current_weather_status = weather_status;
      bmp_init_container(STATUS_RESOURCES[current_weather_status], &icon_weather);
      layer_set_frame(&icon_weather.layer.layer, ICON_FRAME);
      layer_add_child(&icon_w_layer, &icon_weather.layer.layer);
    }
  }
  Tuple* temperature_tuple = dict_find(received, WEATHER_KEY_TEMPERATURE);
  if(temperature_tuple) {
    //HANDLE TEMPERATURE RECEIVE
    int16_t temperature = temperature_tuple->value->int16;
    uint8_t sign = 0;
    if(temperature >= 0)
      sign = 1;
    memcpy(&temperature_text[sign], itoa(temperature), 4);
    uint8_t degree_pos = strlen(temperature_text);
    memcpy(&temperature_text[degree_pos], "°", 3);
    memcpy(&temperature_text[degree_pos + 2], UNIT_SYSTEM, 2);
    text_layer_set_text(&temperature_layer, temperature_text);
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
    reset_location();
    return;
  }
  // Send it.
  if(http_out_send() != HTTP_OK) {
    //HANDLE FAILURE
    reset_location();
    return;
  }
}

void reset_location() {
  my_latitude = -1;
  my_longitude = -1;
}

int8_t is_located() {
  return my_longitude != -1 ? 1 : 0;
}

int8_t need_to_refresh_icon(uint8_t status) {
  if(status == 0 && current_weather_status != -1)
    return 0;
  else if(status > 14)
    return 0;
  else if(status == current_weather_status)
    return 0;
  else
    return 1;
}
