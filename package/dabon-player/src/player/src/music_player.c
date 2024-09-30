#include <stdio.h>
#include "lvgl.h"
#include "music_player.h"
#include "main_selection_screen.h"
#include "file_explorer.h"
#include "spectrum.h"
#include "gstreamer.h"

#define BUTTONS_WIDTH         128
#define BUTTONS_HEIGHT        64

static lv_obj_t *screen = NULL,
                *top_screen, *folders_table, *songs_table, *spectrum,
                *slide_screen, *slide, *time_elapsed,
                *bottom_screen;
static lv_obj_t *play_btn, *bkwd_btn, *fwd_btn, *folder_btn, *menu_btn;
static gstreamer_t *gst;

static lv_timer_t *time_elapsed_timer;

static void button_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target_obj = lv_event_get_target(e);
    lv_state_t target_obj_state = lv_obj_get_state(target_obj);

    if (code == LV_EVENT_CLICKED) {
        if (target_obj == play_btn) {
            if ((target_obj_state & LV_STATE_CHECKED) == LV_STATE_CHECKED) {
                gstreamer_play_file(gst, file_explorer_get_selected_file(songs_table));
            } else {
                gstreamer_stop(gst);
            }
        } else if (target_obj == menu_btn) {
            gstreamer_stop(gst);
            main_selection_screen_start();
        } else if (target_obj == folder_btn) {
            if ((target_obj_state & LV_STATE_CHECKED) == LV_STATE_CHECKED) {
                lv_obj_set_width(spectrum, 0);
                lv_obj_set_width(folders_table, LV_PCT(50));
            } else {
                lv_obj_set_width(spectrum, LV_PCT(50));
                lv_obj_set_width(folders_table, 0);
            }
        } else if (target_obj == bkwd_btn) {
            uint32_t row, col;
            lv_table_get_selected_cell(songs_table, &row, &col);
            if (row > 0) {
                row--;
                lv_table_set_selected_cell(songs_table, row, col, true);
            }
        } else if (target_obj == fwd_btn) {
            uint32_t row, col, max_rows;
            lv_table_get_selected_cell(songs_table, &row, &col);
            max_rows = lv_table_get_row_count(songs_table);
            if (row < max_rows) {
                row++;
                lv_table_set_selected_cell(songs_table, row, col, true);
            }
        }
    } else if ((code == LV_EVENT_STYLE_CHANGED) && (target_obj == play_btn)) {
        lv_obj_t *label = lv_obj_get_child(target_obj, 0);
        if (target_obj_state == LV_STATE_CHECKED) {
            lv_label_set_text(label, "pause");
        } else {
            lv_label_set_text(label, "play");
        }
    }
}

static void add_button(lv_obj_t *parent, lv_obj_t **btn, const char *text, bool is_checkable)
{
    *btn = lv_button_create(parent);
    lv_obj_set_size(*btn, BUTTONS_WIDTH, BUTTONS_HEIGHT);
    lv_obj_add_event_cb(*btn, button_event_handler, LV_EVENT_ALL, NULL);
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

static void songs_table_change_event_cb(lv_event_t * e)
{
    (void) e;
    char *selected_file = file_explorer_get_selected_file(songs_table);
    LV_LOG_WARN("Play file: %s", selected_file);
    gstreamer_play_file(gst, selected_file);

    lv_obj_set_state(play_btn, LV_STATE_CHECKED, true);
}

static void folders_table_change_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    uint32_t col, row;

    lv_table_get_selected_cell(obj, &row, &col);
    const char *cell_text = lv_table_get_cell_value(obj, row, col);

    file_explorer_move_to_folder(folders_table, cell_text, TRUE);
    file_explorer_move_to_folder(songs_table, file_explorer_get_current_folder(folders_table), FALSE);
}

