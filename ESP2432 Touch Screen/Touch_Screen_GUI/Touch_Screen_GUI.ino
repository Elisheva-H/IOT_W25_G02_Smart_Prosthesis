

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
#define HEX_MEDIUM_GRAY lv_color_hex(0x808080) // Medium Gray
#define HEX_DARK_GRAY lv_color_hex(0x404040) // Dark Gray
#define HEX_DARK_BLUE lv_color_hex(0x00008b) // Dark Blue
#define HEX_ROYAL_BLUE lv_color_hex(0x4169e1) // Royal Blue
#define HEX_SKY_BLUE lv_color_hex(0x00bfff) // Sky Blue
#define HEX_PINK lv_color_hex(0xffc0cb) // Pink
#define HEX_PURPLE lv_color_hex(0x800080) // Purple
#define HEX_RED lv_color_hex(0xff0000) // Red
#define HEX_BABY_PINK lv_color_hex(0xf4c2c2) // Baby Pink
#define HEX_BURGUNDY lv_color_hex(0x800020) // Burgundy
#define HEX_GOLD lv_color_hex(0xffd700) // Gold
#define HEX_TURQUOISE lv_color_hex(0x40e0d0) // Turquoise
#define HEX_LIGHT_PURPLE lv_color_hex(0xda70d6) // Light Purple
#define HEX_MEDIUM_PURPLE lv_color_hex(0x9370db) // Medium Purple
#define HEX_DARK_PURPLE lv_color_hex(0x663399) // Dark Purple
#define HEX_YELLOW lv_color_hex(0xf7edbe) // Bannana yellow

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
lv_obj_t *tabview;  // Declare the global tabview variable
bool is_user = false;
bool is_tech = false;
bool yaml_structs_ready=false;

//TODO: remove this later and set from BLE
// static char* gest_map[] = {"HIFIVE", "LIKE", "SCISSORS", "ROCK", "PAPER", ""}; // The last element must be an empty string
const char* empty_string = "";


int tech_pass;
int debug_pass; 

lv_obj_t *initial_user_screen = NULL; //
lv_obj_t * read_yaml_from_prot_screen = NULL;



void show_password_screen(lv_event_t *e) {

  Serial.println("Creating password screen...");
  for (const auto& user : generalEntries) {
      if ((strcmp(user.name.c_str(),"Technician_code"))==0){
        tech_pass = user.code;
        // snprintf(tech_pass, sizeof(tech_pass), "%04d", user.code);
        Serial.printf("%s password changed to %d\n",user.name.c_str(),tech_pass);
      }
      else if ((strcmp(user.name.c_str(),"Debug_code"))==0) {
        debug_pass = user.code;
        // snprintf(debug_pass, sizeof(debug_pass), "%04d", user.code);
        Serial.printf("%s password changedto %d\n",user.name.c_str(),debug_pass);
      }
      else{Serial.printf("User %s is not in user lists\n",user.name.c_str());}
  } 

  // Create a new screen
  lv_obj_t *password_screen = lv_obj_create(NULL); // NULL creates a new screen
  lv_obj_set_style_bg_color(password_screen, HEX_LIGHT_GRAY, 0); // Optional: Set background color

  // Add a label
  lv_obj_t *label = lv_label_create(password_screen);
  lv_label_set_recolor(label, true);            
  lv_label_set_text(label, "#000066 Enter Password: #");

  // lv_label_set_text(label, "#000099 S##000066 P##00007f M##0000b2 T#");

  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_style_text_font(label,&lv_font_montserrat_18,0);


  // Add a text area for password input
  lv_obj_t *textarea = lv_textarea_create(password_screen);
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
  lv_obj_add_event_cb(btn_matrix, [](lv_event_t *e) {
      lv_obj_t *btn_matrix = lv_event_get_target(e);
      const char *btn_text = lv_btnmatrix_get_btn_text(btn_matrix, lv_btnmatrix_get_selected_btn(btn_matrix));

      if (btn_text) {
          lv_obj_t *textarea = (lv_obj_t *)lv_event_get_user_data(e);

          if (strcmp(btn_text, "OK") == 0) {
              const char *password = lv_textarea_get_text(textarea);
              int password_int = atoi(password);

              if (is_tech && password_int==tech_pass) {
                Serial.print("Correct Tech Password \nMode: Tech \n");
                setupMainUI(); // Load the main UI with the "Tech" tab
              } else if (!is_tech && password_int == debug_pass) {
                Serial.printf("Correct Debug Password \nMode: Debug \n");
                setupMainUI(); // Load the main UI with the "Debug" tab
              } 
              else {
                  Serial.println("Incorrect Password:");
                  Serial.println(password);
                  is_tech = false;
                  lv_scr_load(initial_user_screen); // Return to the main screen
              }                
          } else if (strcmp(btn_text, "Cancel") == 0) {
              is_tech = false;
              lv_scr_load(initial_user_screen); // Return to the main screen
          } else if (strcmp(btn_text, LV_SYMBOL_BACKSPACE) == 0) {
              lv_textarea_del_char(textarea);
          } else {
              // Append the pressed key to the textarea
              lv_textarea_add_text(textarea, btn_text);
          }
      }
  }, LV_EVENT_CLICKED, textarea);

  // Load the new screen
  lv_scr_load(password_screen);

  Serial.println("Password screen created.");
}

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

