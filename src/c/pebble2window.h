#pragma once
#include <pebble.h>

void show_pebble2window(void);
void hide_pebble2window(void);


#define SETTINGS_KEY 9998

/*
    dict['VBC'] = configData['vib_bt_c'] ? 1 : 0;  // Send a boolean as an integer
    dict['VBD'] = configData['vib_bt_d'] ? 1 : 0;  // Send a boolean as an integer
    dict['BG_COLOR'] = backgroundColor;
    dict['DATE_FORMAT'] = configData['selDate'];
    dict['LANG_SEL'] = configData['selLang'];
*/



// A structure containing our settings
typedef struct ClaySettings {
  GColor gcBackgroundColor;
  bool bSecondTick;
  bool bVibBC;
  bool bVibBD;
  int iLang;
  int iDateFormat;
  bool bPhoneBat;
} __attribute__((__packed__)) ClaySettings;

static void prv_default_settings();
static void prv_load_settings();
static void prv_save_settings();
static void prv_update_display();
static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
static void prv_window_load(Window *window);
static void prv_window_unload(Window *window);
static void prv_init(void);
static void prv_deinit(void);

static void PhoneBatAusblenden(void);
static void PhoneBatEinblenden(void);
static void phone_battery_update_proc(Layer *layer, GContext *ctx);
static void battery_update_proc(Layer *layer, GContext *ctx);
static void battery_update_proc2(Layer *layer, GContext *ctx);