static void time_elapsed_timer_cb(lv_timer_t *timer)
{
    int curr_sec, total_sec, ret;
    (void) timer;

    ret = gstreamer_get_progress(gst, &curr_sec, &total_sec);
    if (ret < 0) {
        lv_label_set_text(time_elapsed, "--:--/--:--");
        lv_slider_set_value(slide, 0, LV_ANIM_OFF);
    } else {
        lv_label_set_text_fmt(time_elapsed, "%02d:%02d/%02d:%02d",
                              curr_sec / 60, curr_sec % 60,
                              total_sec / 60, total_sec % 60);
        lv_slider_set_range(slide, 0, total_sec);
        lv_slider_set_value(slide, curr_sec, LV_ANIM_OFF);
    }
}

static void initialize_screen()
{
    screen = lv_obj_create(NULL);
    lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    lv_obj_set_style_bg_image_src(screen, "A:background.png", LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    top_screen = lv_obj_create(screen);
    lv_obj_set_size(top_screen, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(top_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(top_screen, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_flex_flow(top_screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(top_screen, 1);

    folders_table = file_explorer_create(top_screen);
    lv_obj_set_size(folders_table, 0, LV_PCT(100));
    lv_obj_set_style_bg_opa(folders_table, LV_OPA_TRANSP, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(folders_table, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(folders_table, folders_table_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_border_color(folders_table, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    file_explorer_show_folders(folders_table);

    songs_table = file_explorer_create(top_screen);
    lv_obj_set_size(songs_table, LV_PCT(50), LV_PCT(100));
    lv_obj_set_style_bg_opa(songs_table, LV_OPA_TRANSP, LV_PART_ITEMS | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(songs_table, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(songs_table, songs_table_change_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    spectrum = spectrum_create(top_screen);
    lv_obj_set_flex_grow(spectrum, 1);
    lv_obj_set_style_bg_opa(spectrum, LV_OPA_TRANSP, LV_STATE_DEFAULT);
    lv_obj_set_height(spectrum, LV_PCT(100));

    gstreamer_register_spectrum_cb(gst, (gstreamer_spectrum_cb_t) spectrum_set_magnitudes, spectrum);

    slide_screen = lv_obj_create(screen);
    lv_obj_set_style_bg_opa(slide_screen, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_size(slide_screen, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(slide_screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(slide_screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_ver(slide_screen, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    slide = lv_slider_create(slide_screen);
    lv_obj_set_height(slide, 5);
    lv_obj_set_flex_grow(slide, 1);

    time_elapsed = lv_label_create(slide_screen);
    lv_obj_set_style_border_color(time_elapsed, lv_palette_main(LV_PALETTE_CYAN), 0);
    lv_label_set_text(time_elapsed, "--:--/--:--");

    time_elapsed_timer = lv_timer_create(time_elapsed_timer_cb, 500, NULL);

    bottom_screen = lv_obj_create(screen);
    lv_obj_set_size(bottom_screen, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_clear_flag(bottom_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(bottom_screen, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(bottom_screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bottom_screen, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(bottom_screen, 0, 0);
    lv_obj_set_style_bg_opa(bottom_screen, LV_OPA_TRANSP, LV_STATE_DEFAULT);

    add_button(bottom_screen, &folder_btn, "change folder", true);
    add_button(bottom_screen, &bkwd_btn, "back", false);
    add_button(bottom_screen, &play_btn, "play", true);
    add_button(bottom_screen, &fwd_btn, "forward", false);
    add_button(bottom_screen, &menu_btn, "menu", false);
}

void music_player_start()
{
    if (screen == NULL) {
        gst = gstreamer_init(GSTREAMER_TYPE_FILE_PLAYER, SPECTRUM_BARS_COUNTS);
        if (!gst) {
            LV_LOG_ERROR("gstreamer initialization failed");
        }
        gstreamer_set_spectrum_bands(gst, SPECTRUM_BARS_COUNTS);
        initialize_screen();
    } else {
        lv_scr_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_LEFT, 500, 0, false);
    }
}