lv_obj_t* create_new_btn(lv_obj_t* parent, lv_coord_t w, lv_coord_t h, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs, const char* name, lv_color_t btn_color, lv_color_t label_color){
    lv_obj_t *new_btn = lv_btn_create(parent); // Create button with home_tab as parent
    lv_obj_set_size(new_btn, w, h); // Set button size

    // Set the button color
    lv_obj_set_style_bg_color(new_btn, btn_color, 0);


    lv_obj_align(new_btn, align, x_ofs, y_ofs); // Align to bottom left
    lv_obj_t *label = lv_label_create(new_btn);
    lv_obj_set_style_text_color(label, label_color, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

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
      // lv_obj_draw_part_dsc_t * dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
      // if (dsc->part == LV_PART_ITEMS){
      //   dsc->rect_dsc->bg_color = HEX_RED;
      // }
      if (!(can_play_gesture.test_and_set())){
        can_play_gesture.clear();
        lv_label_set_text(label, "Can't Play gesture right now");
      }
      else{
        // Serial.printf("PROBLEM");
        can_play_gesture.clear();

        sending_gesture(current_text); 
        // Serial.printf("\n%s\n",current_text );
        char new_text[20]; // Adjust size as needed
        snprintf(new_text, sizeof(new_text), "Playing: %s", current_text);
        lv_label_set_text(label, new_text);
      }
    }
    else if(code == LV_EVENT_RELEASED){
      delay(800);
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
          Serial.printf("string_1_%d : %s\n", (j-1), temp_ptr[j-1]);
        }else
        {
          Serial.printf("string_1_%d : %s\n", j, temp_ptr[j]);
        }
        Serial.printf("string_2_%d : %s\n", i, *((*map)+i));
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
    lv_obj_t* return_btn = create_new_btn(parent, 70, 40, LV_ALIGN_BOTTOM_LEFT, 0, 0, "Return", HEX_RED , HEX_WHITE);
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
        lv_obj_set_style_text_color(label_home_BLE_2, lv_color_hex(0x047a04), 0); // Green for ON
    }
    else{
        lv_label_set_text(label_home_BLE_2,  LV_SYMBOL_CLOSE);
        lv_obj_set_style_text_color(label_home_BLE_2, lv_color_hex(0xc30a12), 0); // Red for OFF
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
    lv_obj_t * info_label = lv_label_create(parent);
    lv_obj_align(info_label,LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_width(info_label, 150);
    lv_label_set_text(info_label, " ");

    int j = 0;
    for(int i=0; i< num_function_total; i++){
      if(functions[i].protocol_type == FUNC_TYPE_GESTURE){
        Serial.printf("HERE\n");
        const char* temp_str = (functions[i].name).c_str();
        Serial.printf("%s\n",temp_str);
        gestures_matrix[j] = create_new_btn(parent, 90, 30, LV_ALIGN_TOP_RIGHT, -7 - (j%max_in_row)*95, 17 + (j/max_in_row)*35 , temp_str,HEX_DARK_BLUE,HEX_WHITE );
        lv_obj_add_event_cb(gestures_matrix[j], gestures_click_event, LV_EVENT_ALL, info_label); // Set the event handler
        j++;
      }
    }
        // gestures matrix hadder
    lv_obj_t* label_gest = lv_label_create(parent);
    lv_label_set_text(label_gest, "Gestures");
    // lv_obj_center(label);
    lv_obj_align(label_gest,LV_ALIGN_TOP_RIGHT, -64, -5);
    lv_obj_set_style_text_font(label_gest,&lv_font_montserrat_18,0);
    // lv_obj_add_event_cb(return_btn, return_to_main, LV_EVENT_CLICKED, NULL); // Set the event handler
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
      if (sensor.status == "on") {
          // Create a label for the sensor name
          lv_obj_t* sensor_name_label = lv_label_create(parent); 
          lv_label_set_text(sensor_name_label, (display_name + ":").c_str()); 
          lv_obj_align(sensor_name_label, LV_ALIGN_TOP_LEFT, 15, y_offset); 
          lv_obj_set_style_text_font(sensor_name_label, &lv_font_montserrat_14, 0); 

          // Create a status label
          lv_obj_t* sensor_status_label = lv_label_create(parent); 
          lv_obj_align(sensor_status_label, LV_ALIGN_TOP_RIGHT, -15, y_offset); 
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_14, 0);
          
          lv_label_set_text(sensor_status_label, "ON " LV_SYMBOL_OK); 
          lv_obj_set_style_text_color(sensor_status_label, lv_color_hex(0x047a04), 0); // Green for ON
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_14, 0);  // Slightly larger font
          y_offset += 25;
      } 

      // Update the vertical offset for the next sensor
      
  }
  for (const auto& sensor : sensors){
      String display_name = sensor.name;
      display_name.replace("_", " ");
      display_name[0] = toupper(display_name[0]);

    if (sensor.status == "off") {
          // Create a label for the sensor name
          lv_obj_t* sensor_name_label = lv_label_create(parent); 
          lv_label_set_text(sensor_name_label, (display_name + ":").c_str()); 
          lv_obj_align(sensor_name_label, LV_ALIGN_TOP_LEFT, 15, y_offset); 
          lv_obj_set_style_text_font(sensor_name_label, &lv_font_montserrat_14, 0); 

          // Create a status label
          lv_obj_t* sensor_status_label = lv_label_create(parent); 
          lv_obj_align(sensor_status_label, LV_ALIGN_TOP_RIGHT, -15, y_offset); 
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_14, 0);
      
          lv_label_set_text(sensor_status_label, "OFF " LV_SYMBOL_CLOSE); 
          lv_obj_set_style_text_color(sensor_status_label, lv_color_hex(0xc30a12), 0); // Red for OFF
          lv_obj_set_style_text_font(sensor_status_label, &lv_font_montserrat_16, 0);  // Slightly larger font

          y_offset += 25;

    }
  }
}

