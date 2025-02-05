

#include <lvgl.h>
#include <atomic>

#include <Arduino_GFX_Library.h>
#include "ble_nimble_server.h"
#include "requests.h"
#include "shared_yaml_parser.h"

#define TFT_BL 27
#define GFX_BL DF_GFX_BL // default backlight pin

/* Display configuration */
Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, -1 /* RST */, 3 /* rotation */, true /* IPS */);

/* Touch include */
#include "touch.h"
#define STACK_SIZE 8192

// define hex_colors
#define HEX_BLACK lv_color_hex(0x000000) // Black
#define HEX_WHITE lv_color_hex(0xffffff) // White
#define HEX_LIGHT_GRAY lv_color_hex(0xf4f4f4) // Light Gray
#define HEX_LIGHT_GRAY_2 lv_color_hex(0xdfdfdf) // Light Gray_2
#define HEX_MEDIUM_GRAY lv_color_hex(0xCACACA) // Medium Gray
#define HEX_DARK_GRAY lv_color_hex(0x404040) // Dark Gray
#define HEX_DARK_BLUE lv_color_hex(0x00008b) // Dark Blue
#define HEX_ROYAL_BLUE lv_color_hex(0x4169e1) // Royal Blue
#define HEX_SKY_BLUE lv_color_hex(0x00bfff) // Sky Blue
#define HEX_PINK lv_color_hex(0xffc0cb) // Pink
#define HEX_PURPLE lv_color_hex(0x800080) // Purple
#define HEX_RED lv_color_hex(0xc30a12) // Red
#define HEX_BABY_PINK lv_color_hex(0xf4c2c2) // Baby Pink
#define HEX_BURGUNDY lv_color_hex(0x800020) // Burgundy
#define HEX_GOLD lv_color_hex(0xffd700) // Gold
#define HEX_TURQUOISE lv_color_hex(0x40e0d0) // Turquoise
#define HEX_LIGHT_PURPLE lv_color_hex(0xda70d6) // Light Purple
#define HEX_MEDIUM_PURPLE lv_color_hex(0x9370db) // Medium Purple
#define HEX_DARK_PURPLE lv_color_hex(0x663399) // Dark Purple
#define HEX_YELLOW lv_color_hex(0xf7edbe) // Bannana yellow
#define HEX_GREEN lv_color_hex(0x047a04) // Green

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
static lv_obj_t *tabview;  // Declare the global tabview variable
static lv_obj_t* tech_tab = NULL;

static bool is_user = false;
static bool is_tech = false;
static bool yaml_structs_ready=false;
static bool initial_user_screen_flag = false;
static bool has_unsaved_changes = false;
static bool curr_is_Setup = false;
static std::vector<lv_obj_t*> sensorSwitchVec;
static lv_obj_t* send_new_switches_msg_box = NULL;
// static lv_obj_t* curr_msg_box = NULL;
static lv_obj_t* save_btn = NULL;
static lv_obj_t* save_btn_tech_sensors = NULL;
static lv_obj_t* save_btn_tech_motors = NULL;
lv_obj_t * meter = NULL;


static lv_obj_t* msg_box_parrent = NULL;
static std::vector<lv_obj_t*> obj_to_delete_sensors;
static std::vector<lv_obj_t*> obj_to_delete_motors;
static int current_edit_sensor_id = -1;
static int current_edit_motor_id = -1;

static std::vector<lv_obj_t*> current_edit_sensor_sliders_vec;
static std::vector<lv_obj_t*> current_edit_motor_sliders_vec;

static uint16_t curr_tech_tabview_id = 4;
static lv_obj_t* tech_tabview =NULL;
static lv_obj_t * dropdown_motors =NULL;
static lv_obj_t * dropdown_sensors =NULL;
static lv_obj_t * dropdown_motors_activity =NULL;
static lv_obj_t * tech_tab_motors =NULL;
static lv_obj_t * tech_tab_sensors =NULL;
static lv_obj_t * tech_tab_motors_activity =NULL;
static bool sensor_trigged_by_tech_tab = false;
static bool motor_trigged_by_tech_tab = false;

const char* empty_string = "";


int tech_pass;
int debug_pass; 

lv_obj_t *initial_user_screen = NULL; //
lv_obj_t * read_yaml_from_prot_screen = NULL;
lv_obj_t* searchBLE_screen = NULL;
lv_obj_t *password_screen =NULL;
static lv_obj_t *textarea = NULL;
static lv_obj_t* msg_close_btm = NULL;

char selected_text_to_title[32]; 











const int buttonPin = 0; // PIN NUMBER OF EMERGEANCY BUTTON
volatile unsigned long lastPressTime = 0;  // Stores the last press time
const unsigned long debounceDelay = 1000;  // Min time between presses (ms). used to avoid stack overflow

void IRAM_ATTR buttonPress() {
      unsigned long currentTime = millis();
    if (currentTime - lastPressTime < debounceDelay) {
        return;  // Ignore if pressed too quickly
    }
    lastPressTime = currentTime;

    if (bleNotifyTaskHandle == NULL) {
        ets_printf("❌ ERROR: Notify Task Handle is NULL in ISR!\n");
        return;
    }

    Serial.printf("ISR triggered!\n");
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(bleNotifyTaskHandle, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}



struct Return_unsaved_param{
  int sensor_id;
  std::vector<int> params_id;
  std::vector<int> new_vals;
};



/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
#endif

  lv_disp_flush_ready(disp);
}

/* Read touch points */
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  if (touch_has_signal())
  {
      if (touch_touched())
      {
          data->state = LV_INDEV_STATE_PR;
          /*Set the coordinates*/
          data->point.x = touch_last_x;
          data->point.y = touch_last_y;
      }
      else if (touch_released())
      {
          data->state = LV_INDEV_STATE_REL;
      }
  }
  else
  {
      data->state = LV_INDEV_STATE_REL;
  }
}

