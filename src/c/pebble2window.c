#include <pebble.h>
#include "pebble2window.h"

// NEU 2017-10-18
// A struct for our specific settings (see main.h)
ClaySettings settings;
// ENDE 2017-10-28
// BEGIN AUTO-GENERATED UI CODE; DO NOT MODIFY

// #define KEY_COLOR_RED     3
// #define KEY_COLOR_GREEN   4
// #define KEY_COLOR_BLUE    5
// #define KEY_HIGH_CONTRAST 6

static bool bBatShown;

static char s_bufferPebbleLevel[4];
static int s_step_average2 = 0;
static int s_step_average3 = 0;
static Window *s_window;
static GBitmap *s_res_image_time_bg;
static char s_bufferLevel[4];

static GBitmap *s_res_image_dp;

static GFont s_res_roboto_bold_subset_49;
// static BitmapLayer *lb_DP;
// static BitmapLayer *lb_time_r;

static TextLayer *lt_Time_L;
static TextLayer *lt_Time_R;
static TextLayer *lt_PebbleBat_P;
static TextLayer *lt_PhoneBat_P;
static Layer *l_battery;
static Layer *l_phone_battery;
static Layer *s_canvas_layer;

static int s_battery_level;
static int s_phone_battery_level;
static int siPhoneBatterySent = 0;

static int siAverage;

static TextLayer *s_step_layer;
static TextLayer *ltDatum;
static TextLayer *ltWochentag;
static char sc_Datum[24];
static char sc_Wochentag[24];
static Layer *s_progress_layer;
static char s_current_steps_buffer[16];
static int s_step_count = 0, s_step_goal = 0, s_step_average = 0;


static int si_width;
static GFont sf_loco;
static char MONATE[2][12][10];
static char WOCHENTAG[2][7][12];

// static char* MONATE[] = {{"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"}, {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"}};
// static char* WOCHENTAG[] = {"SONNTAG", "MONTAG", "DIENSTAG", "MITTWOCH", "DONNERSTAG", "FREITAG", "SAMSTAG"};
static bool sbDebug = false;
uint32_t PhoneBatteryKey = 2;
static GBitmap *s_res_image_herz;// Is step data available?
bool step_data_is_available() {
  return HealthServiceAccessibilityMaskAvailable &
    health_service_metric_accessible(HealthMetricStepCount,
      time_start_of_today(), time(NULL));
}

// Datum + Wochentag
static void get_step_average();
static void update_date()
{
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  // strftime(s_buffer, sizeof(s_buffer), "%D.:%M", tick_time);
  //snprintf(sc_Datum, sizeof(sc_Datum), "%i. %s %i", tick_time->tm_mday, MONATE[tick_time->tm_mon], tick_time->tm_year+1900);

  // 1 = DD. MMM
  // 2 = DD. MMM YYYY
  // 3 = DD. MMM YY
  // 4 = MMM, DD
  // 5 = MMM, DD YYYY
  // 6 = MMM, DD YY
  switch (settings.iDateFormat)
  {
    case 1:
      snprintf(sc_Datum, sizeof(sc_Datum), "%i. %s", tick_time->tm_mday, MONATE[settings.iLang][tick_time->tm_mon]);
      break;
    case 2:
      snprintf(sc_Datum, sizeof(sc_Datum), "%i. %s %i", tick_time->tm_mday, MONATE[settings.iLang][tick_time->tm_mon], tick_time->tm_year+1900);
      break;
    case 3:
      snprintf(sc_Datum, sizeof(sc_Datum), "%i. %s %i", tick_time->tm_mday, MONATE[settings.iLang][tick_time->tm_mon], tick_time->tm_year-100);
      break;
    case 4:
      snprintf(sc_Datum, sizeof(sc_Datum), "%s %i", MONATE[settings.iLang][tick_time->tm_mon], tick_time->tm_mday);
      break;
    case 5:
      snprintf(sc_Datum, sizeof(sc_Datum), "%s %i, %i", MONATE[settings.iLang][tick_time->tm_mon], tick_time->tm_mday, tick_time->tm_year+1900);
      break;
    case 6:
      snprintf(sc_Datum, sizeof(sc_Datum), "%s %i, %i", MONATE[settings.iLang][tick_time->tm_mon], tick_time->tm_mday, tick_time->tm_year-100);
      break;
  }
  // Display this time on the TextLayer
  text_layer_set_text(ltDatum, sc_Datum);
  
  snprintf(sc_Wochentag, sizeof(sc_Wochentag), "%s", WOCHENTAG[settings.iLang][tick_time->tm_wday]);
  text_layer_set_text(ltWochentag, sc_Wochentag);
}

// Daily step goal
static void get_step_goal() {
  const time_t start = time_start_of_today();
  const time_t end = start + SECONDS_PER_DAY;
  s_step_goal = (int)health_service_sum_averaged(HealthMetricStepCount,
    start, end, HealthServiceTimeScopeDaily);
}

// Todays current step count
static void get_step_count() {
  s_step_count = (int)health_service_sum_today(HealthMetricStepCount);
}

// Average daily step count for this time of day
static void get_step_average() {
  const time_t start = time_start_of_today();
  const time_t end = time(NULL);
  s_step_average2  = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDaily);
  s_step_average = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeWeekly);
  s_step_average3 = (int)health_service_sum_averaged(HealthMetricStepCount, start, end, HealthServiceTimeScopeDailyWeekdayOrWeekend);
}

