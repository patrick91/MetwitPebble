#include "pebble.h"

#define TIME_FRAME (GRect(0, 6, 144, 168-6))
#define DATE_FRAME (GRect(0, 62, 144, 168-62))
#define ICON_FRAME (GRect(0, 100, 60, 60))
#define TEMPERATURE_FRAME (GRect(60, 105, 144, 168-62))

static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *temperature_layer;
static Window *window;
static Layer *layer;
static GBitmap *icon_bitmap = NULL;
static BitmapLayer *icon_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

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

enum WeatherKey {
  WEATHER_ICON_KEY,
  WEATHER_TEMPERATURE_KEY,
};

static void request_weather(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);

  app_message_outbox_send();
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00";
  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(time_layer, time_text);

  static char date_text[] = "XXX, XXX 00";
  strftime(date_text, sizeof(date_text), "%a, %b %d", tick_time);
  text_layer_set_text(date_layer, date_text);

  if (tick_time->tm_min % 15 == 0)
    request_weather();
}

static void render_image(uint8_t image_index, GContext* ctx) {
  if (icon_bitmap) {
    gbitmap_destroy(icon_bitmap);
  }
  icon_bitmap = gbitmap_create_with_resource(STATUS_RESOURCES[image_index]);
  GRect bounds = icon_bitmap->bounds;
  bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
}

static void in_received_handler(DictionaryIterator *iter, void *ctx) {

   APP_LOG(APP_LOG_LEVEL_INFO, "Time flies!");

  Tuple *icon_tuple = dict_find(iter, WEATHER_ICON_KEY);

  if (icon_tuple) {
    render_image(icon_tuple->value->uint8, ctx);
  }

  Tuple *temp_tuple = dict_find(iter, WEATHER_TEMPERATURE_KEY);

  if (temp_tuple) {
    text_layer_set_text(temperature_layer, temp_tuple->value->cstring);
  }

}

void deinit() {
}

void init() {

  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  app_message_register_inbox_received(in_received_handler);

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  layer = layer_create(bounds);
  layer_add_child(window_layer, layer);

  icon_layer = bitmap_layer_create(ICON_FRAME);
  layer_add_child(window_layer, bitmap_layer_get_layer(icon_layer));

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
  text_layer_set_text(temperature_layer, "--");
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_minute_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
