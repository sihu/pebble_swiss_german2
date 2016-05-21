#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer, *s_datetext_layer, *s_minutetext_layer, *s_hourtext_layer, *s_cw_layer, *s_connection_layer;
static Layer *s_canvas_layer;

// performance
static int min = 0;
static int hour = 0;
static int battery = 100;

// dialect
const char *hour_string[12] = { "zwölfi", "eis","zwai", "drüü", "viari", "füüfi", "säxi",
		 "sibni", "achti", "nüni", "zehni", "elfi"};

const char *weekday_string[7] = { "So", "Mo", "Di","Mi", "Do", "Fr", "Sa" };

const char *min_string[31] = { "punkt", "eis", "zwai", "drüü", "viar", "füüf", "säx", "siba", "acht", "nüün", "zäh", "elf", "zwölf", 
    "drizehn", "viarzehn", "viartel", "sechzehn", "sibzehn", "achzehn", "nünzehn", "zwänzg", 
    "21", "22", "23", "24", "-", "-", "-", "-", "-", "halb" };

static void handle_battery(BatteryChargeState charge_state) {
  battery = charge_state.charge_percent;
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char minute_text[50];
  static char hour_text[50];
  static char date_text[7];
  static char day_text[4];


  //time_t currentmktime(struct tm * timeptr)
  min = tick_time->tm_min;
  hour = tick_time->tm_hour;  
  
  if (min != 0 && min < 24){
    snprintf(minute_text, sizeof(minute_text), "%s ab", min_string[min]);
  } else if (min > 24 && min < 30){
    snprintf(minute_text, sizeof(minute_text), "%s vor %s", min_string[30-min], min_string[30]);
  } else if (min > 30 && min < 36){
    snprintf(minute_text, sizeof(minute_text), "%s ab %s", min_string[30-60+min], min_string[30]);
  } else if(min > 35) {
    snprintf(minute_text, sizeof(minute_text), "%s vor", min_string[60-min]);
  } else {
    snprintf(minute_text, sizeof(minute_text), "%s", min_string[min]);
  }
  snprintf(hour_text, sizeof(hour_text), "%s", hour_string[min >= 25 ? (hour+1)%12 : hour%12]);
  
  strcpy(date_text , weekday_string[tick_time->tm_wday]);
  snprintf(day_text, sizeof(day_text), " %02d", tick_time->tm_mday);
  strcat(date_text , day_text);
  

  // Show
  text_layer_set_text(s_datetext_layer, date_text);
  text_layer_set_text(s_minutetext_layer, minute_text);
  text_layer_set_text(s_hourtext_layer, hour_text);

}

static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(this_layer);

  // Draw the 'stalk'
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.size.w/4*3, 0, bounds.size.w/4*battery/100, 5), 0, GCornerNone);
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00";
  static char s_cw_text[] = "00";

  //time_t currentmktime(struct tm * timeptr)
  //snprintf(s_min_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  
  strftime(s_time_text, sizeof(s_time_text), "%S", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  
  strftime(s_cw_text, sizeof(s_cw_text), "%V", tick_time);
  text_layer_set_text(s_cw_layer, s_cw_text);
  
  handle_battery(battery_state_service_peek());
  
  if(min != tick_time->tm_min){
      handle_minute_tick(tick_time, MINUTE_UNIT);
  }
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, "test");
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);


  
  // Connection layer
  s_connection_layer = text_layer_create(GRect(0, 40, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);
  //handle_bluetooth(bluetooth_connection_service_peek());

  // Minute as Word ()
  s_minutetext_layer = text_layer_create(GRect(8, 40, bounds.size.w, 34));
  text_layer_set_text_color(s_minutetext_layer, GColorWhite);
  text_layer_set_background_color(s_minutetext_layer, GColorClear);
  text_layer_set_font(s_minutetext_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  
  // Hour as Word ()
  s_hourtext_layer = text_layer_create(GRect(5, 60, bounds.size.w, 48));
  text_layer_set_text_color(s_hourtext_layer, GColorWhite);
  text_layer_set_background_color(s_hourtext_layer, GColorClear);
  text_layer_set_font(s_hourtext_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  
  // Date ()
  s_datetext_layer = text_layer_create(GRect(40, 110, bounds.size.w, 34));
  text_layer_set_text_color(s_datetext_layer, GColorWhite);
  text_layer_set_background_color(s_datetext_layer, GColorClear);
  text_layer_set_font(s_datetext_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));

  // Time digital
  s_time_layer = text_layer_create(GRect(5, 110, bounds.size.w, 34));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    
  // Battery bar
  s_canvas_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  layer_add_child(window_layer, s_canvas_layer);
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  
  // Calendar week
  s_cw_layer = text_layer_create(GRect(-20, 0, bounds.size.w, bounds.size.h));
  text_layer_set_text_color(s_cw_layer, GColorWhite);
  text_layer_set_background_color(s_cw_layer, GColorClear);
  text_layer_set_font(s_cw_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text_alignment(s_cw_layer, GTextAlignmentRight);

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  handle_minute_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
 
  battery_state_service_subscribe(handle_battery);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minutetext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hourtext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_datetext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_cw_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_minutetext_layer);
  text_layer_destroy(s_hourtext_layer);
  text_layer_destroy(s_datetext_layer);
  layer_destroy(s_canvas_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