static void display_step_count() {
  int thousands = s_step_count / 1000;
  int hundreds = s_step_count % 1000;
  static char s_emoji[5];

  if(s_step_count >= s_step_average) {
    text_layer_set_text_color(s_step_layer, GColorJaegerGreen);
    snprintf(s_emoji, sizeof(s_emoji), "\U0001F60C");
  } else {
    text_layer_set_text_color(s_step_layer, GColorPictonBlue);
    //snprintf(s_emoji, sizeof(s_emoji), "\U0001F4A9");
    snprintf(s_emoji, sizeof(s_emoji), " ");
  }


  if(thousands > 0) {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d.%03d %s", thousands, hundreds, s_emoji);
  } else {
    snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer),
      "%d %s", hundreds, s_emoji);
  }
/*  
////////////  
   if (s_step_count > 100000)
   {
      int Step_Count_K = s_step_count / 1000;
      snprintf(s_current_steps_buffer,sizeof(s_current_steps_buffer),"%dk %s",Step_Count_K, s_emoji);
   }
   else if (s_step_count > 10000)
   {
      int Step_Count_K = s_step_count / 1000;
      int Step_Count_H = (s_step_count - (Step_Count_K * 1000)) / 10;
      snprintf(s_current_steps_buffer,sizeof(s_current_steps_buffer),"%d.%02dk %s",Step_Count_K,Step_Count_H, s_emoji);
   }
   else if (s_step_count > 1000)
   {
      int Step_Count_K = s_step_count / 1000;
      int Step_Count_H = (s_step_count - (Step_Count_K * 1000) ) / 10;
      
      snprintf(s_current_steps_buffer,sizeof(s_current_steps_buffer),"%d.%02dk %s",Step_Count_K,Step_Count_H, s_emoji);
   }
   else
   {
      snprintf(s_current_steps_buffer,sizeof(s_current_steps_buffer),"%d %s",s_step_count, s_emoji);
   }
  */
  
////////////
  /*
  if (s_step_count > s_step_goal)
    siAverage = (int)(144 * ((double) s_step_average / (double)(s_step_count)));
  else
    siAverage = (int)(144 * ((double) s_step_average / (double)(s_step_goal)));
  snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%i - %i",s_step_count, s_step_average);
  */
  text_layer_set_text(s_step_layer, s_current_steps_buffer);
}

static void progress_layer_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  int iGoal = 0;
  
  // s_step_count = s_step_goal * 3;
  // DEBUG
  // snprintf(s_current_steps_buffer, sizeof(s_current_steps_buffer), "%i - %i",s_step_count, s_step_goal);
  // text_layer_set_text(s_step_layer, s_current_steps_buffer);
  // printf("progress_layer_update_proc...");
  // DEBUG ENDE
  if (s_step_count <= s_step_goal)
  {
    si_width = (int)(144 * ((double)s_step_count / (double)s_step_goal));
  }
  else
  {
    iGoal = (int)(144 * ((double)s_step_goal / (double)s_step_count));
    si_width = 144;
  }
  // Draw the background
  //graphics_context_set_fill_color(ctx, GColorLightGray);
  // graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0,4,144,bounds.size.h-3), 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 2, si_width, bounds.size.h - 2), 0, GCornerNone);

  if (iGoal > 0)
  {
    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(iGoal-1, 0, 2, bounds.size.h), 0, GCornerNone);
  }
  // Average
  if (s_step_count > s_step_goal)
    siAverage = (int)(144 * ((double) s_step_average / (double)(s_step_count)));
  else
    siAverage = (int)(144 * ((double) s_step_average / (double)(s_step_goal)));

  // siAverage = (int)(144 * ((double) s_step_average / (double)((s_step_count + s_step_goal) / 2)));
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(siAverage-1, 0, 2, bounds.size.h), 0, GCornerNone);
/*
  printf("Schritte: %i / %i; Width = %i", s_step_count, s_step_goal, si_width);
  printf("Steps   : %i", s_step_count);
  printf("Goal    : %i", s_step_goal);
  printf("SPoint  : %i", si_width);
  printf("GPoint  : %i", iGoal);
*/
  
  
  
  /*
  graphics_context_set_fill_color(ctx, s_step_count >= s_step_average ? GColorJaegerGreen : GColorPictonBlue);

  
  graphics_fill_radial(ctx, inset, GOvalScaleModeFitCircle, 12,
    DEG_TO_TRIGANGLE(0),
    DEG_TO_TRIGANGLE(360 * (s_step_count / s_step_goal)));
  */
}



static BitmapLayer *lbHerz;
static GBitmap *s_res_image_schritte;
static BitmapLayer *lbSchritte;
static GFont s_res_roboto_condensed_21;
static TextLayer *ltHerzfrequenz;
static TextLayer *ltSchritte;


//void Battery_Handle_Phone(int i_BatteryLevel,bool b_ChargingState);
static void update_battery();



static void prv_on_health_data(HealthEventType type, void *context) {
  // If the update was from the Heart Rate Monitor, query it
  if (sbDebug)
    printf("prv_on_health_data() Type=%i", type);
  if (type == HealthEventHeartRateUpdate) {
    // HealthValue Heartrate = health_service_peek_current_value(HealthMetricHeartRateRawBPM);
    HealthValue Heartrate = health_service_peek_current_value(HealthMetricHeartRateBPM);
    // Display the heart rate
    if (sbDebug)
      printf("Herzfrequenz gemessen: %i", (int)Heartrate);
    // Buffer to store the heart information
    static char s_HeartText[30];
    snprintf(s_HeartText,sizeof(s_HeartText),"%i", (int)Heartrate);

    // And display the data
    text_layer_set_text(ltHerzfrequenz, s_HeartText);
  }
  if(type == HealthEventSignificantUpdate) {
    get_step_goal();
  }
  if(type != HealthEventSleepUpdate) {
    get_step_count();
    get_step_average();
    display_step_count();
    layer_mark_dirty(s_progress_layer);
  }}

