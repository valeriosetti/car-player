#include <stdio.h>
#include <sys/stat.h>
#include "lvgl.h"
#include "fm_player.h"
#include "main_selection_screen.h"
#include "spectrum.h"
#include "gstreamer.h"
#include "si468x_fm.h"
#include "si468x_ext.h"
#include "si468x_utils.h"

#define BUTTONS_WIDTH         128
#define BUTTONS_HEIGHT        64

#define FW_PATH             "/usr/share/dabon-firmwares"
#define BOOTLOADER_PATH     FW_PATH "/rom00_patch.016.bin"
#define APP_PATH            FW_PATH "/fmhd_radio_5_0_4.bin"

static lv_obj_t *screen = NULL,
                *top_screen, *spectrum, *freq_arc, *freq_label,
                *bottom_screen;
static lv_obj_t *bkwd_btn, *fwd_btn, *menu_btn;
static gstreamer_t *gst;

static void update_shown_frequency(void)
{
    struct si468x_FM_rsq_status rsq_status;
    si468x_fm_rsq_status(&rsq_status);
    int32_t new_freq = rsq_status.read_freq;
    lv_arc_set_value(freq_arc, new_freq/10);
    lv_obj_send_event(freq_arc, LV_EVENT_VALUE_CHANGED, NULL);
}

static void button_event_handler(lv_event_t *e)
{
    lv_obj_t *target_obj = lv_event_get_target(e);

    if (target_obj == menu_btn) {
        gstreamer_stop(gst);
        main_selection_screen_start();
    } else if (target_obj == bkwd_btn) {
        si468x_fm_seek_start(FM_SEEK_DOWN, 1);
        update_shown_frequency();
    } else if (target_obj == fwd_btn) {
        si468x_fm_seek_start(FM_SEEK_UP, 1);
        update_shown_frequency();
    }
}

static void add_button(lv_obj_t *parent, lv_obj_t **btn, const char *text, bool is_checkable)
{
    *btn = lv_button_create(parent);
    lv_obj_set_size(*btn, BUTTONS_WIDTH, BUTTONS_HEIGHT);
    lv_obj_add_event_cb(*btn, button_event_handler, LV_EVENT_CLICKED, NULL);
    if (is_checkable) {
        lv_obj_add_flag(*btn, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_set_style_bg_color(*btn, lv_palette_main(LV_PALETTE_GREEN),
                                  LV_PART_MAIN | LV_STATE_CHECKED);
    }

    lv_obj_t *label = lv_label_create((lv_obj_t *) *btn);
    lv_label_set_text_fmt(label, "%s", text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
}

static void arc_event_handler(lv_event_t *e)
{
    (void) e;

    int32_t value = lv_arc_get_value(freq_arc);
    lv_label_set_text_fmt(freq_label, "%d.%d MHz", value/10, value % 10);
}

static void initialize_screen()
{
    screen = lv_obj_create(NULL);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    lv_obj_set_style_bg_image_src(screen, "A:background.png", LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    top_screen = lv_obj_create(screen);
    lv_obj_clear_flag(top_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(top_screen, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(top_screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_width(top_screen, LV_PCT(100));
    lv_obj_set_flex_grow(top_screen, 1);

    spectrum = spectrum_create(top_screen);
    lv_obj_set_size(spectrum, LV_PCT(50), LV_PCT(100));
    lv_obj_set_style_bg_opa(spectrum, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_height(spectrum, LV_PCT(100));

    gstreamer_register_spectrum_cb(gst, (gstreamer_spectrum_cb_t) spectrum_set_magnitudes, spectrum);

    freq_arc = lv_arc_create(top_screen);
    lv_obj_set_size(freq_arc, LV_PCT(50), LV_PCT(100));
    lv_arc_set_rotation(freq_arc, 135);
    lv_arc_set_bg_angles(freq_arc, 0, 270);
    lv_arc_set_range(freq_arc, 875, 1080);

    lv_obj_add_event_cb(freq_arc, arc_event_handler, LV_EVENT_VALUE_CHANGED, NULL);

    freq_label = lv_label_create(freq_arc);
    lv_obj_set_style_text_font(freq_label, &lv_font_montserrat_32, LV_STATE_DEFAULT);
    lv_obj_center(freq_label);

    update_shown_frequency();

    bottom_screen = lv_obj_create(screen);
    lv_obj_set_size(bottom_screen, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(bottom_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(bottom_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bottom_screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(bottom_screen, 0, 0);
    lv_obj_set_style_bg_opa(bottom_screen, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    add_button(bottom_screen, &bkwd_btn, "back", false);
    add_button(bottom_screen, &fwd_btn, "forward", false);
    add_button(bottom_screen, &menu_btn, "menu", false);
}

void fm_player_start()
{
    int ret;
    uint8_t *bootloader = NULL, *app = NULL;
    size_t bootloader_size, app_size;

    ret = read_file_to_buffer(BOOTLOADER_PATH, &bootloader, &bootloader_size);
    if (ret != 0) {
        LV_LOG_ERROR("Failed to read bootloader image");
        free(app);
        free(bootloader);
        return;
    }
    ret = read_file_to_buffer(APP_PATH, &app, &app_size);
    if (ret != 0) {
        LV_LOG_ERROR("Failed to read app image");
        free(app);
        free(bootloader);
        return;
    }
    ret = si468x_start_image(bootloader, bootloader_size, app, app_size);
    if (ret != 0) {
        LV_LOG_ERROR("Failed to launch FM app");
        free(app);
        free(bootloader);
        return;
    }
    free(app);
    free(bootloader);

    if (screen == NULL) {
        gst = gstreamer_init(GSTREAMER_TYPE_RADIO_PLAYER, SPECTRUM_BARS_COUNTS);
        if (!gst) {
            LV_LOG_ERROR("gstreamer initialization failed");
            return;
        }
        gstreamer_play(gst);
        gstreamer_set_spectrum_bands(gst, SPECTRUM_BARS_COUNTS);
        initialize_screen();
    } else {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    }
}
