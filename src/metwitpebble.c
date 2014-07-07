#include "pebble.h"
#include "config.h"
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

static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *temperature_layer;
static Window *window;
static Layer *layer;

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

static void render_image(int image_index, GContext* ctx) {
  GBitmap *image = gbitmap_create_with_resource(STATUS_RESOURCES[image_index]);
  GRect bounds = image->bounds;
  graphics_draw_bitmap_in_rect(ctx, image,
    (GRect) { .origin = { 0, 105 }, .size = bounds.size });
}

static void layer_update_callback(Layer *me, GContext* ctx) {
  render_image(1, ctx);
  text_layer_set_text(time_layer, "12:30");
  text_layer_set_text(date_layer, "Sun, 4");
  text_layer_set_text(temperature_layer, "23°C");
}

void deinit() {
}

void init() {

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  // Init the layer for display the image
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_set_update_proc(layer, layer_update_callback);
  layer_add_child(window_layer, layer);

  time_layer = text_layer_create(TIME_FRAME);
  text_layer_set_font(time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_49)));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_background_color(time_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  date_layer = text_layer_create(DATE_FRAME);
  text_layer_set_font(date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_23)));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_text_color(date_layer, GColorWhite);
  text_layer_set_background_color(date_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));

  temperature_layer = text_layer_create(TEMPERATURE_FRAME);
  text_layer_set_font(temperature_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MYRIAD_PRO_35)));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

/*void on_failure(int32_t cookie, int http_status, void* context) {
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
}*/