static void bluetooth_callback(bool connected) {
  if (!connected)
  {
    s_phone_battery_level = 0;
    persist_write_int(PhoneBatteryKey, s_phone_battery_level);
    text_layer_set_text(lt_PhoneBat_P, "--");
    if (settings.bVibBD)
      vibes_double_pulse();
  } else
  {
    if (settings.bVibBC)
      vibes_double_pulse();
  }
  layer_mark_dirty(l_phone_battery);
  // layer_mark_dirty((Layer *)lt_PhoneBat_P); // TEST
  if (sbDebug)
    printf("Bluetooth Verbindung: %s", connected ? "true" : "false");
}
/*
static void kit_connection_handler(bool connected) {
  if (!connected)
  {
    s_phone_battery_level = 0;
    persist_write_int(PhoneBatteryKey, s_phone_battery_level);
    text_layer_set_text(lt_PhoneBat_P, "0");
    vibes_double_pulse();
  }
  layer_mark_dirty(l_phone_battery);
  // layer_mark_dirty((Layer *)lt_PhoneBat_P); // TEST
  APP_LOG(APP_LOG_LEVEL_INFO, "PebbleKit %sconnected", connected ? "" : "dis"); 
}
*/

static void PhoneBatAusblenden()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBatAusblenden() BEGINN");
  
  if (bBatShown)
  {
    // Alles ist eingeblendet, also ausblenden...
    /*
    layer_set_hidden(text_layer_get_layer(lt_PebbleBat_P), true);
    layer_set_hidden(text_layer_get_layer(lt_PhoneBat_P), true);
    layer_set_hidden(l_battery, true);
    layer_set_hidden(l_phone_battery, true);
    */
    text_layer_destroy(lt_PebbleBat_P);
    text_layer_destroy(lt_PhoneBat_P);
    layer_destroy(l_battery);
    layer_destroy(l_phone_battery);
    
    // Create battery meter Layer
    l_battery = layer_create(GRect(0, 0, 144, 6));
    layer_set_update_proc(l_battery, battery_update_proc2);
    
    // Pebble und Phone Battery Text
    lt_PebbleBat_P = text_layer_create(GRect(0, 3, 35, 14));
    text_layer_set_background_color(lt_PebbleBat_P, GColorClear);
    // text_layer_set_text(lt_PebbleBat_P, "--");
    text_layer_set_text_color(lt_PebbleBat_P, GColorWhite);
    text_layer_set_text_alignment(lt_PebbleBat_P, GTextAlignmentLeft);
    text_layer_set_font(lt_PebbleBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    
    layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PebbleBat_P);
    
    // Add to Window
    layer_add_child(window_get_root_layer(s_window), l_battery);
    
    
    snprintf(s_bufferPebbleLevel, sizeof(s_bufferPebbleLevel), "%d", s_battery_level);
    text_layer_set_text(lt_PebbleBat_P, s_bufferPebbleLevel);
    
    update_battery();
    
    bBatShown = false;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBatAusblenden() ENDE");
}
static void PhoneBatEinblenden()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBatEinblenden() BEGINN");
  if (bBatShown == false)
  {
    // Alles war ausgeblendet - also einblenden...
    /*
    layer_set_hidden(text_layer_get_layer(lt_PebbleBat_P), false);
    layer_set_hidden(text_layer_get_layer(lt_PhoneBat_P), false);
    layer_set_hidden(l_battery, false);
    layer_set_hidden(l_phone_battery, false);
    */
    text_layer_destroy(lt_PebbleBat_P);
    layer_destroy(l_battery);
    // Create battery meter Layer
    l_battery = layer_create(GRect(0, 0, 70, 6));
    layer_set_update_proc(l_battery, battery_update_proc);
  
    // Create Phone battery meter Layer
    l_phone_battery = layer_create(GRect(72, 0, 70, 6));
    layer_set_update_proc(l_phone_battery, phone_battery_update_proc);
    
    // Pebble und Phone Battery Text
    lt_PebbleBat_P = text_layer_create(GRect(0, 3, 35, 14));
    text_layer_set_background_color(lt_PebbleBat_P, GColorClear);
    // text_layer_set_text(lt_PebbleBat_P, "--");
    text_layer_set_text_color(lt_PebbleBat_P, GColorWhite);
    text_layer_set_text_alignment(lt_PebbleBat_P, GTextAlignmentLeft);
    text_layer_set_font(lt_PebbleBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
    lt_PhoneBat_P = text_layer_create(GRect(124, 3, 20, 14));
    text_layer_set_background_color(lt_PhoneBat_P, GColorClear);
    snprintf(s_bufferLevel, sizeof(s_bufferLevel), "%d", s_phone_battery_level);
    text_layer_set_text(lt_PhoneBat_P, s_bufferLevel);
    // text_layer_set_text(lt_PhoneBat_P, "--");
    text_layer_set_text_color(lt_PhoneBat_P, GColorWhite);
    text_layer_set_text_alignment(lt_PhoneBat_P, GTextAlignmentRight);
    text_layer_set_font(lt_PhoneBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  
    layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PebbleBat_P);
    layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PhoneBat_P);
    
    // Add to Window
    layer_add_child(window_get_root_layer(s_window), l_battery);
    layer_add_child(window_get_root_layer(s_window), l_phone_battery);
    // layer_mark_dirty(l_battery);
    // layer_mark_dirty(l_phone_battery);
    
    
    snprintf(s_bufferPebbleLevel, sizeof(s_bufferPebbleLevel), "%d", s_battery_level);
    text_layer_set_text(lt_PebbleBat_P, s_bufferPebbleLevel);
    
    update_battery();
  
    bBatShown = true;
  } else
  {
    // NIX zu tun - ist ja schon eingeblendet...
  }
  /*
  // Create battery meter Layer
  l_battery = layer_create(GRect(0, 0, 70, 6));
  layer_set_update_proc(l_battery, battery_update_proc);

  // Create Phone battery meter Layer
  l_phone_battery = layer_create(GRect(72, 0, 70, 6));
  layer_set_update_proc(l_phone_battery, phone_battery_update_proc);
  
  // Pebble und Phone Battery Text
  lt_PebbleBat_P = text_layer_create(GRect(0, 3, 35, 14));
  text_layer_set_background_color(lt_PebbleBat_P, GColorClear);
  text_layer_set_text(lt_PebbleBat_P, "--");
  text_layer_set_text_color(lt_PebbleBat_P, GColorWhite);
  text_layer_set_text_alignment(lt_PebbleBat_P, GTextAlignmentLeft);
  text_layer_set_font(lt_PebbleBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  lt_PhoneBat_P = text_layer_create(GRect(124, 3, 20, 14));
  text_layer_set_background_color(lt_PhoneBat_P, GColorClear);
  text_layer_set_text(lt_PhoneBat_P, "--");
  text_layer_set_text_color(lt_PhoneBat_P, GColorWhite);
  text_layer_set_text_alignment(lt_PhoneBat_P, GTextAlignmentRight);
  text_layer_set_font(lt_PhoneBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PebbleBat_P);
  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PhoneBat_P);
  */
  // layer_set_hidden(text_layer_get_layer(lt_PebbleBat_P), false);
  // layer_set_hidden(text_layer_get_layer(lt_PhoneBat_P), false);
  // layer_set_hidden(l_battery, false);
  // layer_set_hidden(l_phone_battery, false);


  APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBatEinblenden() ENDE");
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  if (sbDebug)
    printf("inbox_received_callback: NIX...");
   APP_LOG(APP_LOG_LEVEL_DEBUG, "Message im C-Code");
  
  // High contrast selected?
  Tuple *vbc       = dict_find(iterator, MESSAGE_KEY_VBC);
  Tuple *vbd       = dict_find(iterator, MESSAGE_KEY_VBD);
  Tuple *bgc       = dict_find(iterator, MESSAGE_KEY_BG_COLOR);
  Tuple *sDate     = dict_find(iterator, MESSAGE_KEY_DATE_FORMAT);
  Tuple *sLang     = dict_find(iterator, MESSAGE_KEY_LANG_SEL);
  Tuple *bPhoneBat = dict_find(iterator, MESSAGE_KEY_PHONE_BAT);
  
  if (bPhoneBat)
  {
    settings.bPhoneBat = bPhoneBat->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBattery: %d", settings.bPhoneBat);
    if (settings.bPhoneBat)
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBattery AUSBLENDEN");
      PhoneBatAusblenden();
    } else
    {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "PhoneBattery ANZEIGEN");
      PhoneBatEinblenden();
    }
  }
  
  if(vbc)
  {
    settings.bVibBC = vbc->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "VBC: %d", vbc->value->int8);
  } 
  if(vbd)
  {
    settings.bVibBD = vbc->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "VBD: %d", vbd->value->int8);
  }
  if(bgc)
  {
    settings.gcBackgroundColor = GColorFromHEX(bgc->value->int32);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BGC: %s", bgc->value);
  }
  if(sDate)
  {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Datumauswahl: %s", sDate->value);
    if (strcmp((char*)sDate->value, "DD. MMM") == 0)
      settings.iDateFormat = 1;
    else if (strcmp((char*)sDate->value, "DD. MMM YYYY") == 0)
      settings.iDateFormat = 2;
    else if (strcmp((char*)sDate->value, "DD. MMM YY") == 0)
      settings.iDateFormat = 3;
    else if (strcmp((char*)sDate->value, "MMM DD") == 0)
      settings.iDateFormat = 4;
    else if (strcmp((char*)sDate->value, "MMM DD, YYYY") == 0)
      settings.iDateFormat = 5;
    else if (strcmp((char*)sDate->value, "MMM DD, YY") == 0)
      settings.iDateFormat = 6;
    else
      settings.iDateFormat = 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "DateFormat: %d", settings.iDateFormat);
      update_date();
  }
  if(sLang)
  {
    if (strcmp((char*)sLang->value, "English") == 0 )
      settings.iLang = 1;
    else // if (strcmp(sLang->value, "Deutsch"))
      settings.iLang = 0;
    update_date();
  }
  /*
  if(high_contrast_t && high_contrast_t->value->int8 > 0) {  // Read boolean as an integer
    // Change color scheme
    // window_set_background_color(s_window, GColorBlack);
    text_layer_set_text_color(lt_PebbleBat_P, GColorWhite);

    // Persist value
    persist_write_bool(MESSAGE_KEY_HIGH_CONTRAST, true);
  } else {
    // window_set_background_color(s_window, GColorWhite);
    text_layer_set_text_color(lt_PebbleBat_P, GColorBlack);
    persist_write_bool(MESSAGE_KEY_HIGH_CONTRAST, false);
  }
  */
  
  Tuple *Batterycharge_tuple = dict_find(iterator,MESSAGE_KEY_BATTERY_CHARGE);
  // TEST // Tuple *Batterystate_tuple = dict_find(iterator,MESSAGE_KEY_BATTERY_STATE);

  // If there were new phone battery status
  // TEST // if (Batterycharge_tuple && Batterystate_tuple)
  if (Batterycharge_tuple && settings.bPhoneBat == false)
  {
    if (sbDebug)
      printf("[COM][Communication_InboxReceived] Received data KEY_BATTERY_CHARGE,KEY_BATTERY_STATE");
    
    // Battery_Handle_Phone( (int)Batterycharge_tuple->value->int32, (bool)Batterystate_tuple->value->int32);
    s_phone_battery_level = Batterycharge_tuple->value->int32;
    persist_write_int(PhoneBatteryKey, s_phone_battery_level);
    APP_LOG(APP_LOG_LEVEL_INFO, "BatCharge+BattState WRITE: s_phone_battery_level: %d", s_phone_battery_level);
    if (sbDebug)
      printf("Battery-Prozent: %i", s_phone_battery_level);
    if (siPhoneBatterySent == s_phone_battery_level)
    {
      if (sbDebug)
        printf("PhoneBattery: Wert (%i) nicht neu!", s_phone_battery_level);
    } else
    {
      if (sbDebug)
        printf("PhoneBattery: Neuer Wert (%i / %i) Alt/Neu!", siPhoneBatterySent,  s_phone_battery_level);
      // layer_mark_dirty(l_phone_battery);
      // layer_mark_dirty((Layer *)lt_PhoneBat_P); // TEST
    }
    layer_mark_dirty(l_phone_battery); // TEST
    
    snprintf(s_bufferLevel, sizeof(s_bufferLevel), "%d", s_phone_battery_level);
    text_layer_set_text(lt_PhoneBat_P, s_bufferLevel);
    siPhoneBatterySent = s_phone_battery_level;
    APP_LOG(APP_LOG_LEVEL_INFO, "BatCharge+BattState s_phone_battery_level: ", s_phone_battery_level); 
  } else
    APP_LOG(APP_LOG_LEVEL_INFO, "BatCharge+BattState DISABLED!!!");
  prv_save_settings();
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  if (sbDebug)
    printf("inbox_dropped_callback: Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
  if (sbDebug)
    printf("outbox_failed_callback: Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  if (sbDebug)
    printf("outbox_sent_callback: Outbox send success!");
}



static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  snprintf(s_bufferPebbleLevel, sizeof(s_bufferPebbleLevel), "%d", s_battery_level);
  text_layer_set_text(lt_PebbleBat_P, s_bufferPebbleLevel);
  // Update meter
  layer_mark_dirty(l_battery);
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  
  static char s_bufferL[3];
  static char s_bufferR[3];
  // Display this time on the TextLayer
  // text_layer_set_text(lt_Time, s_buffer);
  
  //bitmap_layer_set_bitmap(lb_Time, s_res_image_time_bg);
  
  strftime(s_bufferL, sizeof(s_bufferL), clock_is_24h_style() ?
                                          "%H" : "%I", tick_time);
  text_layer_set_text(lt_Time_L, s_bufferL);
  
  strftime(s_bufferR, sizeof(s_bufferR), "%M", tick_time);
  text_layer_set_text(lt_Time_R, s_bufferR);
  
  // bitmap_layer_set_bitmap(lb_time_r, s_res_image_time_bg);
  update_date();
  // layer_mark_dirty(s_canvas_layer);
  // deleteme();
}