void show_password_screen(lv_event_t *e) {
  // Load the new screen
  lv_scr_load_anim(password_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

lv_obj_t* create_new_btn(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, const char* name, lv_color_t btn_color, lv_color_t label_color, const lv_font_t* label_size=&lv_font_montserrat_14 ){
    lv_obj_t *new_btn = lv_btn_create(parent); // Create button with home_tab as parent
    lv_obj_set_size(new_btn, w, h); // Set button size

    // Set the button color
    lv_obj_set_style_bg_color(new_btn, btn_color, 0);


    lv_obj_align(new_btn, align, x_ofs, y_ofs); // Align to bottom left
    lv_obj_t *label = lv_label_create(new_btn);
    lv_obj_set_style_text_color(label, label_color, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_AUTO, 0);
    lv_obj_set_style_text_font(label,label_size,0);


    lv_label_set_text(label, name);
    

    return new_btn;
} 

void designd_btnm(lv_event_t * e){
  lv_color_t* color = (lv_color_t*) lv_event_get_user_data(e);
  lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
  if (dsc->part == LV_PART_ITEMS){
    dsc->rect_dsc->bg_color = *color;
  }
}

void designd_label_big(lv_event_t * e){
  lv_color_t* color = (lv_color_t*) lv_event_get_user_data(e);
  lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
  if (dsc->part == LV_PART_ITEMS){
    dsc->label_dsc->color = *color;
    dsc->label_dsc->font = &lv_font_montserrat_20;
  }
}

void designd_label_small(lv_event_t * e){
  lv_color_t* color = (lv_color_t*) lv_event_get_user_data(e);
  lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
  if (dsc->part == LV_PART_ITEMS){
    dsc->label_dsc->color = *color;
    dsc->label_dsc->font = &lv_font_montserrat_10;
  }
}

void gestures_click_event(lv_event_t * e){
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
    lv_obj_t * label_current_text = lv_obj_get_child(btn, 0);
    char* current_text = lv_label_get_text(label_current_text);


    if(code == LV_EVENT_PRESSED ){
      if (!(can_play_gesture.test_and_set())){
        can_play_gesture.clear();
        lv_label_set_text(label, "#c30a12  Can't Play#\n #c30a12 gesture #");
      }
      else{
        can_play_gesture.clear();

        sending_gesture(current_text); 
        char new_text[40]; // Adjust size as needed
        snprintf(new_text, sizeof(new_text), "#047a04  Playing:\n %s#", current_text);
        lv_label_set_text(label, new_text);
      }
    }
    else if(code == LV_EVENT_CLICKED){
      lv_label_set_text(label, " ");
    }

}



lv_obj_t* create_new_matrix_btn_choose_one(lv_obj_t * parent, char *** map, int len_map, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs,lv_color_t bg_color,lv_color_t btms_color,lv_color_t labls_color, bool big = true, lv_coord_t screen_width = 320,lv_coord_t screen_height = 240,int max_in_row=-1, bool need_to_free_old_map=false){
    
    lv_obj_t* btnm = lv_btnmatrix_create(parent);

    // fit labels to matrix
    if (max_in_row > 0){
      // create modify map
      int new_num_pointer = len_map + (len_map-1)/max_in_row;
      char** temp_ptr = (char**)malloc(new_num_pointer * sizeof(char*));
      int j = 0;
      for(int i=0; i<len_map-1; i++){
        temp_ptr[j] = *((*map)+i);
        if(i%max_in_row == (max_in_row-1)){
          j++;
          temp_ptr[j] = "\n";
        }
        j++;
      }
      temp_ptr[j] = "";

      if(need_to_free_old_map){
        free(*map);
      }
      (*map) = temp_ptr;
    }

    lv_btnmatrix_set_map(btnm, (const char**)(*map));
    lv_btnmatrix_set_one_checked(btnm, true); // Only one button can be checked at a time

    lv_obj_set_size(btnm, screen_width*0.7, screen_height*0.7);
    lv_obj_set_style_bg_color(btnm, bg_color, 0);
    
    // design each btmn color
    static lv_color_t btmns_color = btms_color;
    lv_obj_add_event_cb(btnm, designd_btnm, LV_EVENT_DRAW_PART_BEGIN, &btmns_color );

    static lv_color_t labels_color = labls_color;
    if(big) lv_obj_add_event_cb(btnm, designd_label_big, LV_EVENT_DRAW_PART_BEGIN, &labels_color);
    else lv_obj_add_event_cb(btnm, designd_label_small, LV_EVENT_DRAW_PART_BEGIN, &labels_color);

    


    lv_obj_align(btnm, align, x_ofs, y_ofs); // Position below the label
    return btnm;
}

int get_gest_num(){
    int num_gest =0;
    for (const auto& function : functions) {
        if(function.protocol_type == FUNC_TYPE_GESTURE){
          num_gest++;
        }
    }
    return num_gest;
}

void create_controls_for_main(lv_obj_t* parent) {
// Add the "Return" button to home tab
    lv_obj_t* return_btn = create_new_btn(parent, 80, 40, LV_ALIGN_BOTTOM_MID, -110, 0, "Return", HEX_RED , HEX_WHITE,&lv_font_montserrat_16);
    lv_obj_add_event_cb(return_btn, return_to_main, LV_EVENT_CLICKED, NULL); // Set the event handler
    lv_obj_set_style_text_align(return_btn, LV_TEXT_ALIGN_CENTER, 0);


    
    lv_obj_t * label_home = lv_label_create(parent);
    lv_label_set_recolor(label_home, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label_home, "#000099 S##000066 P##00007f M##0000b2 T#");
    lv_obj_set_style_text_align(label_home, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_home, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(label_home,&lv_font_montserrat_18,0);

    lv_obj_t * label_home_more = lv_label_create(parent);
    lv_label_set_recolor(label_home_more, true);        
    lv_obj_set_width(label_home_more, 75);
    lv_label_set_text(label_home_more, "#000099 Powered by IOT Lab Technion#");
    lv_obj_set_style_text_align(label_home_more, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(label_home_more, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_set_style_text_font(label_home_more,&lv_font_montserrat_12,0);

    lv_obj_t * label_home_BLE_1 = lv_label_create(parent);
    lv_obj_t * label_home_BLE_2 = lv_label_create(parent);
    lv_label_set_recolor(label_home_BLE_1, true);  
    lv_label_set_recolor(label_home_BLE_2, true);        
      
    lv_obj_set_width(label_home_BLE_1, 75);
    lv_obj_set_width(label_home_BLE_2, 55);

    lv_label_set_text(label_home_BLE_1,  LV_SYMBOL_BLUETOOTH );

    if(has_client.test_and_set()){
        lv_label_set_text(label_home_BLE_2,  LV_SYMBOL_OK);
        lv_obj_set_style_text_color(label_home_BLE_1, HEX_ROYAL_BLUE, 0); // Green for ON
        lv_obj_set_style_text_color(label_home_BLE_2, HEX_GREEN, 0); // Green for ON
    }
    else{
        lv_label_set_text(label_home_BLE_2,  LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(label_home_BLE_2, HEX_RED, 0); // Red for OFF
        has_client.clear();
    }
    
    lv_obj_set_style_text_align(label_home_BLE_1, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_align(label_home_BLE_2, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_align(label_home_BLE_1, LV_ALIGN_TOP_LEFT, 0, 80);
    lv_obj_align(label_home_BLE_2, LV_ALIGN_TOP_LEFT, 22, 80);
    lv_obj_set_style_text_font(label_home_BLE_1,&lv_font_montserrat_22,0);
    lv_obj_set_style_text_font(label_home_BLE_2,&lv_font_montserrat_22,0);

    int num_gest = get_gest_num();
    int num_function_total = functions.size();

    lv_obj_t** gestures_matrix = (lv_obj_t**)malloc((num_gest)*sizeof(lv_obj_t*));
    int max_in_row = 2;


    int j = 0;
    for(int i=0; i< num_function_total; i++){
      if(functions[i].protocol_type == FUNC_TYPE_GESTURE){
        const char* temp_str = (functions[i].name).c_str();
        gestures_matrix[j] = create_new_btn(parent, 90, 30, LV_ALIGN_TOP_RIGHT, -7 - (j%max_in_row)*95, 20 + (j/max_in_row)*35 , temp_str,HEX_DARK_BLUE,HEX_WHITE );
        j++;
      }
    }

    lv_obj_t * info_label = lv_label_create(parent);
    // int label_offfset_y = std::max( 30 + ((j-1)/max_in_row)*35, 270);
    lv_obj_align(info_label,LV_ALIGN_BOTTOM_LEFT, 0, -45);
    lv_obj_set_width(info_label, 84);
    lv_label_set_recolor(info_label, true);
    lv_obj_set_style_text_font(info_label,&lv_font_montserrat_12,0);
    lv_label_set_text(info_label, " ");

    for(int k=0; k < j; k++){
        lv_obj_add_event_cb(gestures_matrix[k], gestures_click_event, LV_EVENT_PRESSED, info_label); // Set the event handler
        lv_obj_add_event_cb(gestures_matrix[k], [](lv_event_t* e) { delay(800);}, LV_EVENT_CLICKED, NULL); // Set the event handler
        lv_obj_add_event_cb(gestures_matrix[k], gestures_click_event, LV_EVENT_CLICKED, info_label); // Set the event handler
    }



        // gestures matrix hadder
    lv_obj_t* label_gest = lv_label_create(parent);
    lv_label_set_text(label_gest, "Gestures");
    // lv_obj_center(label);
    lv_obj_align(label_gest,LV_ALIGN_TOP_RIGHT, -64, -5);
    lv_obj_set_style_text_font(label_gest,&lv_font_montserrat_18,0);
    // lv_obj_add_event_cb(return_btn, return_to_main, LV_EVENT_CLICKED, NULL); // Set the event handler
}
std::vector<int> find_new_off_sensor(){
  std::vector<int> ret_vec;
  for (int i=0; i< sensors.size(); i++) {
    if((sensors[i].status.equalsIgnoreCase("ON")) && (!lv_obj_has_state(sensorSwitchVec[i],LV_STATE_CHECKED))){
      ret_vec.push_back(i);
    }
  }
  return ret_vec;
}

std::vector<int> find_new_on_sensor(){
  std::vector<int> ret_vec;
  for (int i=0; i< sensors.size(); i++) {
    if((sensors[i].status.equalsIgnoreCase("OFF")) && (lv_obj_has_state(sensorSwitchVec[i],LV_STATE_CHECKED))){
      ret_vec.push_back(i);
    }
  }
  return ret_vec;
}

void create_controls_for_stat(lv_obj_t* parent){

  // Create a title label
  lv_obj_t* title_label = lv_label_create(parent); 
  lv_label_set_text(title_label, "Sensors State:"); 
  lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10); 
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0); 

  int y_offset = 40; // Vertical spacing between rows

  // Loop through each sensor
  for (const auto& sensor : sensors) { 
      // Replace "_" in the name with spaces
      String display_name = sensor.name;
      display_name.replace("_", " ");
      display_name[0] = toupper(display_name[0]);



      // Set the style of the status label based on the sensor status
      if (sensor.status.equalsIgnoreCase("ON")) {
          // Create a label for the sensor name
          lv_obj_t* sensor_name_label = lv_label_create(parent); 
          lv_label_set_text(sensor_name_label, (display_name + ":").c_str()); 
          lv_obj_align(sensor_name_label, LV_ALIGN_TOP_LEFT, 15, y_offset); 
          lv_obj_set_style_text_font(sensor_name_label, &lv_font_montserrat_16, 0); 

          // Create a status label
          lv_obj_t* sensor_status_label = lv_label_create(parent); 
          lv_obj_align(sensor_status_label, LV_ALIGN_TOP_RIGHT, -15, y_offset); 
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_16, 0);
          
          lv_label_set_text(sensor_status_label, "ON " LV_SYMBOL_OK); 
          lv_obj_set_style_text_color(sensor_status_label, HEX_GREEN, 0); // Green for ON
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_16, 0);  // Slightly larger font
          y_offset += 28;
      } 

      // Update the vertical offset for the next sensor
      
  }
  for (const auto& sensor : sensors){
      String display_name = sensor.name;
      display_name.replace("_", " ");
      display_name[0] = toupper(display_name[0]);

    if (sensor.status.equalsIgnoreCase("OFF")) {
          // Create a label for the sensor name
          lv_obj_t* sensor_name_label = lv_label_create(parent); 
          lv_label_set_text(sensor_name_label, (display_name + ":").c_str()); 
          lv_obj_align(sensor_name_label, LV_ALIGN_TOP_LEFT, 15, y_offset); 
          lv_obj_set_style_text_font(sensor_name_label, &lv_font_montserrat_16, 0); 

          // Create a status label
          lv_obj_t* sensor_status_label = lv_label_create(parent); 
          lv_obj_align(sensor_status_label, LV_ALIGN_TOP_RIGHT, -15, y_offset); 
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_16, 0);
      
          lv_label_set_text(sensor_status_label, "OFF " LV_SYMBOL_CLOSE); 
          lv_obj_set_style_text_color(sensor_status_label,HEX_RED, 0); // Red for OFF
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_16, 0);  // Slightly larger font

          y_offset += 28;

    }
  }
}

void discard_btn_click_event(lv_event_t * e){

  for (int i=0; i< sensors.size(); i++) {
        if(sensors[i].status.equalsIgnoreCase("ON")){
          lv_obj_add_state(sensorSwitchVec[i], LV_STATE_CHECKED);
        }
        else if(sensors[i].status.equalsIgnoreCase("OFF")){
          lv_obj_clear_state(sensorSwitchVec[i], LV_STATE_CHECKED);
        }
        else{
          Serial.printf("ERROR PLEASE CHECK");
        }
  }
  
}

void discard_sensor_btn_click_event(lv_event_t * e){
  struct Return_unsaved_param unsave_struct = check_unsave_sensor_param();
  if(unsave_struct.params_id.size() > 0){
    // int param_slider_id = 0;
    int i = 0;
    for (const auto& [paramName, param] : sensors[current_edit_sensor_id].function.parameters){
      if(param.current_val != lv_slider_get_value(current_edit_sensor_sliders_vec[i])){
        lv_slider_set_value(current_edit_sensor_sliders_vec[i], param.current_val, LV_ANIM_ON);
        lv_event_send(current_edit_sensor_sliders_vec[i], LV_EVENT_VALUE_CHANGED, NULL);
      }
      i++;
    }
    

  }

}


void discard_motor_btn_click_event(lv_event_t * e){
  struct Return_unsaved_param unsave_tsh = check_unsave_motor_param();
  if(unsave_tsh.params_id.size() > 0){
    lv_slider_set_value(current_edit_motor_sliders_vec[0], motors[current_edit_motor_id].safety_threshold.current_val, LV_ANIM_ON);
    lv_event_send(current_edit_motor_sliders_vec[0], LV_EVENT_VALUE_CHANGED, NULL);
  }
}

void save_new_switch_btms_to_struct(){
  for (int i=0; i< sensors.size(); i++) {
    if(!lv_obj_has_state(sensorSwitchVec[i],LV_STATE_CHECKED)){
      sensors[i].status = "off";
    }
    else if(lv_obj_has_state(sensorSwitchVec[i],LV_STATE_CHECKED)){
      sensors[i].status = "on";
    }
    else{
      Serial.printf("PROBLEM IN SAVE NEW BTM");
    }
  }
}

void save_new_motor_val_to_struct(struct Return_unsaved_param motor_ths_struct){
    motors[motor_ths_struct.sensor_id].safety_threshold.current_val = motor_ths_struct.new_vals[0];
}

void save_new_sensors_val_to_struct(struct Return_unsaved_param sensors_struct){
    int i = 0;
    int j = 0;
    for(auto& [name, param] : sensors[sensors_struct.sensor_id].function.parameters){
      if(sensors_struct.params_id[i] == j){
        param.current_val = sensors_struct.new_vals[i];
        i++;
      }
      j++;
    }
}


void save_btn_approved(lv_event_t * e){
  if(send_new_switches_msg_box){
    lv_msgbox_close(send_new_switches_msg_box);
    send_new_switches_msg_box = NULL;
  }
  save_new_switch_btms_to_struct();
  lv_obj_clean(stat_tab);
  create_controls_for_stat(stat_tab);
  lv_obj_t* curr_msg_box = lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.\nProthesis is updated.",NULL, true);
  lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
}

void save_btn_tech_motor_approved(lv_event_t * e){
  if(send_new_switches_msg_box){
    lv_msgbox_close(send_new_switches_msg_box);
    send_new_switches_msg_box = NULL;
  }
  struct Return_unsaved_param unsave_motor_ths = check_unsave_motor_param();
  save_new_motor_val_to_struct(unsave_motor_ths);
  lv_obj_t* curr_msg_box = lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.\nProthesis is updated.",NULL, true);
  lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
}


void save_btn_tech_sensor_approved(lv_event_t * e){
  if(send_new_switches_msg_box){
    lv_msgbox_close(send_new_switches_msg_box);
    send_new_switches_msg_box = NULL;
  }
  struct Return_unsaved_param unsave_sensors = check_unsave_sensor_param();
  save_new_sensors_val_to_struct(unsave_sensors);

  lv_obj_t* curr_msg_box = lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.\nProthesis is updated.",NULL, true);
  lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
}



void save_btn_click_event(lv_event_t * e){
    std::vector<int> new_on_sensors = find_new_on_sensor();
    std::vector<int> new_off_sensors = find_new_off_sensor();
    if((new_on_sensors.size() == 0 ) && (new_off_sensors.size() == 0 )){
      lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,"Nothing to save...",NULL,NULL, true);
      lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
    }
    else{
      if(is_demo_yaml.test_and_set()){
        save_new_switch_btms_to_struct();
        lv_obj_clean(stat_tab);
        create_controls_for_stat(stat_tab);
        lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
      }
      else{
        is_demo_yaml.clear();
        not_finish_update_sensors.test_and_set();
        send_new_switches_msg_box = lv_msgbox_create(NULL,"Saving changes...","Sending new sensors states to prosthesis.",NULL, false);
        lv_obj_center(send_new_switches_msg_box);
        lv_refr_now(NULL);
        SendStatusChangeReq(new_on_sensors, new_off_sensors);  
        delay(1000);
        lv_refr_now(NULL);
        while(not_finish_update_sensors.test_and_set()){
          delay(200);
          if(!not_remove_box.test_and_set()){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
            break;
          }
        }
        if((!not_remove_box.test_and_set()) || (!not_finish_update_sensors.test_and_set()) ){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
        }
        else{
          lv_event_send(save_btn,EVENT_SENSOR_CHANGED_SECC ,NULL);  
        }
      }
    }
}

void save_btn_tech_motor_click_event(lv_event_t * e){
    struct Return_unsaved_param unsave_motor_ths = check_unsave_motor_param();

    if(unsave_motor_ths.params_id.size() == 0 ){

      lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,"Nothing to save...",NULL,NULL, true);
      lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
    }
    else{
      if(is_demo_yaml.test_and_set()){
        save_new_motor_val_to_struct(unsave_motor_ths);
        lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
      }
      else{
        is_demo_yaml.clear();
        not_finish_update_sensors.test_and_set();
        send_new_switches_msg_box = lv_msgbox_create(NULL,"Saving changes...","Sending new motor's safety threshold to prosthesis.",NULL, false);
        lv_obj_center(send_new_switches_msg_box);
        lv_refr_now(NULL);

        SendMotorParamChangeReq(unsave_motor_ths.sensor_id, unsave_motor_ths.new_vals);

        delay(1000);
        lv_refr_now(NULL);
        while(not_finish_update_sensors.test_and_set()){
          delay(200);
          if(!not_remove_box.test_and_set()){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
            break;
          }
        }
        if((!not_remove_box.test_and_set()) || (!not_finish_update_sensors.test_and_set()) ){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
        }
        else{
          lv_event_send(save_btn_tech_motors,EVENT_SENSOR_CHANGED_SECC ,NULL);  
        }
      }
    }
}

