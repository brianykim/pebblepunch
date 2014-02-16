#include <pebble.h>

//Track acceleration data
void accel_data_handler(AccelData*, uint32_t);

static Window *window;
static TextLayer *text_layer;

typedef enum {PUNCH, BLOCK, NEUTRAL} State;
State state = NEUTRAL;
uint8_t blockcount = 0;
bool blockreset = false;


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Up");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Down");
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press a button");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

//HANDLE ACCELERATION DATA HERE
void accel_data_handler(AccelData *data, uint32_t num_samples)
{
  for(uint32_t i = 0; i < num_samples; ++i)
  {
    if(data[i].x > 1200)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "%i %i %i %s", data[i].x, data[i].y, data[i].z, "punch");
      blockreset = true;
      return;
    }
    else if(blockreset && data[i].x < -800)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "%i %i %i %s", data[i].x, data[i].y, data[i].z, "block");
      ++blockcount;
      if(blockcount > 200)
      {
        blockreset = false;
        return;
      }
      return;
    }
    else if(!blockreset && data[i].x > -600)
    {
      blockreset = true;
      return;
    }
  }
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);

  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  accel_data_service_subscribe(10, &accel_data_handler);
}

static void deinit(void) {
  window_destroy(window);
  accel_data_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
