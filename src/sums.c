#include <pebble.h>

static TextLayer *s_hour_layer;
static TextLayer *s_minute_layer;
static TextLayer *s_total_layer;
static Layer *s_lines_layer;

static void update_time(struct tm *tick_time) {
  // Write the current hours and minutes into a buffer
  static char s_hour_buffer[8];

  strftime(s_hour_buffer, sizeof(s_hour_buffer),
           clock_is_24h_style() ? "%H" : "%I", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buffer);

  static char s_minute_buffer[8];
  snprintf(s_minute_buffer, sizeof(s_minute_buffer), "%d", tick_time->tm_min);
  text_layer_set_text(s_minute_layer, s_minute_buffer);

  static char s_total_buffer[8];
  int total = tick_time->tm_hour + tick_time->tm_min;

  snprintf(s_total_buffer, sizeof(s_total_buffer), "%d", total);
  text_layer_set_text(s_total_layer, s_total_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time(tick_time);
}

static TextLayer *configured_text_layer(GRect bounds) {
  TextLayer *layer = text_layer_create(bounds);

  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_text_color(layer, GColorBlack);
  text_layer_set_font(layer,
                      fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(layer, GTextAlignmentRight);

  return layer;
}

static void lines_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, GColorBlack);

  // Lines.
  graphics_fill_rect(ctx, GRect(10, 95, bounds.size.w - 20, 6), 3, GCornersAll);
  graphics_fill_rect(ctx, GRect(10, 148, bounds.size.w - 20, 6), 3,
                     GCornersAll);

  // Plus.
  graphics_fill_rect(ctx, GRect(20, 70, 20, 4), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(28, 62, 4, 20), 0, GCornerNone);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_hour_layer = configured_text_layer(GRect(0, 0, bounds.size.w - 30, 50));
  s_minute_layer = configured_text_layer(GRect(0, 40, bounds.size.w - 30, 50));
  s_total_layer = configured_text_layer(GRect(0, 93, bounds.size.w - 30, 50));

  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minute_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_total_layer));

  s_lines_layer = layer_create(bounds);
  layer_set_update_proc(s_lines_layer, lines_update_proc);
  layer_add_child(window_layer, s_lines_layer);
  layer_mark_dirty(s_lines_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_minute_layer);
  text_layer_destroy(s_total_layer);
  layer_destroy(s_lines_layer);
  window_set_user_data(window, NULL);
}

static Window *init(void) {
  Window *window = window_create();

  window_set_window_handlers(window,
                             (WindowHandlers){
                                 .load = window_load, .unload = window_unload,
                             });

  window_stack_push(window, true);

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  update_time(tick_time);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  return window;
}

static void deinit(Window *window) { window_destroy(window); }

int main(void) {
  Window *window = init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();

  deinit(window);
}