void save_btn_tech_sensor_click_event(lv_event_t * e){
    struct Return_unsaved_param unsave_sensors = check_unsave_sensor_param();

    if(unsave_sensors.params_id.size() == 0 ){
      lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,"Nothing to save...",NULL,NULL, true);
      lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
    }
    else{
      if(is_demo_yaml.test_and_set()){
        save_new_sensors_val_to_struct(unsave_sensors);
        lv_obj_t* curr_msg_box =lv_msgbox_create(NULL,LV_SYMBOL_OK,"Changes have been saved.",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
      }
      else{
        is_demo_yaml.clear();
        not_finish_update_sensors.test_and_set();
        send_new_switches_msg_box = lv_msgbox_create(NULL,"Saving changes...","Sending new sensor's parameters to prosthesis.",NULL, false);
        lv_obj_center(send_new_switches_msg_box);
        
        lv_refr_now(NULL);

        SendSensorParamChangeReq(unsave_sensors.sensor_id, unsave_sensors.params_id, unsave_sensors.new_vals);

        delay(1000);
        lv_refr_now(NULL);
        while(not_finish_update_sensors.test_and_set()){
          delay(200);
          if(!not_remove_box.test_and_set()){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
            break;
          }
        }
        if((!not_remove_box.test_and_set()) || (!not_finish_update_sensors.test_and_set()) ){
            if(send_new_switches_msg_box){
              lv_msgbox_close(send_new_switches_msg_box);
              send_new_switches_msg_box = NULL;
            }
        }
        else{
          lv_event_send(save_btn_tech_sensors,EVENT_SENSOR_CHANGED_SECC ,NULL);  
        }
      }
    }
}


