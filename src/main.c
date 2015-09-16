#include <pebble.h>
  
static Window *s_main_window;
static TextLayer *s_time_layer, *s_battery_layer, *s_datetext_layer, *s_minutetext_layer, *s_hourtext_layer, *s_connection_layer;

// performance
static int min = 0;
static int hour = 0;

// dialect
const char *hour_string[25] = { "zwölfi", "eis","zwei", "drei", "viäri", "füüfi", "säxi",
		 "sibni", "achti", "nüni", "zehni", "ölfi"};

const char *weekday_string[7] = { "So", "Mo", "Di","Mi", "Do", "Fr", "Sa" };

const char *min_string[30] = { "\neis ", "\nzwei ", "\ndrii ", "\nviar ", "\nfüüf ", "\nsäx ", "\nsiba ", "\nacht ", "\nnüün ", "\nzäh ", "\nelf ", "\nzwölf ", 
    "\ndrizehn ", "\nviarzehn ", "\nviartel ", "\nsechzehn", "\nsibzehn ", "\nachzehn ", "\nnünzehn ", "\nzwänzg ", 
    "ainazwenzg\n", "zwaiazwenzg\n", "driazwenzg\n", "viarazwenzg\n", "füfazwenzg\n", "sexazwenzg\n", 
    "sibanazwenzg\n", "achtazwenzg", "nünazwenzg\n", "\nhalb" };


static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "+");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char minute_text[50];
  static char hour_text[50];
  static char date_text[7];
  static char day_text[4];


  //time_t currentmktime(struct tm * timeptr)
  min = tick_time->tm_min;
  hour = tick_time->tm_hour > 11 ? tick_time->tm_hour-12 : tick_time->tm_hour;  
  
  if(min > 30) {
    strcpy(minute_text , min_string[60-min-1]);
    strcat(minute_text , "vor");
  } else if (min < 30){
    strcpy(minute_text , min_string[min-1]);
    strcat(minute_text , "ab");
  } else {
    strcpy(minute_text , min_string[min-1]);
  }
  strcpy(hour_text , hour_string[min >= 30 ? hour+1 : hour]);
  
  strcpy(date_text , weekday_string[tick_time->tm_wday]);
  snprintf(day_text, sizeof(day_text), " %02d", tick_time->tm_mday);
  strcat(date_text , day_text);
  

  // Show
  text_layer_set_text(s_datetext_layer, date_text);
  text_layer_set_text(s_minutetext_layer, minute_text);
  text_layer_set_text(s_hourtext_layer, hour_text);

}


static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00:00";
  //time_t currentmktime(struct tm * timeptr)
  //snprintf(s_min_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  
  strftime(s_time_text, sizeof(s_time_text), "%T", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  
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
  s_minutetext_layer = text_layer_create(GRect(8, 12, bounds.size.w, 68));
  text_layer_set_text_color(s_minutetext_layer, GColorWhite);
  text_layer_set_background_color(s_minutetext_layer, GColorClear);
  text_layer_set_font(s_minutetext_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  
  // Hour as Word ()
  s_hourtext_layer = text_layer_create(GRect(5, 60, bounds.size.w, 48));
  text_layer_set_text_color(s_hourtext_layer, GColorWhite);
  text_layer_set_background_color(s_hourtext_layer, GColorClear);
  text_layer_set_font(s_hourtext_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  
  // Date ()
  s_datetext_layer = text_layer_create(GRect(0, 110, bounds.size.w, 34));
  text_layer_set_text_color(s_datetext_layer, GColorWhite);
  text_layer_set_background_color(s_datetext_layer, GColorClear);
  text_layer_set_font(s_datetext_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_datetext_layer, GTextAlignmentRight);

  
  // Time digital
  s_time_layer = text_layer_create(GRect(0, 140, bounds.size.w, 34));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  
  // Battery-Status (top, right)
  s_battery_layer = text_layer_create(GRect(0, 0, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100%");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  handle_minute_tick(current_time, MINUTE_UNIT);


  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);


  battery_state_service_subscribe(handle_battery);
  //bluetooth_connection_service_subscribe(handle_bluetooth);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_minutetext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hourtext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_datetext_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
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
