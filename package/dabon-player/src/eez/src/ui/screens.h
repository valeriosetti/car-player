#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Screens

enum ScreensEnum {
    _SCREEN_ID_FIRST = 1,
    SCREEN_ID_MAIN_SCREEN = 1,
    SCREEN_ID_CHANGE_FOLDER_SCREEN = 2,
    _SCREEN_ID_LAST = 2
};

typedef struct _objects_t {
    lv_obj_t *main_screen;
    lv_obj_t *change_folder_screen;
    lv_obj_t *obj0;
    lv_obj_t *clock_label;
    lv_obj_t *obj1;
    lv_obj_t *song_list_table;
    lv_obj_t *spectrogram_chart;
    lv_obj_t *obj2;
    lv_obj_t *folder_list_table;
} objects_t;

extern objects_t objects;

void create_screen_main_screen();
void tick_screen_main_screen();

void create_screen_change_folder_screen();
void tick_screen_change_folder_screen();

void tick_screen_by_id(enum ScreensEnum screenId);
void tick_screen(int screen_index);

void create_screens();

#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/