void create_controls_for_setup(lv_obj_t* parent){
  // Create a title label
  lv_obj_t* title_label = lv_label_create(parent); 
  lv_label_set_text(title_label, "Sensors Setup:"); 
  lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 10); 
  lv_obj_set_style_text_font(title_label, &lv_font_montserrat_18, 0); 

  int y_offset = 40; // Vertical spacing between rows

  static lv_style_t style_indic_on;
  static lv_style_t style_indic_off;
  static lv_style_t style_knob_on;
  static lv_style_t style_knob_off;

  lv_style_init(&style_indic_on);
  lv_style_set_bg_color(&style_indic_on, HEX_DARK_BLUE);
  lv_style_set_bg_opa(&style_indic_on, LV_OPA_COVER);

  lv_style_init(&style_indic_off);
  lv_style_set_bg_color(&style_indic_off,  HEX_LIGHT_GRAY_2); 
  lv_style_set_bg_opa(&style_indic_off, LV_OPA_COVER);


  // Loop through each sensor
  for (const auto& sensor : sensors) { 
      // Replace "_" in the name with spaces
      String display_name = sensor.name;
      display_name.replace("_", " ");
      display_name[0] = toupper(display_name[0]);
      
      // Create a label for the sensor name
      lv_obj_t* sensor_name_label = lv_label_create(parent); 
      lv_label_set_text(sensor_name_label, (display_name + ":").c_str()); 
      lv_obj_align(sensor_name_label, LV_ALIGN_TOP_LEFT, 15, y_offset+5); 
      lv_obj_set_style_text_font(sensor_name_label, &lv_font_montserrat_16, 0); 

      // Create a status label
      lv_obj_t* sensor_switch = lv_switch_create(parent); 
      sensorSwitchVec.push_back(sensor_switch);
     
      lv_obj_align(sensor_switch, LV_ALIGN_TOP_RIGHT, -15, y_offset); 

      lv_obj_add_style(sensor_switch, &style_indic_on, LV_PART_INDICATOR | LV_STATE_CHECKED);
      lv_obj_add_style(sensor_switch, &style_indic_off, LV_PART_INDICATOR | LV_STATE_DEFAULT);

      if(sensor.status.equalsIgnoreCase("ON")){
        lv_obj_clear_state(sensor_switch, LV_STATE_DEFAULT);
        lv_obj_add_state(sensor_switch, LV_STATE_CHECKED);
      }
      else if(sensor.status.equalsIgnoreCase("OFF")){
        lv_obj_clear_state(sensor_switch, LV_STATE_CHECKED);
        lv_obj_add_state(sensor_switch, LV_STATE_DEFAULT);
      }
      else{
        Serial.printf("ERROR PLEASE CHECK");
      }
      y_offset += 37;  
  }
  save_btn = create_new_btn(parent, 90, 40, LV_ALIGN_TOP_MID, -50, y_offset+8, "Save", HEX_GREEN, HEX_WHITE, &lv_font_montserrat_16);
  lv_obj_t* discard_btn = create_new_btn(parent, 90, 40, LV_ALIGN_TOP_MID, 50, y_offset+8, "Discard", HEX_RED, HEX_WHITE, &lv_font_montserrat_16);
  lv_obj_add_event_cb(save_btn, save_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
  lv_obj_add_event_cb(save_btn, save_btn_approved, EVENT_SENSOR_CHANGED_SECC, NULL); // Set the event handler
  lv_obj_add_event_cb(discard_btn, discard_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
}


int num_points = 80; // Number of points in the chart
static lv_obj_t *show_chart_btn;


// Function to generate dummy sensor data (Replace with real sensor function later)
static int get_sensor_value() {
    return lv_rand(10, 90); // Simulated sensor value between 10 and 90
}

static void update_chart_req(lv_timer_t *t) {
  int* arr = static_cast<int*>(t->user_data);
  char* msg_to_send=(char*)malloc(MAX_MSG_LEN);
  int pos = 0;
  String is_motor_str =String(arr[0]);
  strcpy(&(msg_to_send[pos]), is_motor_str.c_str());
  pos+=is_motor_str.length();
  strcpy(&msg_to_send[pos++],"|");
  String hardware_id_str =String(arr[1]);
  strcpy(&(msg_to_send[pos]), hardware_id_str.c_str());
  SendNotifyToClient(msg_to_send, READ_REQ, pCharacteristic);
  free(msg_to_send);
}



// Timer callback to update the chart
static void update_chart(lv_timer_t *t) {
    lv_chart_set_next_value(chart, ser, get_sensor_value());

    // Create a gap by setting the next few points to LV_CHART_POINT_NONE
    uint16_t p = lv_chart_get_point_count(chart);
    uint16_t s = lv_chart_get_x_start_point(chart, ser);
    lv_coord_t *a = lv_chart_get_y_array(chart, ser);

    a[(s + 1) % p] = LV_CHART_POINT_NONE;
    a[(s + 2) % p] = LV_CHART_POINT_NONE;
    a[(s + 3) % p] = LV_CHART_POINT_NONE;
    a[(s + 4) % p] = LV_CHART_POINT_NONE;
    a[(s + 5) % p] = LV_CHART_POINT_NONE;
    a[(s + 6) % p] = LV_CHART_POINT_NONE;
    a[(s + 7) % p] = LV_CHART_POINT_NONE;
    a[(s + 8) % p] = LV_CHART_POINT_NONE;
    a[(s + 9) % p] = LV_CHART_POINT_NONE;

    lv_chart_refresh(chart);
}

// Button event callback to show the chart
static void show_chart_event_cb(bool is_motor, int id) {

    // Hide elements that we dont want right now
    lv_obj_add_flag(dropdown_motors_bug, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(dropdown_sensors_bug, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(debug_tabview, LV_OBJ_FLAG_HIDDEN);

    // Create the chart
    chart = lv_chart_create(debug_tab);

    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_CIRCULAR);
    lv_obj_set_size(chart, 280, 125);
    lv_obj_align(chart, LV_ALIGN_BOTTOM_MID, 0, -30);

    // Set chart parameters
    lv_chart_set_point_count(chart, num_points);
    ser = lv_chart_add_series(chart, HEX_DARK_BLUE, LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);    
    static int arr[2];  // Declare static array without initialization
    arr[0] = (int)is_motor;  // Update dynamically
    arr[1] = id;

    if(is_demo_yaml.test_and_set()){
      if (chart_timer) {
        lv_timer_del(chart_timer); // Stop the timer
        chart_timer = NULL;
      }
      chart_timer = lv_timer_create(update_chart, 200, static_cast<void*>(arr)); // update_chart without BLE

    }else{
      is_demo_yaml.clear();
      chart_timer = lv_timer_create(update_chart_req, 200, static_cast<void*>(arr)); // update_chart_req - request
    }

    // Create the "Close Chart" button
    close_chart_btn = lv_btn_create(debug_tab);
    lv_obj_set_size(close_chart_btn, 100, 25);
    lv_obj_align(close_chart_btn, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_set_style_bg_color(close_chart_btn, HEX_DARK_BLUE, 0);


    lv_obj_t * label = lv_label_create(close_chart_btn);
    lv_label_set_text(label, "Close Chart");
    lv_obj_center(label);
    lv_obj_set_style_text_font(label,&lv_font_montserrat_12,0);

    lv_obj_add_event_cb(close_chart_btn, close_chart_event_cb, LV_EVENT_CLICKED, NULL);

    // Create a title label
    title_label_bug = lv_label_create(debug_tab); 
    lv_label_set_text(title_label_bug, selected_text_to_title);
    lv_obj_align(title_label_bug, LV_ALIGN_TOP_MID, 0, 0); 
    lv_obj_set_style_text_font(title_label_bug, &lv_font_montserrat_18, 0);
}
// Button event callback to close the chart
static void close_chart_event_cb(lv_event_t * e) {
  if (chart_timer) {
    lv_timer_del(chart_timer); // Stop the timer
    chart_timer = NULL;
  }
  
  delay(200); // To make sure that the last message was handled properly

  if (chart) {// Delete the chart
    lv_obj_del(chart);
    chart = NULL;
  }
  if (close_chart_btn) {// Delete the close button
    lv_obj_del(close_chart_btn);
    close_chart_btn = NULL;
  }
  ser = NULL;
  if(title_label_bug){
    lv_obj_del(title_label_bug);
    title_label_bug = NULL;
  }
  
  // Show back the elements  we hid
  lv_obj_clear_flag(dropdown_motors_bug, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(dropdown_sensors_bug, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(debug_tabview, LV_OBJ_FLAG_HIDDEN);
}


static void dropdown_event_debug_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    char selected_text[32]; // Buffer for selected item
    lv_dropdown_get_selected_str(dropdown, selected_text, sizeof(selected_text));
    strcpy(selected_text_to_title,selected_text);
    
    String selected_text_str = String(selected_text);

    // Determine if this is the motors dropdown or sensors dropdown
    bool is_motor = (dropdown == dropdown_motors_bug);

    int id = 0;

    if (is_motor) {
        for (const auto& motor : motors) { 
            if (motor.name == selected_text_str) {
                break;
            }
            id++;
        }
        if (id == motors.size()){
          return;
        }
    } else {
        for (const auto& sensor : sensors) {
            if (sensor.name == selected_text_str) {
                break;
            }
            id++;
        }
        if (id == sensors.size()){
          return;
        }
    }    
    show_chart_event_cb(is_motor, id);
}

char* get_options_string(bool is_motors){
  int totalLength  = 0;
  if(is_motors){

    for (const auto& motor : motors) {
      totalLength += motor.name.length() + 1;

    }
    if (totalLength == 0) return nullptr; // No motors, return nullptr
    // Allocate memory for the total length + 1 for the null terminator
    char* result = (char*) malloc(totalLength + 1);
    if (result == nullptr) return nullptr; // Failed to allocate memory

    // Concatenate all names into the allocated space
    char* currentPos = result;
    for (const auto& motor : motors) {
        strcpy(currentPos, motor.name.c_str()); // Copy the motor name
        currentPos += motor.name.length();      // Move the pointer
        *currentPos = '\n';                     // Add a newline character
        currentPos++;                           // Move past the newline
    }
    *(currentPos - 1) = '\0'; // Replace the last newline with a null terminator
    return result;
    
  }
  else{
    for (const auto& sensor : sensors) {
      totalLength += sensor.name.length() + 1;

    }
    if (totalLength == 0) return nullptr; // No motors, return nullptr
    // Allocate memory for the total length + 1 for the null terminator
    char* result = (char*) malloc(totalLength + 1);
    if (result == nullptr) return nullptr; // Failed to allocate memory

    // Concatenate all names into the allocated space
    char* currentPos = result;
    for (const auto& sensor : sensors) {
        strcpy(currentPos, sensor.name.c_str()); // Copy the motor name
        currentPos += sensor.name.length();      // Move the pointer
        *currentPos = '\n';                     // Add a newline character
        currentPos++;                           // Move past the newline
    }
    *(currentPos - 1) = '\0'; // Replace the last newline with a null terminator
        
    return result;
  }
}

lv_obj_t* create_dropdown_debug(lv_obj_t* parent, bool is_motors){
  lv_obj_t * dropdown = lv_dropdown_create(parent);
  lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, -10, -10);
  char* options = get_options_string(is_motors);
  if(is_motors){
    lv_dropdown_set_text(dropdown, "Motor");
  } else{
    lv_dropdown_set_text(dropdown, "Sensor");
  }
  lv_dropdown_set_options(dropdown,options);
  lv_dropdown_set_selected_highlight(dropdown, true);
  lv_obj_add_event_cb(dropdown, dropdown_event_debug_cb, LV_EVENT_VALUE_CHANGED, NULL); // Attach event
  if (options){
    free(options);
  }
  return dropdown;
}


void create_controls_for_debug(lv_obj_t* parent) {
  // Tabview
    debug_tabview = lv_tabview_create(parent, LV_DIR_RIGHT, 70);
    lv_obj_set_style_bg_color(debug_tabview,HEX_MEDIUM_GRAY,0);
    lv_obj_set_style_bg_opa(debug_tabview, LV_OPA_COVER, 0);
    
    lv_obj_t* debug_tab_btns = lv_tabview_get_tab_btns(debug_tabview);
    lv_obj_set_style_bg_color(debug_tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(debug_tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(debug_tab_btns, LV_BORDER_SIDE_LEFT, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_set_style_bg_opa(debug_tab_btns, LV_OPA_COVER, 0);        
    lv_obj_t * debug_tab_sensors = lv_tabview_add_tab(debug_tabview, "Sensors");
    lv_obj_t * debug_tab_motors = lv_tabview_add_tab(debug_tabview, "Motors");

    lv_obj_clear_flag(lv_tabview_get_content(debug_tabview), LV_OBJ_FLAG_SCROLLABLE);

    dropdown_motors_bug = create_dropdown_debug(debug_tab_motors, true);
    dropdown_sensors_bug =create_dropdown_debug(debug_tab_sensors, false);
    
}


  
lv_obj_t* create_motors_tech(lv_obj_t* parent){
  lv_obj_t * dropdown = lv_dropdown_create(parent);
  lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, -10, -10);
  char* options = get_options_string(true);
  lv_dropdown_set_text(dropdown, "Motor");
  lv_dropdown_set_options(dropdown,options);
  lv_dropdown_set_selected_highlight(dropdown, true);
  if (options){
    free(options);
  }
  return dropdown;
}

lv_obj_t* create_sensors_tech(lv_obj_t* parent){
  lv_obj_t * dropdown = lv_dropdown_create(parent);
  lv_obj_align(dropdown, LV_ALIGN_TOP_LEFT, -10, -10);
  char* options = get_options_string(false);
  lv_dropdown_set_options(dropdown,options);
  lv_dropdown_set_text(dropdown, "Sensor");
  lv_dropdown_set_selected_highlight(dropdown, true);


  if (options){
    free(options);
  }
  return dropdown;
}


void slider_event_cb_anim(lv_event_t* e){
  lv_obj_t* val_label = (lv_obj_t*)lv_event_get_user_data(e);
  lv_obj_t * slider = lv_event_get_target(e);
  lv_label_set_text(val_label, String(lv_slider_get_value(slider)).c_str());
}

void slider_event_cb_updated_val(lv_event_t* e){
  lv_obj_t * slider = lv_event_get_target(e);
  // Serial.printf("sliders for sensor %d: \n", current_edit_sensor_id);
  for( int i = 0; i < current_edit_sensor_sliders_vec.size(); i++){
    // Serial.printf("%d ",lv_slider_get_value(current_edit_sensor_sliders_vec[i]));
  }  
  // Serial.printf("\n");
}


void slider_event_cb_updated_val_motor(lv_event_t* e){
  lv_obj_t * slider = lv_event_get_target(e);
  // Serial.printf("sliders for motor %d: \n", current_edit_motor_id);
  for( int i = 0; i < current_edit_motor_sliders_vec.size(); i++){
    // Serial.printf("%d ",lv_slider_get_value(current_edit_motor_sliders_vec[i]));
  }  
  // Serial.printf("\n");
}


struct Return_unsaved_param check_unsave_sensor_param(){
  struct Return_unsaved_param ret_struct;
  std::vector<int> ret_params_id;
  std::vector<int> ret_new_vals;
  ret_struct.sensor_id = current_edit_sensor_id;
  int i = 0;
  if(current_edit_sensor_sliders_vec.size() > 0){
    for (const auto& [paramName, param] : sensors[current_edit_sensor_id].function.parameters){
      if(param.current_val != lv_slider_get_value(current_edit_sensor_sliders_vec[i])){
        ret_params_id.push_back(i);
        ret_new_vals.push_back(lv_slider_get_value(current_edit_sensor_sliders_vec[i]));
      }
      i++;
    }
  }

  ret_struct.params_id = ret_params_id;
  ret_struct.new_vals = ret_new_vals;

  return ret_struct;
}


struct Return_unsaved_param check_unsave_motor_param(){
  struct Return_unsaved_param ret_struct;
  std::vector<int> ret_params_id;
  std::vector<int> ret_new_vals;
  ret_struct.sensor_id = current_edit_motor_id;
  int i = 0;
  if(current_edit_motor_sliders_vec.size() > 0){
    if(motors[current_edit_motor_id].safety_threshold.current_val != lv_slider_get_value(current_edit_motor_sliders_vec[i])){
      ret_params_id.push_back(i);
      ret_new_vals.push_back(lv_slider_get_value(current_edit_motor_sliders_vec[i]));
    }
    i++;
  }
  ret_struct.params_id = ret_params_id;
  ret_struct.new_vals = ret_new_vals;

  return ret_struct;
}
static void set_value(void * indic, int32_t v)
{
    lv_meter_set_indicator_value(meter, (lv_meter_indicator_t*)indic, v);
}

void motors_activity(lv_obj_t* parent){

    meter = lv_meter_create(parent);
    lv_obj_align(meter, LV_ALIGN_BOTTOM_MID, 0, 5);
    lv_obj_set_size(meter, 130, 130);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 41, 2, 10, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 8, 4, 15, lv_color_black(), 10);

    lv_meter_indicator_t * indic;

    /*Add a blue arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, HEX_DARK_BLUE, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines blue at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, HEX_DARK_BLUE, HEX_DARK_BLUE,false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, HEX_RED, 0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, HEX_RED, HEX_RED, false,0);
    lv_meter_set_indicator_start_value(meter, indic, 80);
    lv_meter_set_indicator_end_value(meter, indic, 100);

    /*Add a needle line indicator*/
    indic = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -10);

    /*Create an animation to set the value*/
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_exec_cb(&a, set_value);
    lv_anim_set_var(&a, indic);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_repeat_delay(&a, 100);
    lv_anim_set_playback_time(&a, 500);
    lv_anim_set_playback_delay(&a, 100);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);
}



void event_view_and_edit_motors(lv_event_t* e){
  lv_obj_t* parent = (lv_obj_t*)lv_event_get_user_data(e);
  lv_obj_t * obj = lv_event_get_target(e);
  int id = -1;
  if(obj){
    id = lv_dropdown_get_selected(obj);
  }
  Motor& motor = motors[id];
  if( ((current_edit_motor_id != -1) && (current_edit_motor_id != id)) || motor_trigged_by_tech_tab ){

    if(check_unsave_motor_param().params_id.size() != 0 ){
        lv_dropdown_set_selected(obj,current_edit_motor_id);
        if(motor_trigged_by_tech_tab) {
          lv_tabview_set_act(tech_tabview,0,LV_ANIM_ON);
        }
        lv_obj_t* curr_msg_box = lv_msgbox_create(NULL, "Changes were not saved", "Please choose Save/Discard",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
        motor_trigged_by_tech_tab = false;


    }
    else {
      if(motor_trigged_by_tech_tab) {
        motor_trigged_by_tech_tab = false;
        return;
      }    
      current_edit_motor_id = id;

      // delete_all_children_except(parent, obj);
      for( auto& obj_td: obj_to_delete_motors){
        lv_obj_del(obj_td);
      }
      obj_to_delete_motors.clear();
      current_edit_motor_sliders_vec.clear();

      String param_info = "Safety_threshold:\n";
      int y_offset = 45;  // Initial Y position for UI elements


      // Update the label for parameters
      lv_obj_t * label_motor_params = lv_label_create(parent);
      lv_obj_set_width(label_motor_params, 180); // Set width for better display
      lv_obj_align(label_motor_params, LV_ALIGN_TOP_LEFT, -10, y_offset);
      obj_to_delete_motors.push_back(label_motor_params);
      lv_label_set_text(label_motor_params, param_info.c_str());

      y_offset += 25;
      // Iterate over all parameters
      const auto& param = motor.safety_threshold;

      lv_obj_t* slider = lv_slider_create(parent);
      lv_slider_set_range(slider, param.min, param.max);
      lv_slider_set_value(slider, param.current_val, LV_ANIM_OFF);
      lv_obj_add_flag(slider, LV_OBJ_FLAG_ADV_HITTEST);
      lv_obj_set_size(slider, 140, 8);
      lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 10, y_offset);
      obj_to_delete_motors.push_back(slider);

      lv_obj_set_ext_click_area(slider, 10);
      static lv_style_t style_disable;
      lv_style_init(&style_disable);
      lv_style_set_bg_color(&style_disable, HEX_DARK_GRAY);
      lv_style_set_line_color(&style_disable, HEX_DARK_GRAY);
      lv_obj_add_style(slider, &style_disable, LV_PART_INDICATOR | LV_STATE_DISABLED);
      lv_obj_add_style(slider, &style_disable, LV_PART_KNOB | LV_STATE_DISABLED);
      lv_obj_add_style(slider, &style_disable, LV_PART_MAIN | LV_STATE_DISABLED);
      lv_obj_set_style_border_color(slider, HEX_BLACK,LV_PART_INDICATOR);


      lv_obj_t* min_value_label = lv_label_create(parent);
      lv_label_set_text_fmt(min_value_label, "%d", param.min);
      lv_obj_align(min_value_label, LV_ALIGN_TOP_LEFT, -10, y_offset-5);
      lv_obj_set_style_text_font(min_value_label,&lv_font_montserrat_12,0);
      obj_to_delete_motors.push_back(min_value_label);

      lv_obj_t* max_value_label = lv_label_create(parent);
      lv_label_set_text_fmt(max_value_label, "%d", param.max);
      lv_obj_align(max_value_label, LV_ALIGN_TOP_LEFT, 155, y_offset-5);
      lv_obj_set_style_text_font(max_value_label,&lv_font_montserrat_12,0);
      obj_to_delete_motors.push_back(max_value_label);


      y_offset += 15;
      // Create a label to display the current value
      lv_obj_t* value_label = lv_label_create(parent);
      lv_label_set_text_fmt(value_label, "%d", param.current_val);
      lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 75,y_offset);
      obj_to_delete_motors.push_back(value_label);
      current_edit_motor_sliders_vec.push_back(slider);

      lv_obj_add_event_cb(slider, slider_event_cb_anim, LV_EVENT_VALUE_CHANGED, value_label);
      lv_obj_add_event_cb(slider, slider_event_cb_updated_val_motor, LV_EVENT_RELEASED, NULL);


      y_offset += 20;
    
      lv_obj_set_user_data(slider, value_label);

      // If parameter is not editable, disable the slider
      if (!param.modify_permission) {
          lv_obj_add_state(slider, LV_STATE_DISABLED);
      }
      
      save_btn_tech_motors = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, -50, y_offset+8, "Save", HEX_GREEN, HEX_WHITE, &lv_font_montserrat_14);
      obj_to_delete_motors.push_back(save_btn_tech_motors);
      lv_obj_t* discard_btn = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, 30, y_offset+8, "Discard", HEX_RED, HEX_WHITE, &lv_font_montserrat_14);
      obj_to_delete_motors.push_back(discard_btn);


      y_offset +=50;


      lv_obj_t * label_motor_name = lv_label_create(parent);
      lv_obj_set_width(label_motor_name, 180); // Set width for better display
      lv_obj_align(label_motor_name, LV_ALIGN_TOP_LEFT, -10, y_offset);
      lv_obj_set_style_text_font(label_motor_name,&lv_font_montserrat_12,0);
      obj_to_delete_motors.push_back(label_motor_name);
      y_offset += 20;

      lv_obj_t * label_motor_type = lv_label_create(parent);
      lv_obj_set_width(label_motor_type, 180); // Set width for better display
      lv_obj_align(label_motor_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
      lv_obj_set_style_text_font(label_motor_type,&lv_font_montserrat_12,0);
      obj_to_delete_motors.push_back(label_motor_type);
      y_offset += 25;

      lv_label_set_text_fmt(label_motor_name, "Name: %s", motor.name.c_str());
      lv_label_set_text_fmt(label_motor_type, "Type: %s", motor.type.c_str());

      lv_obj_t * label_motor_PIN = lv_label_create(parent);
      lv_obj_set_width(label_motor_PIN, 180); // Set width for better display
      lv_obj_align(label_motor_PIN, LV_ALIGN_TOP_LEFT, -10, y_offset);
      lv_obj_set_style_text_font(label_motor_PIN,&lv_font_montserrat_14,0);
      lv_label_set_text(label_motor_PIN, "Pins:");
      obj_to_delete_motors.push_back(label_motor_PIN);

      y_offset += 20;
      // Iterate over all parameters
      int i = 0;
      for (const auto& param: motor.pins) {
            lv_obj_t * label_pin_type = lv_label_create(parent);
            lv_obj_set_width(label_pin_type, 180); // Set width for better display
            lv_obj_align(label_pin_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
            lv_obj_set_style_text_font(label_pin_type, &lv_font_montserrat_12, 0);
            obj_to_delete_motors.push_back(label_pin_type);
            lv_label_set_text_fmt(label_pin_type, "Pin Type: %s", param.type.c_str());
            y_offset += 20; 

            lv_obj_t * label_pin_number = lv_label_create(parent);
            lv_obj_set_width(label_pin_number, 180);
            lv_obj_align(label_pin_number, LV_ALIGN_TOP_LEFT, -10, y_offset);
            lv_obj_set_style_text_font(label_pin_number, &lv_font_montserrat_12, 0);
            obj_to_delete_motors.push_back(label_pin_number);
            lv_label_set_text_fmt(label_pin_number, "Pin Number: %d", param.pin_number);
            y_offset += 20; 
      }

      lv_obj_add_event_cb(save_btn_tech_motors, save_btn_tech_motor_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
      lv_obj_add_event_cb(save_btn_tech_motors, save_btn_tech_motor_approved, EVENT_SENSOR_CHANGED_SECC, NULL); // Set the event handler
      lv_obj_add_event_cb(discard_btn, discard_motor_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
      
    }
  }
  else if (current_edit_motor_id == id){
    return;
  }
  else {
    if(motor_trigged_by_tech_tab) {
      motor_trigged_by_tech_tab = false;
      return;
    }    
    current_edit_motor_id = id;

    for( auto& obj_td: obj_to_delete_motors){
      lv_obj_del(obj_td);
    }
    obj_to_delete_motors.clear();
    current_edit_motor_sliders_vec.clear();

    String param_info = "Safety_threshold:\n";
    int y_offset = 45;  // Initial Y position for UI elements


    // Update the label for parameters
    lv_obj_t * label_motor_params = lv_label_create(parent);
    lv_obj_set_width(label_motor_params, 180); // Set width for better display
    lv_obj_align(label_motor_params, LV_ALIGN_TOP_LEFT, -10, y_offset);
    obj_to_delete_motors.push_back(label_motor_params);
    lv_label_set_text(label_motor_params, param_info.c_str());

    y_offset += 25;
    // Iterate over all parameters
    const auto& param = motor.safety_threshold;

    lv_obj_t* slider = lv_slider_create(parent);
    lv_slider_set_range(slider, param.min, param.max);
    lv_slider_set_value(slider, param.current_val, LV_ANIM_OFF);
    lv_obj_add_flag(slider, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_set_size(slider, 140, 8);
    lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 10, y_offset);
    obj_to_delete_motors.push_back(slider);

    lv_obj_set_ext_click_area(slider, 10);
    static lv_style_t style_disable;
    lv_style_init(&style_disable);
    lv_style_set_bg_color(&style_disable, HEX_DARK_GRAY);
    lv_style_set_line_color(&style_disable, HEX_DARK_GRAY);
    lv_obj_add_style(slider, &style_disable, LV_PART_INDICATOR | LV_STATE_DISABLED);
    lv_obj_add_style(slider, &style_disable, LV_PART_KNOB | LV_STATE_DISABLED);
    lv_obj_add_style(slider, &style_disable, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_set_style_border_color(slider, HEX_BLACK,LV_PART_INDICATOR);

    lv_obj_t* min_value_label = lv_label_create(parent);
    lv_label_set_text_fmt(min_value_label, "%d", param.min);
    lv_obj_align(min_value_label, LV_ALIGN_TOP_LEFT, -10, y_offset-5);
    lv_obj_set_style_text_font(min_value_label,&lv_font_montserrat_12,0);
    obj_to_delete_motors.push_back(min_value_label);

    lv_obj_t* max_value_label = lv_label_create(parent);
    lv_label_set_text_fmt(max_value_label, "%d", param.max);
    lv_obj_align(max_value_label, LV_ALIGN_TOP_LEFT, 155, y_offset-5);
    lv_obj_set_style_text_font(max_value_label,&lv_font_montserrat_12,0);
    obj_to_delete_motors.push_back(max_value_label);


    y_offset += 15;
    // Create a label to display the current value
    lv_obj_t* value_label = lv_label_create(parent);
    lv_label_set_text_fmt(value_label, "%d", param.current_val);
    lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 75,y_offset);
    obj_to_delete_motors.push_back(value_label);
    current_edit_motor_sliders_vec.push_back(slider);

    lv_obj_add_event_cb(slider, slider_event_cb_anim, LV_EVENT_VALUE_CHANGED, value_label);
    lv_obj_add_event_cb(slider, slider_event_cb_updated_val_motor, LV_EVENT_RELEASED, NULL);


    y_offset += 20;
  
    lv_obj_set_user_data(slider, value_label);

    // If parameter is not editable, disable the slider
    if (!param.modify_permission) {
        lv_obj_add_state(slider, LV_STATE_DISABLED);
    }
    
    save_btn_tech_motors = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, -50, y_offset+8, "Save", HEX_GREEN, HEX_WHITE, &lv_font_montserrat_14);
    obj_to_delete_motors.push_back(save_btn_tech_motors);
    lv_obj_t* discard_btn = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, 30, y_offset+8, "Discard", HEX_RED, HEX_WHITE, &lv_font_montserrat_14);
    obj_to_delete_motors.push_back(discard_btn);


    y_offset +=50;


    lv_obj_t * label_motor_name = lv_label_create(parent);
    lv_obj_set_width(label_motor_name, 180); // Set width for better display
    lv_obj_align(label_motor_name, LV_ALIGN_TOP_LEFT, -10, y_offset);
    lv_obj_set_style_text_font(label_motor_name,&lv_font_montserrat_12,0);
    obj_to_delete_motors.push_back(label_motor_name);
    y_offset += 20;

    lv_obj_t * label_motor_type = lv_label_create(parent);
    lv_obj_set_width(label_motor_type, 180); // Set width for better display
    lv_obj_align(label_motor_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
    lv_obj_set_style_text_font(label_motor_type,&lv_font_montserrat_12,0);
    obj_to_delete_motors.push_back(label_motor_type);
    y_offset += 25;

    lv_label_set_text_fmt(label_motor_name, "Name: %s", motor.name.c_str());
    lv_label_set_text_fmt(label_motor_type, "Type: %s", motor.type.c_str());

    lv_obj_t * label_motor_PIN = lv_label_create(parent);
    lv_obj_set_width(label_motor_PIN, 180); // Set width for better display
    lv_obj_align(label_motor_PIN, LV_ALIGN_TOP_LEFT, -10, y_offset);
    lv_obj_set_style_text_font(label_motor_PIN,&lv_font_montserrat_14,0);
    lv_label_set_text(label_motor_PIN, "Pins:");
    obj_to_delete_motors.push_back(label_motor_PIN);

    y_offset += 20;
    // Iterate over all parameters
    int i = 0;
    for (const auto& param: motor.pins) {
          lv_obj_t * label_pin_type = lv_label_create(parent);
          lv_obj_set_width(label_pin_type, 180); // Set width for better display
          lv_obj_align(label_pin_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
          lv_obj_set_style_text_font(label_pin_type, &lv_font_montserrat_12, 0);
          obj_to_delete_motors.push_back(label_pin_type);
          lv_label_set_text_fmt(label_pin_type, "Pin Type: %s", param.type.c_str());
          y_offset += 20; 

          lv_obj_t * label_pin_number = lv_label_create(parent);
          lv_obj_set_width(label_pin_number, 180);
          lv_obj_align(label_pin_number, LV_ALIGN_TOP_LEFT, -10, y_offset);
          lv_obj_set_style_text_font(label_pin_number, &lv_font_montserrat_12, 0);
          obj_to_delete_motors.push_back(label_pin_number);
          lv_label_set_text_fmt(label_pin_number, "Pin Number: %d", param.pin_number);
          y_offset += 20; 
    }
    lv_obj_add_event_cb(save_btn_tech_motors, save_btn_tech_motor_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
    lv_obj_add_event_cb(save_btn_tech_motors, save_btn_tech_motor_approved, EVENT_SENSOR_CHANGED_SECC, NULL); // Set the event handler
    lv_obj_add_event_cb(discard_btn, discard_motor_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
    
  }
  
}

void event_view_and_edit_sensors(lv_event_t* e){
  lv_obj_t* parent = (lv_obj_t*)lv_event_get_user_data(e);
  lv_obj_t * obj = lv_event_get_target(e);
  int id = -1;
  if(obj){
    id = lv_dropdown_get_selected(obj);
  }
  Sensor& sensor = sensors[id];

  if( ((current_edit_sensor_id != -1) && (current_edit_sensor_id != id)) || sensor_trigged_by_tech_tab ){

    if(check_unsave_sensor_param().params_id.size() != 0 ){
        lv_dropdown_set_selected(obj,current_edit_sensor_id);
        if(sensor_trigged_by_tech_tab) {
          lv_tabview_set_act(tech_tabview,1,LV_ANIM_ON);
        }
        lv_obj_t* curr_msg_box = lv_msgbox_create(NULL, "Changes were not saved", "Please choose Save/Discard",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
        sensor_trigged_by_tech_tab = false;


    }
    else {
      if(sensor_trigged_by_tech_tab) {
        sensor_trigged_by_tech_tab = false;
        return;
      }
      else{
        current_edit_sensor_id = id;
        for( auto& obj_td: obj_to_delete_sensors){
          lv_obj_del(obj_td);
        }
        obj_to_delete_sensors.clear();
        current_edit_sensor_sliders_vec.clear();
        String param_info = "Parameters:\n";
        int y_offset = 30;  // Initial Y position for UI elements

        // Update the label for parameters
        lv_obj_t * label_sensor_params = lv_label_create(parent);
        lv_obj_set_width(label_sensor_params, 180); // Set width for better display
        lv_obj_align(label_sensor_params, LV_ALIGN_TOP_LEFT, -10, y_offset);
        obj_to_delete_sensors.push_back(label_sensor_params);
        lv_label_set_text(label_sensor_params, param_info.c_str());

        y_offset += 20;
        // Iterate over all parameters
        int i = 0;
        for (const auto& param : sensor.function.parameters) {

            // Create a label for parameter name
            lv_obj_t* param_label = lv_label_create(parent);
            lv_label_set_text_fmt(param_label, "%s:", param.first.c_str());
            lv_obj_align(param_label, LV_ALIGN_TOP_LEFT, -10, y_offset);
            lv_obj_set_style_text_font(param_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(param_label);

            y_offset += 20;
            lv_obj_t* slider = lv_slider_create(parent);
            lv_slider_set_range(slider, param.second.min, param.second.max);
            lv_slider_set_value(slider, param.second.current_val, LV_ANIM_OFF);
            lv_obj_add_flag(slider, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_set_size(slider, 140, 8);
            lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 10, y_offset);
            obj_to_delete_sensors.push_back(slider);

            lv_obj_set_ext_click_area(slider, 10);
            static lv_style_t style_disable;
            lv_style_init(&style_disable);
            lv_style_set_bg_color(&style_disable, HEX_DARK_GRAY);
            lv_style_set_line_color(&style_disable, HEX_DARK_GRAY);
            lv_obj_add_style(slider, &style_disable, LV_PART_INDICATOR | LV_STATE_DISABLED);
            lv_obj_add_style(slider, &style_disable, LV_PART_KNOB | LV_STATE_DISABLED);
            lv_obj_add_style(slider, &style_disable, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_border_color(slider, HEX_BLACK,LV_PART_INDICATOR);

            lv_obj_t* min_value_label = lv_label_create(parent);
            lv_label_set_text_fmt(min_value_label, "%d", param.second.min);
            lv_obj_align(min_value_label, LV_ALIGN_TOP_LEFT, -10, y_offset-5);
            lv_obj_set_style_text_font(min_value_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(min_value_label);

            lv_obj_t* max_value_label = lv_label_create(parent);
            lv_label_set_text_fmt(max_value_label, "%d", param.second.max);
            lv_obj_align(max_value_label, LV_ALIGN_TOP_LEFT, 155, y_offset-5);
            lv_obj_set_style_text_font(max_value_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(max_value_label);


            y_offset += 10;
            // Create a label to display the current value
            lv_obj_t* value_label = lv_label_create(parent);
            lv_label_set_text_fmt(value_label, "%d", param.second.current_val);
            lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 75,y_offset);
            obj_to_delete_sensors.push_back(value_label);
            current_edit_sensor_sliders_vec.push_back(slider);

            lv_obj_add_event_cb(slider, slider_event_cb_anim, LV_EVENT_VALUE_CHANGED, value_label);
            lv_obj_add_event_cb(slider, slider_event_cb_updated_val, LV_EVENT_RELEASED, NULL);


            y_offset += 20;
          
            lv_obj_set_user_data(slider, value_label);

            // If parameter is not editable, disable the slider
            if (!param.second.modify_permission) {
                lv_obj_add_state(slider, LV_STATE_DISABLED);
            } 
            i++;  
        }

        save_btn_tech_sensors = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, -50, y_offset+8, "Save", HEX_GREEN, HEX_WHITE, &lv_font_montserrat_14);
        obj_to_delete_sensors.push_back(save_btn_tech_sensors);
        lv_obj_t* discard_btn = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, 30, y_offset+8, "Discard", HEX_RED, HEX_WHITE, &lv_font_montserrat_14);
        obj_to_delete_sensors.push_back(discard_btn);

        y_offset += 50;

        // Create labels for sensor details
        lv_obj_t * label_sensor_name = lv_label_create(parent);
        lv_obj_set_width(label_sensor_name, 180); // Set width for better display
        lv_obj_align(label_sensor_name, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_name,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_name);
        y_offset += 20;



        lv_obj_t * label_sensor_status = lv_label_create(parent);
        lv_obj_set_width(label_sensor_status, 180); // Set width for better display
        lv_obj_align(label_sensor_status, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_status,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_status);
        y_offset += 20;


        lv_obj_t * label_sensor_type = lv_label_create(parent);
        lv_obj_set_width(label_sensor_type, 180); // Set width for better display
        lv_obj_align(label_sensor_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_type,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_type);
        y_offset += 20;

        lv_label_set_text_fmt(label_sensor_name, "Name: %s", sensor.name.c_str());
        lv_label_set_text_fmt(label_sensor_status, "Status: %s", sensor.status.c_str());
        lv_label_set_text_fmt(label_sensor_type, "Type: %s", sensor.type.c_str());

        lv_obj_add_event_cb(save_btn_tech_sensors, save_btn_tech_sensor_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
        lv_obj_add_event_cb(save_btn_tech_sensors, save_btn_tech_sensor_approved, EVENT_SENSOR_CHANGED_SECC, NULL); // Set the event handler
        lv_obj_add_event_cb(discard_btn, discard_sensor_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
      }    
    }

  }
  else if (current_edit_sensor_id == id){
    return;
  }
  else{
        current_edit_sensor_id = id;

        for( auto& obj_td: obj_to_delete_sensors){
          lv_obj_del(obj_td);
        }
        obj_to_delete_sensors.clear();
        current_edit_sensor_sliders_vec.clear();

        // Prepare formatted parameter information
        String param_info = "Parameters:\n";
        int y_offset = 30;  // Initial Y position for UI elements

        // Update the label for parameters
        lv_obj_t * label_sensor_params = lv_label_create(parent);
        lv_obj_set_width(label_sensor_params, 180); // Set width for better display
        lv_obj_align(label_sensor_params, LV_ALIGN_TOP_LEFT, -10, y_offset);
        obj_to_delete_sensors.push_back(label_sensor_params);
        lv_label_set_text(label_sensor_params, param_info.c_str());

        y_offset += 20;
        // Iterate over all parameters
        int i = 0;
        for (const auto& param : sensor.function.parameters) {

            // Create a label for parameter name
            lv_obj_t* param_label = lv_label_create(parent);
            lv_label_set_text_fmt(param_label, "%s:", param.first.c_str());
            lv_obj_align(param_label, LV_ALIGN_TOP_LEFT, -10, y_offset);
            lv_obj_set_style_text_font(param_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(param_label);

            y_offset += 20;
            lv_obj_t* slider = lv_slider_create(parent);
            lv_slider_set_range(slider, param.second.min, param.second.max);
            lv_slider_set_value(slider, param.second.current_val, LV_ANIM_OFF);
            lv_obj_add_flag(slider, LV_OBJ_FLAG_ADV_HITTEST);
            lv_obj_set_size(slider, 140, 8);
            lv_obj_align(slider, LV_ALIGN_TOP_LEFT, 10, y_offset);
            obj_to_delete_sensors.push_back(slider);
      
            lv_obj_set_ext_click_area(slider, 10);
            static lv_style_t style_disable;
            lv_style_init(&style_disable);
            lv_style_set_bg_color(&style_disable, HEX_DARK_GRAY);
            lv_style_set_line_color(&style_disable, HEX_DARK_GRAY);
            lv_obj_add_style(slider, &style_disable, LV_PART_INDICATOR | LV_STATE_DISABLED);
            lv_obj_add_style(slider, &style_disable, LV_PART_KNOB | LV_STATE_DISABLED);
            lv_obj_add_style(slider, &style_disable, LV_PART_MAIN | LV_STATE_DISABLED);
            lv_obj_set_style_border_color(slider, HEX_BLACK,LV_PART_INDICATOR);

            lv_obj_t* min_value_label = lv_label_create(parent);
            lv_label_set_text_fmt(min_value_label, "%d", param.second.min);
            lv_obj_align(min_value_label, LV_ALIGN_TOP_LEFT, -10, y_offset-5);
            lv_obj_set_style_text_font(min_value_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(min_value_label);

            lv_obj_t* max_value_label = lv_label_create(parent);
            lv_label_set_text_fmt(max_value_label, "%d", param.second.max);
            lv_obj_align(max_value_label, LV_ALIGN_TOP_LEFT, 155, y_offset-5);
            lv_obj_set_style_text_font(max_value_label,&lv_font_montserrat_12,0);
            obj_to_delete_sensors.push_back(max_value_label);


            y_offset += 10;
            // Create a label to display the current value
            lv_obj_t* value_label = lv_label_create(parent);
            lv_label_set_text_fmt(value_label, "%d", param.second.current_val);
            lv_obj_align(value_label, LV_ALIGN_TOP_LEFT, 75,y_offset);
            obj_to_delete_sensors.push_back(value_label);
            current_edit_sensor_sliders_vec.push_back(slider);

            lv_obj_add_event_cb(slider, slider_event_cb_anim, LV_EVENT_VALUE_CHANGED, value_label);
            lv_obj_add_event_cb(slider, slider_event_cb_updated_val, LV_EVENT_RELEASED, NULL);


            y_offset += 20;
          
            lv_obj_set_user_data(slider, value_label);

            // If parameter is not editable, disable the slider
            if (!param.second.modify_permission) {
                lv_obj_add_state(slider, LV_STATE_DISABLED);
            } 
            i++;  
        }

        save_btn_tech_sensors = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, -50, y_offset+8, "Save", HEX_GREEN, HEX_WHITE, &lv_font_montserrat_14);
        obj_to_delete_sensors.push_back(save_btn_tech_sensors);
        lv_obj_t* discard_btn = create_new_btn(parent, 72, 32, LV_ALIGN_TOP_MID, 30, y_offset+8, "Discard", HEX_RED, HEX_WHITE, &lv_font_montserrat_14);
        obj_to_delete_sensors.push_back(discard_btn);

        y_offset += 50;

        // Create labels for sensor details
        lv_obj_t * label_sensor_name = lv_label_create(parent);
        lv_obj_set_width(label_sensor_name, 180); // Set width for better display
        lv_obj_align(label_sensor_name, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_name,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_name);
        y_offset += 20;



        lv_obj_t * label_sensor_status = lv_label_create(parent);
        lv_obj_set_width(label_sensor_status, 180); // Set width for better display
        lv_obj_align(label_sensor_status, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_status,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_status);
        y_offset += 20;


        lv_obj_t * label_sensor_type = lv_label_create(parent);
        lv_obj_set_width(label_sensor_type, 180); // Set width for better display
        lv_obj_align(label_sensor_type, LV_ALIGN_TOP_LEFT, -10, y_offset);
        lv_obj_set_style_text_font(label_sensor_type,&lv_font_montserrat_12,0);
        obj_to_delete_sensors.push_back(label_sensor_type);
        y_offset += 20;

        lv_label_set_text_fmt(label_sensor_name, "Name: %s", sensor.name.c_str());
        lv_label_set_text_fmt(label_sensor_status, "Status: %s", sensor.status.c_str());
        lv_label_set_text_fmt(label_sensor_type, "Type: %s", sensor.type.c_str());

        lv_obj_add_event_cb(save_btn_tech_sensors, save_btn_tech_sensor_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
        lv_obj_add_event_cb(save_btn_tech_sensors, save_btn_tech_sensor_approved, EVENT_SENSOR_CHANGED_SECC, NULL); // Set the event handler
        lv_obj_add_event_cb(discard_btn, discard_sensor_btn_click_event, LV_EVENT_CLICKED, NULL); // Set the event handler
  }
}

void tech_tabview_event_cb(lv_event_t* e){
  uint16_t new_tab_id = lv_tabview_get_tab_act(tech_tabview);
  if (curr_tech_tabview_id != new_tab_id){
    switch (curr_tech_tabview_id){
      case 0:
        motor_trigged_by_tech_tab = true;
        lv_event_send(dropdown_motors, LV_EVENT_VALUE_CHANGED, tech_tab_motors);
        break;
      case 1:
        sensor_trigged_by_tech_tab = true;
        lv_event_send(dropdown_sensors, LV_EVENT_VALUE_CHANGED, tech_tab_sensors);
        break;
      case 2:
        break;
      default:
        break;
    }

    new_tab_id = lv_tabview_get_tab_act(tech_tabview);
    curr_tech_tabview_id = new_tab_id;
  }
  else{
    curr_tech_tabview_id = new_tab_id;
  }
}


void create_controls_for_tech(lv_obj_t* parent){
  
    tech_tabview = lv_tabview_create(parent, LV_DIR_RIGHT, 80);
    lv_obj_set_style_bg_color(tech_tabview,HEX_MEDIUM_GRAY,0);
    lv_obj_set_style_bg_opa(tech_tabview, LV_OPA_COVER, 0);

    lv_obj_t * tech_tab_btns = lv_tabview_get_tab_btns(tech_tabview);
    lv_obj_set_style_bg_color(tech_tab_btns, lv_palette_darken(LV_PALETTE_GREY, 3), 0);
    lv_obj_set_style_text_color(tech_tab_btns, lv_palette_lighten(LV_PALETTE_GREY, 5), 0);
    lv_obj_set_style_border_side(tech_tab_btns, LV_BORDER_SIDE_LEFT, LV_PART_ITEMS | LV_STATE_CHECKED);

    lv_obj_set_style_bg_opa(tech_tab_btns, LV_OPA_COVER, 0);
    tech_tab_motors = lv_tabview_add_tab(tech_tabview, "Motors\n" LV_SYMBOL_SETTINGS );

    tech_tab_sensors = lv_tabview_add_tab(tech_tabview,  "Sensors\n" LV_SYMBOL_SETTINGS);

    tech_tab_motors_activity = lv_tabview_add_tab(tech_tabview, "Motors\nActivity");  


    lv_obj_clear_flag(lv_tabview_get_content(tech_tabview), LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(tech_tabview, tech_tabview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    dropdown_motors = create_motors_tech(tech_tab_motors);
    dropdown_sensors =create_sensors_tech(tech_tab_sensors);
    dropdown_motors_activity =create_motors_tech(tech_tab_motors_activity);

    //motors_activity(tech_tab_motors_activity);

    lv_obj_add_event_cb(dropdown_sensors, event_view_and_edit_sensors, LV_EVENT_VALUE_CHANGED, tech_tab_sensors);
    lv_obj_add_event_cb(dropdown_motors, event_view_and_edit_motors, LV_EVENT_VALUE_CHANGED, tech_tab_motors);

}



void setup()
{

    Serial.begin(115200);


    delay(1500); // milisec


    xTaskCreate(Start_BLE_server_NIMBLE,"Initialize", STACK_SIZE, nullptr, 2, nullptr);
    // initial atomic flags
    can_play_gesture.test_and_set();
    not_remove_box.test_and_set();
    // has_client.test_and_set();

    // Init Display
    gfx->begin(80000000);
    #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255); // Screen brightness can be modified by adjusting this parameter.1 (0-255)
    #endif

    lv_init();
    touch_init();


    screenWidth = gfx->width();
    screenHeight = gfx->height();

    #ifdef ESP32
    disp_draw_buf = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * screenWidth * screenHeight/2, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    #else
    disp_draw_buf = (lv_color_t *)malloc(sizeof(lv_color_t) * screenWidth * screenHeight/2);
    #endif
   
    if (!disp_draw_buf) {
      Serial.println("LVGL disp_draw_buf allocate failed!");
      return;
    }
    lv_disp_draw_buf_init(&draw_buf, disp_draw_buf, NULL, screenWidth * screenHeight/2);

    // Initialize the display with handlers
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize the (dummy) input device driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
    
    // Initial setup screen for setting is_user
    msg_box_parrent =  lv_obj_create(NULL);
    lv_obj_clear_flag(msg_box_parrent, LV_OBJ_FLAG_SCROLLABLE);
    not_finish_update_sensors.test_and_set();
    searchClientBLEScreen();

    read_yaml_from_prot_screen_function();
    setupWelcomeScreen(); // Show the welcome screen
    
    Serial.println("Setup done");
}


void read_yaml_from_prot_screen_function(){
    read_yaml_from_prot_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(read_yaml_from_prot_screen, HEX_WHITE, 0); // Pink background

    lv_obj_t * label1 = lv_label_create(read_yaml_from_prot_screen);
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#00007f Reading YAML File. \n Please Wait... #");
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label1,&lv_font_montserrat_28,0);
    lv_obj_add_event_cb(read_yaml_from_prot_screen, load_yaml_step, LV_EVENT_CLICKED    , NULL);
  
} 

void setupWelcomeScreen() {
    welcome_screen = lv_obj_create(NULL);
    lv_scr_load_anim(welcome_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    lv_obj_set_style_bg_color(welcome_screen, HEX_WHITE, 0); // Pink background

    lv_obj_t * label1 = lv_label_create(welcome_screen);
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#000099 S##000066 P##00007f M##0000b2 T#");
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -30);
    lv_obj_set_style_text_font(label1,&lv_font_montserrat_48,0);


    lv_obj_t * label2 = lv_label_create(welcome_screen);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
    lv_obj_set_width(label2, 150);
    lv_label_set_text(label2, "Smart Prosthesis' Management Tool");
    lv_obj_set_style_text_font(label2,&lv_font_montserrat_12,0);
    lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label3 = lv_label_create(welcome_screen);
    lv_label_set_text(label3, "Press and hold to start");
    lv_obj_set_style_text_font(label3,&lv_font_montserrat_16,0);
    lv_obj_align(label3, LV_ALIGN_CENTER, 0, 50);

    lv_obj_add_event_cb(welcome_screen, changeToMainScreen, LV_EVENT_LONG_PRESSED , NULL);
}

void load_yaml_step(lv_event_t* e){
  if(has_client.test_and_set()){
      if ( send_yaml_request ) {
        SendNotifyToClient("Please send YAML data", YAML_REQ, pCharacteristic);
        Serial.println("Sent yaml request");
        send_yaml_request = false;
      }
      if (is_yml_sensors_ready){
          sensors.clear(); // making sure to clear demo yaml data before replacong it with real data
          splitSensorsField((char*)*pointer_to_sensor_buff);
          is_yml_sensors_ready = false;
          free(*pointer_to_sensor_buff);
      }

      if (is_yml_motors_ready){
        motors.clear(); // making sure to clear demo yaml data before replacong it with real data
        splitMotorsField((char*)*pointer_to_motors_buff);
        is_yml_motors_ready = false;
        free(*pointer_to_motors_buff);
      }

      if (is_yml_functions_ready){
        functions.clear(); // making sure to clear demo yaml data before replacong it with real data
        splitFunctionsField((char*)*pointer_to_func_buff);
        is_yml_functions_ready = false;
        free(*pointer_to_func_buff);
      }

      if (is_yml_general_ready){
        generalEntries.clear(); // making sure to clear demo yaml data before replacong it with real data
        splitGeneralField((char*)*pointer_to_general_buff);
        is_yml_general_ready = false;
        free(*pointer_to_general_buff);
        yaml_structs_ready=true;
      }

      if(yaml_structs_ready){
        yaml_structs_ready = false;
        xTaskCreate(bleNotifyTask, "BLE Notify Task", 2048, NULL, 2, &bleNotifyTaskHandle);
        pinMode(buttonPin, INPUT_PULLUP);
        attachInterrupt(buttonPin, buttonPress, RISING);
        setupInitialUserScreen();
      }
      
      else{
        delay(100);
        load_yaml_step(e);
      }
  }
  else{
    has_client.clear();
    lv_scr_load_anim(searchBLE_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
  }
}

void changeToMainScreen(lv_event_t * e) {

    lv_refr_now(NULL);

    obj_to_delete_sensors.clear();
    obj_to_delete_motors.clear();
    current_edit_sensor_id = -1;
    current_edit_motor_id = -1;

    current_edit_sensor_sliders_vec.clear();
    current_edit_motor_sliders_vec.clear();


    welcome_screen_flag = false;
    if(has_client.test_and_set()){
      lv_scr_load_anim(read_yaml_from_prot_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    }

    else {
      has_client.clear();
      lv_scr_load_anim(searchBLE_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

    }
}

void set_searching(lv_event_t * e){
  lv_obj_t * btn = lv_event_get_target(e);
  lv_obj_t * label = lv_obj_get_child(btn, 0);
  lv_label_set_text(label, "searching...");
}

void set_search_again(lv_event_t * e){
  lv_obj_t * btn = lv_event_get_target(e);
  lv_obj_t * label = lv_obj_get_child(btn, 0);
  lv_label_set_text(label, "Search Again");
}

void search_ble_again_event(lv_event_t * e){
    delay(2500);
    changeToMainScreen(e);
    if(send_yaml_request){
      lv_event_send(read_yaml_from_prot_screen, LV_EVENT_CLICKED , NULL);
    }
}

void start_demo_event(lv_event_t * e){
  is_demo_yaml.test_and_set();
  sensors.clear();
  motors.clear();
  functions.clear();
  communications.clear();
  generalEntries.clear();
  fileType.clear();

  init_default_yaml();
  setupInitialUserScreen();
}

void searchClientBLEScreen(){
    searchBLE_screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(searchBLE_screen, HEX_WHITE, 0); // Pink background

    lv_obj_t * label1 = lv_label_create(searchBLE_screen);
    lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label1, 250);
    lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    lv_label_set_text(label1, "#00007f No Bluetooth connection found for the prosthesis#");
    lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, -50);
    lv_obj_set_style_text_font(label1,&lv_font_montserrat_20,0);

    lv_obj_t* search_BLE_btn = create_new_btn(searchBLE_screen,170,30, LV_ALIGN_CENTER, 0, 10, "Search Again", HEX_ROYAL_BLUE, HEX_BLACK);
    lv_obj_t* test_example_btn = create_new_btn(searchBLE_screen,170,30, LV_ALIGN_CENTER, 0, 50, "Start DEMO YAML", HEX_YELLOW, HEX_BLACK);


    // lv_obj_t * label_search = lv_obj_get_child(search_BLE_btn, 0);
    lv_obj_add_event_cb(search_BLE_btn, set_searching, LV_EVENT_PRESSED , NULL);
    lv_obj_add_event_cb(search_BLE_btn, search_ble_again_event, LV_EVENT_CLICKED , NULL);
    lv_obj_add_event_cb(search_BLE_btn, set_search_again, LV_EVENT_CLICKED  ,NULL);
    lv_obj_add_event_cb(test_example_btn, start_demo_event, LV_EVENT_CLICKED , NULL);


}

void main_select_click_event(lv_event_t* e) {
        uint32_t id = lv_btnmatrix_get_selected_btn(lv_event_get_target(e));
        const char* txt = lv_btnmatrix_get_btn_text(lv_event_get_target(e), id);

        if(!txt){
          return;
        }

        if (strcmp(txt, "User") == 0) {
          is_user = true;
          if(main_first_time){
            main_first_time = false;
            sensorSwitchVec.clear();
            setupMainUI();
            lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
          }
          else{
            lv_obj_del(mainUI_screen);
            sensorSwitchVec.clear();
            setupMainUI();
            lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
          } 
   

        } else if (strcmp(txt, "Tech") == 0) {
          is_tech = true;
          lv_scr_load_anim(password_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

        } else if (strcmp(txt, "Debug") == 0) {
          lv_scr_load_anim(password_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
        }
}

void pass_click_event(lv_event_t *e) {
      lv_obj_t *btn_matrix = lv_event_get_target(e);
      const char *btn_text = lv_btnmatrix_get_btn_text(btn_matrix, lv_btnmatrix_get_selected_btn(btn_matrix));

      if (btn_text) {
          if (strcmp(btn_text, "OK") == 0) {
              const char *password = lv_textarea_get_text(textarea);
              int password_int = atoi(password);

              if (is_tech && password_int==tech_pass) {
                lv_textarea_set_text(textarea, "");
                if(main_first_time){
                  main_first_time = false;
                  sensorSwitchVec.clear();
                  setupMainUI();
                  lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
                }
                else{
                  lv_obj_del(mainUI_screen);
                  sensorSwitchVec.clear();
                  setupMainUI();
                  lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
                } 

              } else if (!is_tech && password_int == debug_pass) {
                lv_textarea_set_text(textarea, "");
                if(main_first_time){
                  main_first_time = false;
                  sensorSwitchVec.clear();
                  setupMainUI();
                  lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
                }
                else{
                  lv_obj_del(mainUI_screen);
                  sensorSwitchVec.clear();
                  setupMainUI();
                  lv_scr_load_anim(mainUI_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
                } 
              } 
              else {
                  is_tech = false;
                  lv_textarea_set_text(textarea, "");
                  lv_scr_load_anim(initial_user_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
              }                
          } else if (strcmp(btn_text, "Cancel") == 0) {
              is_tech = false;
              lv_textarea_set_text(textarea, "");
              lv_scr_load_anim(initial_user_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);

          } else if (strcmp(btn_text, LV_SYMBOL_BACKSPACE) == 0) {
              lv_textarea_del_char(textarea);
          } else {
              // Append the pressed key to the textarea
              lv_textarea_add_text(textarea, btn_text);
          }
      }
}

void setupInitialUserScreen() {
  is_user = false;
  is_tech = false;
  initial_user_screen = lv_obj_create(NULL);
  lv_scr_load_anim(initial_user_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
  lv_refr_now(NULL);

  // Create a label
  lv_obj_t* label = lv_label_create(initial_user_screen);
  lv_label_set_text(label, "Select Mode:");
  lv_obj_align(label,LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_font(label,&lv_font_montserrat_18,0);

  // Create a button matrix for user mode selection
  static char* btnm_map[] = {"User", "\n", "Tech", "Debug", ""}; // The last element must be an empty string
  int num_pointer = sizeof(btnm_map) / sizeof(btnm_map[0]);

  static char*** map_ptr = (char***)malloc(sizeof(char**));
  *map_ptr = btnm_map;

  lv_obj_t* btnm = create_new_matrix_btn_choose_one(initial_user_screen,map_ptr,num_pointer,LV_ALIGN_CENTER,0,20,HEX_SKY_BLUE,HEX_DARK_BLUE,HEX_WHITE);

  for (const auto& user : generalEntries) {
      if ((strcmp(user.name.c_str(),"Technician_code"))==0){
        tech_pass = user.code;
      }
      else if ((strcmp(user.name.c_str(),"Debug_code"))==0) {
        debug_pass = user.code;
      }
  } 

   // Create a new screen
  password_screen = lv_obj_create(NULL); // NULL creates a new screen
  lv_obj_set_style_bg_color(password_screen, HEX_LIGHT_GRAY, 0); // Optional: Set background color

  // Add a label
  lv_obj_t *label_pass = lv_label_create(password_screen);
  lv_label_set_recolor(label_pass, true);            
  lv_label_set_text(label_pass, "#000066 Enter Password: #");

  lv_obj_align(label_pass, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_font(label_pass,&lv_font_montserrat_18,0);

  // Add a text area for password input
  textarea = lv_textarea_create(password_screen);
  lv_obj_set_size(textarea, 200, 40);
  lv_obj_align(textarea, LV_ALIGN_TOP_MID, 0, 45);
  lv_textarea_set_password_mode(textarea, true);
  lv_textarea_set_one_line(textarea, true);

  // Create a custom button matrix
  static const char *btn_map[] = {
      "1", "2", "3","4","5", "\n",
      "6","7", "8", "9","0", "\n",
      "OK", "Cancel",LV_SYMBOL_BACKSPACE, ""
  };

  lv_obj_t *btn_matrix = lv_btnmatrix_create(password_screen);
  lv_btnmatrix_set_map(btn_matrix, btn_map);
  lv_obj_set_size(btn_matrix, 250, 150); // Adjust size as needed
  lv_obj_align(btn_matrix, LV_ALIGN_BOTTOM_MID, 0, 0);

  // Event handler for the button matrix
  lv_obj_add_event_cb(btn_matrix, pass_click_event , LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(btnm, main_select_click_event, LV_EVENT_CLICKED, NULL);

}


void tabview_event_cb(lv_event_t* e){
  uint16_t new_tab_id = lv_tabview_get_tab_act(tabview);  
  if(new_tab_id == 2){
    curr_is_Setup = true;
  }
  else{
    if(curr_is_Setup){
      std::vector<int> new_on_sensors = find_new_on_sensor();
      std::vector<int> new_off_sensors = find_new_off_sensor();
      if((new_on_sensors.size() != 0 ) || (new_off_sensors.size() != 0 )){
        lv_tabview_set_act(tabview,2,LV_ANIM_ON);
        lv_obj_t* curr_msg_box = lv_msgbox_create(NULL, "Changes were not saved", "Please choose Save/Discard",NULL, true);
        lv_obj_align(curr_msg_box, LV_ALIGN_CENTER, 0, 0); 
      }
      else{
        curr_is_Setup = false;
      } 
    }
  }
}


void setupMainUI() {

    mainUI_screen = lv_obj_create(NULL); // NULL creates a new screen
    tabview = lv_tabview_create(mainUI_screen, LV_DIR_TOP, 30);
    lv_obj_set_style_bg_color(tabview,HEX_LIGHT_GRAY,0);

    // Add tabs
    home_tab = lv_tabview_add_tab(tabview, "Home");
    stat_tab = lv_tabview_add_tab(tabview, "Stat");
    setup_tab = lv_tabview_add_tab(tabview, "Setup");

    //            is_user = false;            is_tech = true;
    if (!is_user) {
      tech_tab = lv_tabview_add_tab(tabview, "Tech");
      lv_obj_set_style_bg_color(tech_tab,HEX_MEDIUM_GRAY,0);
      lv_obj_set_style_bg_opa(tech_tab, LV_OPA_COVER, 0);
    }
    if  (!is_user && !is_tech){
        debug_tab = lv_tabview_add_tab(tabview, "Debug");
    }

    create_controls_for_main(home_tab);
    create_controls_for_stat(stat_tab);
    create_controls_for_setup(setup_tab);
    if (!is_user) {
      create_controls_for_tech(tech_tab);
    }
    if  (!is_user && !is_tech){
    
    	create_controls_for_debug(debug_tab);
    }

    lv_obj_add_event_cb(tabview, tabview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
}


void return_to_main(lv_event_t *e) {    
    // Get the object that triggered the event
    lv_obj_t *btn = lv_event_get_target(e);    

    // Check if the event is a "clicked" event
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
      is_tech = false;
      is_user = false;
      
      for( auto& obj_td: obj_to_delete_sensors){
          lv_obj_del(obj_td);
      }
      for( auto& obj_td: obj_to_delete_motors){
          lv_obj_del(obj_td);
      }
      obj_to_delete_sensors.clear();
      obj_to_delete_motors.clear();
      current_edit_sensor_id = -1;
      current_edit_motor_id = -1;

      current_edit_sensor_sliders_vec.clear();
      current_edit_motor_sliders_vec.clear();
      if(debug_tab){
        delete_debug();
      }
      lv_scr_load_anim(initial_user_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
    }
}


void loop(){
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
}
