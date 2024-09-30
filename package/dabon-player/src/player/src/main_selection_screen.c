#include <unistd.h>
#include <errno.h>
#include "lvgl.h"
#include "main_selection_screen.h"
#include "music_player.h"
#include "fm_player.h"
#include "utils.h"

#define BUTTONS_SIZE        128

enum button_id {
    MP3_BTN = 0,
    FM_RADIO_BTN,
    DAB_RADIO_BTN,
    BLUETOOTH_BTN,
    CONFIG_BTN,
    _BTN_COUNT
};

static lv_obj_t *screen = NULL;

static void event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    enum button_id btn_id = (enum button_id) lv_event_get_user_data(e);

    if(code == LV_EVENT_CLICKED) {
        switch (btn_id) {
            case MP3_BTN:
                music_player_start();
                break;
            case FM_RADIO_BTN:
                fm_player_start();
                break;
            default:
                break;
        }
    }
}

static void add_button(enum button_id id, lv_obj_t *parent, const char *image)
{
    lv_obj_t *btn_obj, *label_obj;

    btn_obj = lv_button_create(parent);
    lv_obj_set_size(btn_obj, BUTTONS_SIZE, BUTTONS_SIZE);
    lv_obj_add_event_cb(btn_obj, event_handler, LV_EVENT_ALL, (void *) id);

    label_obj = lv_label_create(btn_obj);
    lv_label_set_text_fmt(label_obj, "%s", image);
    lv_obj_set_width(label_obj, BUTTONS_SIZE);
    lv_obj_set_style_text_align(label_obj, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_obj, LV_ALIGN_CENTER, 0, 0);
}

static void initialize_screen()
{
    screen = lv_obj_create(NULL);
    lv_screen_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, true);

    lv_obj_set_style_bg_image_src(screen, "A:background.png", LV_STATE_DEFAULT);

    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    add_button(MP3_BTN, screen, "usb");
    add_button(FM_RADIO_BTN, screen, "fm radio");
    add_button(DAB_RADIO_BTN, screen, "dab radio");
    add_button(BLUETOOTH_BTN, screen, "bluetooth");
    add_button(CONFIG_BTN, screen, "config");
}

void main_selection_screen_start()
{
    if (screen == NULL) {
        initialize_screen();
    } else {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    }
}