void create_controls_for_setup(lv_obj_t* parent){

}




/*
void create_controls_for_tab(lv_obj_t* parent, const char* btn1_text, const char* btn2_text) {
    // Calculate positions for side-by-side buttons
    const int button_width = 120;
    const int button_height = 50;
    const int button_spacing = 20;  // Space between buttons
    const int total_width = (button_width * 2) + button_spacing;
    const int start_x = -(total_width / 2) + (button_width / 2);  // Center the button group

    // Create first button
    lv_obj_t* btn1 = lv_btn_create(parent);
    lv_obj_set_size(btn1, button_width, button_height);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, start_x, 20);  // Position from center, offset left
    lv_obj_set_style_bg_color(btn1, lv_color_hex(0xFFC0CB), 0);
    lv_obj_t* label1 = lv_label_create(btn1);
    lv_label_set_text(label1, btn1_text);
    lv_obj_center(label1);

    // Create second button
    lv_obj_t* btn2 = lv_btn_create(parent);
    lv_obj_set_size(btn2, button_width, button_height);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, start_x + button_width + button_spacing, 20);  // Position from center, offset right
    lv_obj_t* label2 = lv_label_create(btn2);
    lv_label_set_text(label2, btn2_text);
    lv_obj_center(label2);



    // Create first slider and its label
    lv_obj_t* slider1 = lv_slider_create(parent);
    lv_obj_set_size(slider1, 200, 10);
    lv_obj_align(slider1, LV_ALIGN_TOP_MID, 0, 90);  // Adjusted Y position after buttons
    lv_obj_t* slider1_label = lv_label_create(parent);
    lv_label_set_text(slider1_label, "0");
    lv_obj_align_to(slider1_label, slider1, LV_ALIGN_OUT_TOP_MID, 0, -5);

    // Create second slider and its label
    lv_obj_t* slider2 = lv_slider_create(parent);
    lv_obj_set_size(slider2, 200, 10);
    lv_obj_align(slider2, LV_ALIGN_TOP_MID, 0, 140);  // Adjusted Y position
    lv_obj_t* slider2_label = lv_label_create(parent);
    lv_label_set_text(slider2_label, "0");
    lv_obj_align_to(slider2_label, slider2, LV_ALIGN_OUT_TOP_MID, 0, -5);

    // Event handlers remain the same
    lv_obj_add_event_cb(btn1, [](lv_event_t* e) {
        uint32_t tab_num = (uint32_t)lv_obj_get_index(lv_obj_get_parent(lv_event_get_target(e))) + 1;
        Serial.printf("Button 1 pressed in tab %d\n", tab_num);
    }, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(btn2, [](lv_event_t* e) {
        uint32_t tab_num = (uint32_t)lv_obj_get_index(lv_obj_get_parent(lv_event_get_target(e))) + 1;
        Serial.printf("Button 2 pressed in tab %d\n", tab_num);
    }, LV_EVENT_CLICKED, NULL);

    // Slider event handler
    auto slider_event_cb = [](lv_event_t* e) {
        lv_obj_t* slider = lv_event_get_target(e);
        lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
        uint32_t tab_num = (uint32_t)lv_obj_get_index(lv_obj_get_parent(slider)) + 1;
        char buf[8];
        snprintf(buf, sizeof(buf), "%d", (int)lv_slider_get_value(slider));
        lv_label_set_text(label, buf);
        Serial.printf("Slider changed to %d in tab %d\n", 
            (int)lv_slider_get_value(slider),
            tab_num);
    };

    lv_obj_add_event_cb(slider1, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider1_label);
    lv_obj_add_event_cb(slider2, slider_event_cb, LV_EVENT_VALUE_CHANGED, slider2_label);
}
*/


