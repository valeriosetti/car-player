#include "lvgl.h"
#include <dirent.h>

#define FILE_EXPLORER_MAX_PATH_LENGTH         4096
#define FILE_EXPLORER_MAX_NAME_LENGTH         256
#define FILE_EXPLORER_DEFAULT_PATH            "/media/"

typedef struct {
    char name[FILE_EXPLORER_MAX_NAME_LENGTH];
    bool is_folder;
    bool hide;
} folder_item_t;

typedef struct {
    lv_table_t obj;
    char current_path[FILE_EXPLORER_MAX_PATH_LENGTH];
    char selected_file[FILE_EXPLORER_MAX_PATH_LENGTH];
    DIR *current_dir;
    folder_item_t *items_list;
    size_t items_count;
    bool is_showing_folders;
} file_explorer_t;

lv_obj_t * file_explorer_create(lv_obj_t *parent);
void file_explorer_move_to_folder(lv_obj_t *obj, const char *folder_name, bool is_absolute);
char *file_explorer_get_current_folder(lv_obj_t *obj);
char *file_explorer_get_selected_file(lv_obj_t *obj);
void file_explorer_show_folders(lv_obj_t *obj);
void file_explorer_show_files(lv_obj_t *obj);
bool file_explorer_is_showing_folders(lv_obj_t *obj);