static void update_battery()
{
  if (sbDebug)
    printf("Update Battery - Funktionsaufruf...");
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, MESSAGE_KEY_BATTERY_CHARGE, MESSAGE_KEY_BATTERY_CHARGE);

  // Send the message!
  app_message_outbox_send();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // update_battery();
}

static void update_health()
{
  // Hier aktualisieren... Also aus dem Speicher lesen...
  HealthValue Heartrate = health_service_peek_current_value(HealthMetricHeartRateBPM);
  // Buffer to store the heart information
  static char s_HeartText[30];
  snprintf(s_HeartText,sizeof(s_HeartText),"%i", (int)Heartrate);

  // And display the data
  text_layer_set_text(ltHerzfrequenz, s_HeartText);

}

// Batterie Pebble (Position  0 -  70)
// Batterie Phone  (Position 74 - 144)
static void phone_battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // if (sbDebug)
  //  printf("phone_battery_update_proc: %i Phone-Battery-Level (VORHER)", s_phone_battery_level);

  // Find the width of the bar (total width = 70px)
 if (persist_exists(PhoneBatteryKey))
 {
   s_phone_battery_level = persist_read_int(PhoneBatteryKey);
   APP_LOG(APP_LOG_LEVEL_INFO, "Persistent s_phone_battery_level: ", s_phone_battery_level); 
 }
  int width = (s_phone_battery_level * 70) / 100;

  
  
  // snprintf(s_bufferLevel, sizeof(s_bufferLevel), "%d", s_phone_battery_level);
  // text_layer_set_text(lt_PhoneBat_P, s_bufferLevel);
  // layer_mark_dirty((Layer *)lt_PhoneBat_P); // TEST
  
  // if (sbDebug)
  //   printf("phone_battery_update_proc: %i Phone-Battery-Level (HINTERHER)", s_phone_battery_level);
  // Draw the background
  //graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0,0,70,2), 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(bounds.size.w - width, 0, bounds.size.w, bounds.size.h-2), 0, GCornerNone);
  // 50 % Linie
  graphics_fill_rect(ctx, GRect(35, 0, 1, bounds.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(17, 0, 1, bounds.size.h-1), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(52, 0, 1, bounds.size.h-1), 0, GCornerNone);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // if (sbDebug)
  //       printf("battery_update_proc()");
  // Find the width of the bar (total width = 70px)
  int width = (s_battery_level * 70) / 100;

  // snprintf(s_bufferPebbleLevel, sizeof(s_bufferPebbleLevel), "%d", s_battery_level);
  // text_layer_set_text(lt_PebbleBat_P, s_bufferPebbleLevel);
  
  // Draw the background
  //graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0,0,70,2), 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h-2), 0, GCornerNone);
  // 50 % Linie
  graphics_fill_rect(ctx, GRect(35, 0, 1, bounds.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(17, 0, 1, bounds.size.h-1), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(52, 0, 1, bounds.size.h-1), 0, GCornerNone);
}

static void battery_update_proc2(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  // if (sbDebug)
  //       printf("battery_update_proc()");
  // Find the width of the bar (total width = 70px)
  int width = (s_battery_level * 144) / 100;

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_fill_rect(ctx, GRect(0,0,144,2), 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h-2), 0, GCornerNone);
  // 50 % Linie
  graphics_fill_rect(ctx, GRect(72, 0, 1, bounds.size.h), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(108, 0, 1, bounds.size.h-1), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(36, 0, 1, bounds.size.h-1), 0, GCornerNone);
}