void setup()
{

    Serial.begin(115200);


    delay(1500); // milisec


    xTaskCreate(Start_BLE_server_NIMBLE,"Initialize", STACK_SIZE, nullptr, 2, nullptr);

    // initial atomic flags
    can_play_gesture.test_and_set();
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
    // gfx->fillScreen(BLACK);

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
    setupWelcomeScreen(); // Show the welcome screen


    // delay(5000);
    // lv_event_send(welcome_screen, LV_EVENT_READY, NULL);

    Serial.println("Setup done");
}


void read_yaml_from_prot_screen_function(){
    read_yaml_from_prot_screen = lv_obj_create(NULL);
    // lv_scr_load(read_yaml_from_prot_screen);

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
    lv_scr_load(welcome_screen);

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
        SendNotiyToClient("Please send YAML data", YAML_REQ, pCharacteristic);
        Serial.println("Sent yaml request");
        send_yaml_request = false;
      }
        // delay(5000);
      if (is_yml_sensors_ready){
          sensors.clear(); // making sure to clear demo yaml data before replacong it with real data
          Serial.printf("sensors are ready\n");
          splitSensorsField((char*)*pointer_to_sensor_buff);
          is_yml_sensors_ready = false;
          free(*pointer_to_sensor_buff);
          Serial.printf("done parsing and storing sensors\n");
      }

      if (is_yml_motors_ready){
        motors.clear(); // making sure to clear demo yaml data before replacong it with real data
        Serial.printf("motors are ready\n");
        splitMotorsField((char*)*pointer_to_motors_buff);
        is_yml_motors_ready = false;
        free(*pointer_to_motors_buff);
        Serial.printf("done parsing and storing motors\n");

      }

      if (is_yml_functions_ready){
        functions.clear(); // making sure to clear demo yaml data before replacong it with real data
        Serial.printf("functions are ready\n");
        splitFunctionsField((char*)*pointer_to_func_buff);
        is_yml_functions_ready = false;
        free(*pointer_to_func_buff);
        Serial.printf("done parsing and storing functions\n");

      }

      if (is_yml_general_ready){
        generalEntries.clear(); // making sure to clear demo yaml data before replacong it with real data
        Serial.printf("passwords are ready\n");
        splitGeneralField((char*)*pointer_to_general_buff);
        is_yml_general_ready = false;
        free(*pointer_to_general_buff);
        Serial.printf("done parsing and storing passwords, changing users passwords\n");
        yaml_structs_ready=true;
      }

      if(yaml_structs_ready){
        setupInitialUserScreen();
      }
      
      else{
        delay(100);
        Serial.print("ops...");
        load_yaml_step(e);
      }
  }
  else{
    has_client.clear();
    searchClientBLEScreen();
  }
}





