/*******************************************************************************
 * LVGL Widgets
 * This is a widgets demo for LVGL - Light and Versatile Graphics Library
 ******************************************************************************/
#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#define TFT_BL 27
#define GFX_BL DF_GFX_BL // default backlight pin

/* Display configuration */
Arduino_DataBus *bus = new Arduino_ESP32SPI(2 /* DC */, 15 /* CS */, 14 /* SCK */, 13 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, -1 /* RST */, 3 /* rotation */, true /* IPS */);

/* Touch include */
#include "touch.h"

/* Change to your screen resolution */
static uint32_t screenWidth;
static uint32_t screenHeight;
static lv_disp_draw_buf_t draw_buf;
static lv_color_t *disp_draw_buf;
static lv_disp_drv_t disp_drv;
lv_obj_t *tabview;  // Declare the global tabview variable
bool is_user = false;
bool is_tech = false;

void show_password_popup(lv_event_t *e) {
    // Get the screen object
    lv_obj_t *screen = lv_scr_act();

    // Create a container for the popup
    lv_obj_t *popup = lv_obj_create(screen);
    lv_obj_set_size(popup, 200, 150);
    lv_obj_set_pos(popup, 50, 50);
    lv_obj_set_style_bg_color(popup, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(popup, 2, 0);
    lv_obj_set_style_border_color(popup, lv_color_hex(0x000000), 0);
    
    // Add a label to the popup
    lv_obj_t *label = lv_label_create(popup);
    lv_label_set_text(label, "Enter Password:");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

    // Add a text area for password input
    lv_obj_t *textarea = lv_textarea_create(popup);
    lv_obj_set_size(textarea, 180, 40);
    lv_obj_align(textarea, LV_ALIGN_CENTER, 0, -20);
    lv_textarea_set_password_mode(textarea, true);

    // Add an "OK" button
    lv_obj_t *ok_btn = lv_btn_create(popup);
    lv_obj_set_size(ok_btn, 80, 30);
    lv_obj_align(ok_btn, LV_ALIGN_BOTTOM_LEFT, 20, -10);
    lv_obj_t *ok_label = lv_label_create(ok_btn);
    lv_label_set_text(ok_label, "OK");

    // Add a "Cancel" button
    lv_obj_t *cancel_btn = lv_btn_create(popup);
    lv_obj_set_size(cancel_btn, 80, 30);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_RIGHT, -20, -10);
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    lv_label_set_text(cancel_label, "Cancel");

    // Event callback for the "OK" button
    lv_obj_add_event_cb(ok_btn, [](lv_event_t *e) {
        lv_obj_t *textarea = (lv_obj_t *)lv_event_get_user_data(e);
        const char *password = lv_textarea_get_text(textarea);
        if (strcmp(password, "1234") == 0) {
            Serial.println("Correct Password");
            // Add code to load the "Tech" tab
        } else {
            Serial.println("Incorrect Password");
        }
    }, LV_EVENT_CLICKED, textarea);

    // Event callback for the "Cancel" button
    lv_obj_add_event_cb(cancel_btn, [](lv_event_t *e) {
        lv_obj_t *popup = lv_obj_get_parent(lv_event_get_target(e));
        lv_obj_del(popup); // Close the popup
    }, LV_EVENT_CLICKED, NULL);
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

void create_controls_for_main(lv_obj_t* parent) {
// Add the "Return" button to each tab
    lv_obj_t *return_btn = lv_btn_create(parent); // Create button with home_tab as parent
    lv_obj_set_size(return_btn, 100, 50); // Set button size
    lv_obj_align(return_btn, LV_ALIGN_BOTTOM_MID, 0, -10); // Align to bottom center
    lv_obj_t *return_label = lv_label_create(return_btn);
    lv_label_set_text(return_label, "Return");
    lv_obj_add_event_cb(return_btn, return_to_main, LV_EVENT_CLICKED, NULL); // Set the event handler

}

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


void setup()
{
    Serial.begin(115200);
    Serial.println("LVGL Tabview Demo");

    // Init Display
    gfx->begin(80000000);
    #ifdef TFT_BL
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    ledcSetup(0, 2000, 8);
    ledcAttachPin(TFT_BL, 0);
    ledcWrite(0, 255); // Screen brightness can be modified by adjusting this parameter.1 (0-255)
    #endif
    gfx->fillScreen(BLACK);

    lv_init();
    touch_init();

    screenWidth = gfx->width();
    screenHeight = gfx->height();

    // Allocate memory for the drawing buffer
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
    setupInitialUserScreen();

    Serial.println("Setup done");
}

void setupInitialUserScreen() {
    lv_obj_t* screen = lv_obj_create(NULL);
    lv_scr_load(screen);

    // Create a label
    lv_obj_t* label = lv_label_create(screen);
    lv_label_set_text(label, "Select Mode:");
    lv_obj_center(label);

    // Create a button matrix for user mode selection
    static const char* btnm_map[] = {"User", "Tech", "Debug", ""}; // The last element must be an empty string

    lv_obj_t* btnm = lv_btnmatrix_create(screen);
    lv_btnmatrix_set_map(btnm, btnm_map);
    lv_btnmatrix_set_one_checked(btnm, true); // Only one button can be checked at a time
    lv_obj_align(btnm, LV_ALIGN_CENTER, 0, 50); // Position below the label

    lv_obj_add_event_cb(btnm, [](lv_event_t* e) {
        uint32_t id = lv_btnmatrix_get_selected_btn(lv_event_get_target(e));
        const char* txt = lv_btnmatrix_get_btn_text(lv_event_get_target(e), id);
        if (strcmp(txt, "User") == 0) {
            is_user = true;
            is_tech = false;
            Serial.println("Mode: User");
        } else if (strcmp(txt, "Tech") == 0) {
            is_user = false;
            is_tech = true;
            Serial.println("Mode: Tech");
        } else {
            is_user = false;
            is_tech = false;
            Serial.println("Mode: Debug");
        }
        // Load the main UI after setting
        setupMainUI();
    }, LV_EVENT_VALUE_CHANGED, NULL);
}



void setupMainUI() {
    lv_obj_t* tabview = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 30);

    // Add tabs
    lv_obj_t* home_tab = lv_tabview_add_tab(tabview, "Home");
    lv_obj_t* stat_tab = lv_tabview_add_tab(tabview, "Stat");
    lv_obj_t* setup_tab = lv_tabview_add_tab(tabview, "Setup");
    //            is_user = false;            is_tech = true;
    if (!is_user) {
      lv_obj_t* tech_tab = lv_tabview_add_tab(tabview, "Tech");
    }
    if  (!is_user && !is_tech){
        lv_obj_t* debug_tab = lv_tabview_add_tab(tabview, "Bug");
    }

    // Create controls for both tabs
    //create_controls_for_tab(home_tab, "home_tab Btn1", "home_tab Btn2");
    //create_controls_for_tab(tab2, "Tab2 Btn1", "Tab2 Btn2");
    create_controls_for_main(home_tab);
    
}


void return_to_main(lv_event_t *e) {
    // Get the object that triggered the event
    lv_obj_t *btn = lv_event_get_target(e);
    
    // Check if the event is a "clicked" event
    if(lv_event_get_code(e) == LV_EVENT_CLICKED) {
        // Assuming 'tabview' is your tabview object
        lv_tabview_set_act(tabview, 0, LV_ANIM_OFF); // Set the active tab to the first tab (index 0)
    }
}


void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}