static void layer_update_proc(Layer *layer, GContext *ctx) {
  // Draw the image with the correct compositing mode
  graphics_context_set_compositing_mode(ctx, GCompOpSet);
  graphics_draw_bitmap_in_rect(ctx, s_res_image_time_bg, gbitmap_get_bounds(s_res_image_time_bg));
}

static void initialise_ui(void) {
  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  #ifndef PBL_SDK_3
    window_set_fullscreen(s_window, true);
  #endif
  
  // Create GFont
  sf_loco = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LECO_55));

  s_res_image_dp = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DP);
  
  s_res_image_time_bg = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BL);
  s_res_roboto_bold_subset_49 = fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49);
  
  // lb_DP
  // lb_DP = bitmap_layer_create(GRect(67, 56, 10, 52));
  // bitmap_layer_set_bitmap(lb_DP, s_res_image_dp);
  // layer_add_child(window_get_root_layer(s_window), (Layer *)lb_DP);
  
  //lb_time_r = bitmap_layer_create(GRect(81, 55, 63, 42));
  // lb_time_r = bitmap_layer_create(GRect(0, 0, 63, 42));
  // bitmap_layer_set_bitmap(lb_time_r, s_res_image_time_bg);
  //layer_add_child(window_get_root_layer(s_window), (Layer *)lb_time_r);
  
  // TEST MW 2017-10-11
  // Layer *window_layer = window_get_root_layer(s_window);
  
  // HIER // Layer *window_layer = (Layer *)lb_Time;
    
  // Create GBitmap
  // s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_B_T_WHITE);

  // Create canvas Layer
  // s_canvas_layer = layer_create(layer_get_bounds(window_layer));
  // s_canvas_layer = layer_create(GRect(81, 55, 63, 42));
  s_canvas_layer = layer_create(GRect(0, 0, 72, 71));
  layer_set_update_proc(s_canvas_layer, layer_update_proc);
  
  // TEST MW 2017-10-11 ENDE
  
  
  // lt_Time
  // lt_Time = text_layer_create(GRect(67, 50, 10, 52));
  // text_layer_set_background_color(lt_Time, GColorClear);
  // text_layer_set_text(lt_Time, "22:22");
  // text_layer_set_text_color(lt_Time, GColorWhite);
  // text_layer_set_text_alignment(lt_Time, GTextAlignmentCenter);
  // 2017-10-11
  // // text_layer_set_font(lt_Time, sf_scifi);
  // text_layer_set_font(lt_Time, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  // layer_add_child(window_get_root_layer(s_window), (Layer *)lt_Time);
  
  lt_Time_L = text_layer_create(GRect(0, 50, 71, 71));
  text_layer_set_background_color(lt_Time_L, GColorClear);
  text_layer_set_text(lt_Time_L, "22");
  text_layer_set_text_color(lt_Time_L, GColorWhite);
  text_layer_set_text_alignment(lt_Time_L, GTextAlignmentRight);
  // text_layer_set_font(lt_Time_L, fonts_get_system_font(FONT_KEY_LECO_42_NUMBERS));
  text_layer_set_font(lt_Time_L, sf_loco);
  
  lt_Time_R = text_layer_create(GRect(73, 50, 71, 71));
  text_layer_set_background_color(lt_Time_R, GColorClear);
  text_layer_set_text(lt_Time_R, "22");
  text_layer_set_text_color(lt_Time_R, GColorWhite);
  text_layer_set_text_alignment(lt_Time_R, GTextAlignmentLeft);
  text_layer_set_font(lt_Time_R, sf_loco);

  
  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_Time_L);
  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_Time_R);
  // layer_add_child(window_get_root_layer(s_window), (Layer *)lb_DP);
  // layer_add_child(window_layer, s_canvas_layer);
  // layer_add_child((Layer *)lt_Time_R, (Layer *)lb_time_r);
  // layer_add_child((Layer *)lb_time_r, s_canvas_layer);
  layer_add_child((Layer *)lt_Time_R, s_canvas_layer);

  // PhoneBatEinblenden();
  bBatShown = true;
  // Create battery meter Layer
  l_battery = layer_create(GRect(0, 0, 70, 6));
  layer_set_update_proc(l_battery, battery_update_proc);

  // Create Phone battery meter Layer
  l_phone_battery = layer_create(GRect(72, 0, 70, 6));
  layer_set_update_proc(l_phone_battery, phone_battery_update_proc);
  
  // Pebble und Phone Battery Text
  lt_PebbleBat_P = text_layer_create(GRect(0, 3, 35, 14));
  text_layer_set_background_color(lt_PebbleBat_P, GColorClear);
  text_layer_set_text(lt_PebbleBat_P, "--");
  text_layer_set_text_color(lt_PebbleBat_P, GColorWhite);
  text_layer_set_text_alignment(lt_PebbleBat_P, GTextAlignmentLeft);
  text_layer_set_font(lt_PebbleBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  lt_PhoneBat_P = text_layer_create(GRect(124, 3, 20, 14));
  text_layer_set_background_color(lt_PhoneBat_P, GColorClear);
  text_layer_set_text(lt_PhoneBat_P, "--");
  text_layer_set_text_color(lt_PhoneBat_P, GColorWhite);
  text_layer_set_text_alignment(lt_PhoneBat_P, GTextAlignmentRight);
  text_layer_set_font(lt_PhoneBat_P, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PebbleBat_P);
  layer_add_child(window_get_root_layer(s_window), (Layer *)lt_PhoneBat_P);
  
  // ENDE
  s_res_image_herz = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HERZ_W);
  // lbHerz
  lbHerz = bitmap_layer_create(GRect(20, 116, 20, 20));
  bitmap_layer_set_bitmap(lbHerz, s_res_image_herz);
  layer_add_child(window_get_root_layer(s_window), (Layer *)lbHerz);

  s_res_roboto_condensed_21 = fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21);
  // ltHerzfrequenz
  ltHerzfrequenz = text_layer_create(GRect(55, 112, 89, 22));
  text_layer_set_background_color(ltHerzfrequenz, GColorClear);
  text_layer_set_text_color(ltHerzfrequenz, GColorWhite);
  text_layer_set_text(ltHerzfrequenz, "---");
  text_layer_set_text_alignment(ltHerzfrequenz, GTextAlignmentLeft);
  text_layer_set_font(ltHerzfrequenz, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)ltHerzfrequenz);

  // s_res_image_schritte = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SCHRITTE);
  s_res_image_schritte = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SHOE_W);
  // lbSchritte
  // lbSchritte = bitmap_layer_create(GRect(20, 138, 20, 20));
  lbSchritte = bitmap_layer_create(GRect(20, 142, 25, 13));
  bitmap_layer_set_bitmap(lbSchritte, s_res_image_schritte);
  layer_add_child(window_get_root_layer(s_window), (Layer *)lbSchritte);

  // Schritte : step count
  // s_step_layer = text_layer_create(GRect(0, 54, 144, 38));
  s_step_layer = text_layer_create(GRect(55, 132, 89, 38));
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentLeft);
  layer_add_child(window_get_root_layer(s_window), text_layer_get_layer(s_step_layer));

  
  // Progress indicator
  // s_progress_layer = layer_create(window_bounds);
  s_progress_layer = layer_create(GRect(0, 162, 144, 6));
  layer_set_update_proc(s_progress_layer, progress_layer_update_proc);
  layer_add_child(window_get_root_layer(s_window), s_progress_layer);