void changeToMainScreen(lv_event_t * e) {
    // delay(10000);
    welcome_screen_flag = false;
    if(has_client.test_and_set()){
      Serial.printf("1");
      read_yaml_from_prot_screen_function();
      Serial.printf("2");
      lv_scr_load(read_yaml_from_prot_screen);
      Serial.printf("3");
      // delay(200);
      // if(send_yaml_request){
      //   lv_event_send(read_yaml_from_prot_screen, LV_EVENT_CLICKED , NULL);

      // }


      // if ( send_yaml_request ) {
      //   SendNotiyToClient("Please send YAML data", YAML_REQ, pCharacteristic);
      //   Serial.println("Sent yaml request");
      //   send_yaml_request = false;
      // }
      //   // delay(5000);
      // if (is_yml_sensors_ready.test_and_set()){
      //     sensors.clear(); // making sure to clear demo yaml data before replacong it with real data
      //     Serial.printf("sensors are ready\n");
      //     splitSensorsField((char*)*pointer_to_sensor_buff);
      //     is_yml_sensors_ready.clear();
      //     free(*pointer_to_sensor_buff);
      //     Serial.printf("done parsing and storing sensors\n");
      // }

      // if (is_yml_motors_ready){
      //   motors.clear(); // making sure to clear demo yaml data before replacong it with real data
      //   Serial.printf("motors are ready\n");
      //   splitMotorsField((char*)*pointer_to_motors_buff);
      //   is_yml_motors_ready = false;
      //   free(*pointer_to_motors_buff);
      //   Serial.printf("done parsing and storing motors\n");

      // }

      // if (is_yml_functions_ready){
      //   functions.clear(); // making sure to clear demo yaml data before replacong it with real data
      //   Serial.printf("functions are ready\n");
      //   splitFunctionsField((char*)*pointer_to_func_buff);
      //   is_yml_functions_ready = false;
      //   free(*pointer_to_func_buff);
      //   Serial.printf("done parsing and storing functions\n");

      // }

      // if (is_yml_general_ready){
      //   generalEntries.clear(); // making sure to clear demo yaml data before replacong it with real data
      //   Serial.printf("passwords are ready\n");
      //   splitGeneralField((char*)*pointer_to_general_buff);
      //   is_yml_general_ready = false;
      //   free(*pointer_to_general_buff);
      //   Serial.printf("done parsing and storing passwords, changing users passwords\n");
      //   yaml_structs_ready=true;
      // }

      // if(yaml_structs_ready){
      //   setupInitialUserScreen();
      // }
      
      // else{
      //   is_yml_sensors_ready.clear();
      //   delay(1000);
      //   changeToMainScreen(e);
      // }

      // while (!yaml_structs_ready) {
      //     delay(5000);

      // }
      // setupInitialUserScreen(); // Switch to the main screen
    }



     
    else {
      has_client.clear();
      searchClientBLEScreen();
    }
}