/*
  // ltSchritte
  ltSchritte = text_layer_create(GRect(24, 144, 120, 22));
  text_layer_set_background_color(ltSchritte, GColorClear);
  text_layer_set_text_color(ltSchritte, GColorWhite);
  text_layer_set_text(ltSchritte, "---");
  text_layer_set_text_alignment(ltSchritte, GTextAlignmentLeft);
  text_layer_set_font(ltSchritte, s_res_roboto_condensed_21);
  layer_add_child(window_get_root_layer(s_window), (Layer *)ltSchritte);
*/
  // Add to Window
  layer_add_child(window_get_root_layer(s_window), l_battery);
  layer_add_child(window_get_root_layer(s_window), l_phone_battery);

  // ltDatum
  ltDatum = text_layer_create(GRect(0, 28, 144, 30));
  text_layer_set_background_color(ltDatum, GColorClear);
  text_layer_set_text_color(ltDatum, GColorWhite);
  text_layer_set_text(ltDatum, "14. Mai 2017");
  text_layer_set_text_alignment(ltDatum, GTextAlignmentCenter);
  text_layer_set_font(ltDatum, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_get_root_layer(s_window), (Layer *)ltDatum);
  // ltWochentag
  ltWochentag = text_layer_create(GRect(6, 10, 132, 30));
  text_layer_set_background_color(ltWochentag, GColorClear);
  text_layer_set_text_color(ltWochentag, GColorWhite);
  text_layer_set_text(ltWochentag, "Sonntag");
  text_layer_set_text_alignment(ltWochentag, GTextAlignmentCenter);
  text_layer_set_font(ltWochentag, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_get_root_layer(s_window), (Layer *)ltWochentag);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  // Register for Bluetooth connection updates
/*
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback,
    .pebblekit_connection_handler = kit_connection_handler
  });
*/
connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  // Subscribe to health event handler
  health_service_events_subscribe(prv_on_health_data, NULL);

  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void destroy_ui(void) {
  window_destroy(s_window);
  //bitmap_layer_destroy(lb_DP);
  
  if (bBatShown)
  {
    text_layer_destroy(lt_PhoneBat_P);
    layer_destroy(l_phone_battery);
  }
  text_layer_destroy(lt_Time_L);
  text_layer_destroy(lt_Time_R);
  text_layer_destroy(lt_PebbleBat_P);
  
  text_layer_destroy(ltDatum);
  text_layer_destroy(ltWochentag);
  gbitmap_destroy(s_res_image_time_bg);
  layer_destroy(l_battery);
  
  bitmap_layer_destroy(lbHerz);
  gbitmap_destroy(s_res_image_herz);
  text_layer_destroy(ltHerzfrequenz);
  gbitmap_destroy(s_res_image_schritte);
  text_layer_destroy(ltSchritte);
  layer_destroy(text_layer_get_layer(s_step_layer));
  layer_destroy(s_progress_layer);
}
// END AUTO-GENERATED UI CODE

static void handle_window_unload(Window* window) {
  destroy_ui();
}