void set_searching(lv_event_t * e){
  lv_obj_t * btn = lv_event_get_target(e);
  lv_obj_t * label = lv_obj_get_child(btn, 0);
  lv_label_set_text(label, "searching...");
  lv_obj_add_event_cb(btn, search_ble_again_event, LV_EVENT_CLICKED , NULL);
}

void search_ble_again_event(lv_event_t * e){
    delay(2500);
    changeToMainScreen(e);
    // delay(2000);
    if(send_yaml_request){
      lv_event_send(read_yaml_from_prot_screen, LV_EVENT_CLICKED , NULL);
    }
}

void start_demo_event(lv_event_t * e){
  // TODO: load the deafult YAML DEMO
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
    lv_obj_t* searchBLE_screen = lv_obj_create(NULL);
    lv_scr_load(searchBLE_screen);

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

    lv_obj_add_event_cb(search_BLE_btn, set_searching, LV_EVENT_PRESSED , NULL);
    lv_obj_add_event_cb(test_example_btn, start_demo_event, LV_EVENT_CLICKED , NULL);


}

void setupInitialUserScreen() {
    //  TODO: delete:
    


    is_user = false;
    is_tech = false;
    initial_user_screen = lv_obj_create(NULL);
    lv_scr_load(initial_user_screen);

    // Create a label
    lv_obj_t* label = lv_label_create(initial_user_screen);
    lv_label_set_text(label, "Select Mode:");
    // lv_obj_center(label);
    lv_obj_align(label,LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_set_style_text_font(label,&lv_font_montserrat_18,0);

    // Create a button matrix for user mode selection
    static char* btnm_map[] = {"User", "\n", "Tech", "Debug", ""}; // The last element must be an empty string
    int num_pointer = sizeof(btnm_map) / sizeof(btnm_map[0]);

    // char** new_btnm_map = (char**)malloc(num_pointer * sizeof(char*));
    // for(int i=0; i<num_pointer; i++){
    //   int temp_len = strlen(btnm_map[i]);
    //   Serial.printf("len_string_%d : %d\n", i, temp_len);
    // }

    Serial.printf("num_pinter: %d\n", num_pointer);
    // for(int i=0; i<num_pointer-1; i++){
    //   int temp_len = strlen(btnm_map[i]);
    //   Serial.printf("len_string_%d : %d\n", i, temp_len);
    // }
    // static char** ptr = (char**)malloc(num_pointer * sizeof(char*));
    // Serial.printf("all_good");
    // free(ptr);
    static char*** map_ptr = (char***)malloc(sizeof(char**));
    *map_ptr = btnm_map;
    
    // lv_obj_t* btnm = lv_btnmatrix_create(initial_user_screen);
    // lv_btnmatrix_set_map(btnm, btnm_map);
    // lv_btnmatrix_set_one_checked(btnm, true); // Only one button can be checked at a time
    // lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 50); // Position below the label
    lv_obj_t* btnm = create_new_matrix_btn_choose_one(initial_user_screen,map_ptr,num_pointer,LV_ALIGN_CENTER,0,20,HEX_SKY_BLUE,HEX_DARK_BLUE,HEX_WHITE);

    // int new_num_pointer = sizeof(*map_ptr) / sizeof(char*);
    // Serial.printf("\nnew_num_pointer: %d\n", new_num_pointer);
    for(int i=0; (*map_ptr)[i] != ""; i++){
      int temp_len = strlen((*map_ptr)[i]);
      Serial.printf("len_string_%d: %d    string: '%s' \n", i, temp_len,(*map_ptr)[i]);
    }


    lv_obj_add_event_cb(btnm, [](lv_event_t* e) {
        uint32_t id = lv_btnmatrix_get_selected_btn(lv_event_get_target(e));
        const char* txt = lv_btnmatrix_get_btn_text(lv_event_get_target(e), id);

        if (strcmp(txt, "User") == 0) {
          is_user = true;
          Serial.println("Mode: User");
          setupMainUI();
        } else if (strcmp(txt, "Tech") == 0) {
          is_tech = true;
          Serial.println("Requesting password for Tech mode");
          //show_password_popup(e); // Show the password popup here
          show_password_screen(e); // Show the password screen here
        } else {
          Serial.println("Requesting password for Debug mode"); 
          show_password_screen(e); // Show the password screen here
        }
    }, LV_EVENT_CLICKED, NULL);
}



void setupMainUI() {

    lv_obj_t *mainUI_screen = lv_obj_create(NULL); // NULL creates a new screen
    lv_scr_load(mainUI_screen);

    tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 30);
    lv_obj_set_style_bg_color(tabview,HEX_LIGHT_GRAY,0);

    

    // Add tabs
    home_tab = lv_tabview_add_tab(tabview, "Home");
    stat_tab = lv_tabview_add_tab(tabview, "Stat");
    setup_tab = lv_tabview_add_tab(tabview, "Setup");
    //            is_user = false;            is_tech = true;
    if (!is_user) {
      lv_obj_t* tech_tab = lv_tabview_add_tab(tabview, "Tech");
    }
    if  (!is_user && !is_tech){
        lv_obj_t* debug_tab = lv_tabview_add_tab(tabview, "Bug");
    }

    // lv_obj_add_event_cb(home_tab, [](lv_event_t* e) {
    //       create_controls_for_main(home_tab);
    // }, LV_EVENT_REFRESH, NULL);


    // Create controls for both tabs
    //create_controls_for_tab(home_tab, "home_tab Btn1", "home_tab Btn2");
    //create_controls_for_tab(tab2, "Tab2 Btn1", "Tab2 Btn2");
    create_controls_for_main(home_tab);

    create_controls_for_stat(stat_tab);
    // create_controls_for_setup(setup_tab);

    
}


void return_to_main(lv_event_t *e) {    
    // Get the object that triggered the event
    lv_obj_t *btn = lv_event_get_target(e);    
    // Check if the event is a "clicked" event
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
      Serial.println("Return Button was pressed");
      is_tech = false;
      is_user = false;
      // return_BLE();


      // delay(100);

      // if (!confirmationReceived) {
      //   //Serial.println("NO");
      // }
      // else {
      //   Serial.println("YES");
      // }

      lv_scr_load(initial_user_screen); // Return to the main screen

      // Assuming 'tabview' is your tabview object
      //lv_tabview_set_act(tabview, 1, LV_ANIM_OFF); // Set the active tab to the second tab (index 1)
    }
}


void loop(){
  lv_timer_handler(); /* let the GUI do its work */
  delay(5);
  // Flags for YAML parsing
  // if (is_yml_sensors_ready.test_and_set()){
  //     sensors.clear(); // making sure to clear demo yaml data before replacong it with real data
  //     Serial.printf("sensors are ready\n");
  //     splitSensorsField((char*)*pointer_to_sensor_buff);
  //     is_yml_sensors_ready.clear();
  //     free(*pointer_to_sensor_buff);
  //     Serial.printf("done parsing and storing sensors\n");

  // }
  // if (is_yml_motors_ready){
  //   motors.clear(); // making sure to clear demo yaml data before replacong it with real data
  //   Serial.printf("motors are ready\n");
  //   splitMotorsField((char*)*pointer_to_motors_buff);
  //   is_yml_motors_ready = false;
  //   free(*pointer_to_motors_buff);
  //   Serial.printf("done parsing and storing motors\n");

  // }
  // if (is_yml_functions_ready){
  //   functions.clear(); // making sure to clear demo yaml data before replacong it with real data
  //   Serial.printf("functions are ready\n");
  //   splitFunctionsField((char*)*pointer_to_func_buff);
  //   is_yml_functions_ready = false;
  //   free(*pointer_to_func_buff);
  //   Serial.printf("done parsing and storing functions\n");

  // }
  // if (is_yml_general_ready){
  //   generalEntries.clear(); // making sure to clear demo yaml data before replacong it with real data
  //   Serial.printf("passwords are ready\n");
  //   splitGeneralField((char*)*pointer_to_general_buff);
  //   is_yml_general_ready = false;
  //   free(*pointer_to_general_buff);
  //   Serial.printf("done parsing and storing passwords, changing users passwords\n");
  //   yaml_structs_ready=true;
  // }
}