/*
void testpersistantreadwrite()
{
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() BEGINN");
  

  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(vorher): %d", settings.bPhoneBat);
  settings.bPhoneBat = false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gesetzt): %d", settings.bPhoneBat);
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(geschrieben): %d", settings.bPhoneBat);
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gelesen): %d", settings.bPhoneBat);

  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(vorher): %d", settings.bPhoneBat);
  settings.bPhoneBat = true;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gesetzt): %d", settings.bPhoneBat);
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(geschrieben): %d", settings.bPhoneBat);
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gelesen): %d", settings.bPhoneBat);

  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(vorher): %d", settings.bPhoneBat);
  settings.bPhoneBat = false;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gesetzt): %d", settings.bPhoneBat);
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(geschrieben): %d", settings.bPhoneBat);
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() PhoneBattery(gelesen): %d", settings.bPhoneBat);

  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "testpersistantreadwrite() ENDE");
}
*/



void show_pebble2window(void) {
  // TEST -- SAVE PERSISTANT -- BEGINN
  
  // testpersistantreadwrite();
  
  // TEST -- SAVE PERSISTANT -- ENDE
  prv_load_settings();
  initialise_ui();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .unload = handle_window_unload,
  });
  
  // BEGINN
  // Offenbar wird beim iPhone gelegentlich die Pebble "resettet" - und dann nicht die defaults geladen...
  if (settings.bPhoneBat)
  {
    PhoneBatAusblenden();
  }
  
  window_stack_push(s_window, true);
  update_time();
  battery_callback(battery_state_service_peek());
  if (sbDebug)
    printf("show_pebble2window()");
  update_battery();
  update_health();
}

void hide_pebble2window(void) {
  window_stack_remove(s_window, true);
  // prv_save_settings();
}




// NEU 2017-10-18

// Initialize the default settings
static void prv_default_settings() {
/*
  GColor gcBackgroundColor;
  bool bSecondTick;
  bool bVibBC;
  bool bVibBD;
  char cLang[20];
  int iLang;
  int iDateFormat;
*/
  
  settings.gcBackgroundColor = GColorBlack;
  settings.bSecondTick = false;
  settings.bVibBC = false;
  settings.bVibBD = false;
  settings.iLang = 0;
  settings.iDateFormat = 1;
  settings.bPhoneBat = true;
  // Init Sprachbasierende Informationen...
  // English  
  strcpy(MONATE[1][0], "January");
  strcpy(MONATE[1][1], "February");
  strcpy(MONATE[1][2], "March");
  strcpy(MONATE[1][3], "April");
  strcpy(MONATE[1][4], "May");
  strcpy(MONATE[1][5], "June");
  strcpy(MONATE[1][6], "July");
  strcpy(MONATE[1][7], "August");
  strcpy(MONATE[1][8], "September");
  strcpy(MONATE[1][9], "October");
  strcpy(MONATE[1][10], "November");
  strcpy(MONATE[1][11], "December");
  
  strcpy(WOCHENTAG[1][0], "SUNDAY");
  strcpy(WOCHENTAG[1][1], "MONDAY");
  strcpy(WOCHENTAG[1][2], "TUESDAY");
  strcpy(WOCHENTAG[1][3], "WEDNESDAY");
  strcpy(WOCHENTAG[1][4], "THURSDAY");
  strcpy(WOCHENTAG[1][5], "FRIDAY");
  strcpy(WOCHENTAG[1][6], "SATURDAY");
  
  // Deutsch (DEFAULT)
  strcpy(MONATE[0][0], "Januar");
  strcpy(MONATE[0][1], "Februar");
  strcpy(MONATE[0][2], "März");
  strcpy(MONATE[0][3], "April");
  strcpy(MONATE[0][4], "Mai");
  strcpy(MONATE[0][5], "Juni");
  strcpy(MONATE[0][6], "Juli");
  strcpy(MONATE[0][7], "August");
  strcpy(MONATE[0][8], "September");
  strcpy(MONATE[0][9], "Oktober");
  strcpy(MONATE[0][10], "November");
  strcpy(MONATE[0][11], "Dezember");
  
  strcpy(WOCHENTAG[0][0], "SONNTAG");
  strcpy(WOCHENTAG[0][1], "MONTAG");
  strcpy(WOCHENTAG[0][2], "DIENSTAG");
  strcpy(WOCHENTAG[0][3], "MITTWOCH");
  strcpy(WOCHENTAG[0][4], "DONNERSTAG");
  strcpy(WOCHENTAG[0][5], "FREITAG");
  strcpy(WOCHENTAG[0][6], "SAMSTAG");
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  // Update the display based on new settings
  prv_update_display();
}

// Update the display elements
static void prv_update_display() {
  // Background color
  window_set_background_color(s_window, settings.gcBackgroundColor);
}