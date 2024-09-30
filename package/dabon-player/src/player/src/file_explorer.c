#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "lvgl.h"
#include "file_explorer.h"

/* Ignore "." folder and hidden files/folders. */
static bool ignore_entry(char* entry_name)
{
    if (strcmp(entry_name, ".") == 0) {
        return 1;
    } else if (strcmp(entry_name, "..") == 0) {
        return 0;
    } else if ((strlen(entry_name) > 1) && (entry_name[0] == '.')) {
        return 1;
    }
    return 0;
}

static int items_comparator(const void *_item1, const void *_item2)
{
    folder_item_t * item1 = (folder_item_t *) _item1;
    folder_item_t * item2 = (folder_item_t *) _item2;

    return strcmp(item1->name, item2->name);
}

static int update_file_list(lv_obj_t *obj)
{
    struct dirent *dir_item;
    file_explorer_t *explorer = (file_explorer_t *) obj;

    if (explorer->items_list != NULL) {
        free(explorer->items_list);
        explorer->items_count = 0;
    }

    rewinddir(explorer->current_dir);
    while ((dir_item = readdir(explorer->current_dir)) != NULL) {
        explorer->items_count++;
    }

    explorer->items_list = calloc(explorer->items_count, sizeof(folder_item_t));
    if (explorer->items_list == NULL) {
        LV_LOG_ERROR("Unable to allocate memory for list");
        return -ENOMEM;
    }

    rewinddir(explorer->current_dir);
    int i = 0;
    folder_item_t *item;
    while ((dir_item = readdir(explorer->current_dir)) != NULL) {
        item = &(explorer->items_list[i]);
        strcpy(item->name, dir_item->d_name);
        item->hide = ignore_entry(dir_item->d_name);
        item->is_folder = (dir_item->d_type == DT_DIR);
        i++;
    }

    qsort(explorer->items_list, explorer->items_count, sizeof(folder_item_t),
         (__compar_fn_t) items_comparator);

    return 0;
}

static void refresh_list(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;
    size_t row = 0;
    folder_item_t *item;

    lv_table_set_row_count(obj, 0);
    for (size_t i = 0; i < explorer->items_count; i++) {
        item = &(explorer->items_list[i]);
        if (item->hide) {
            continue;
        }
        if (explorer->is_showing_folders == item->is_folder) {
            lv_table_set_cell_value_fmt(obj, row, 0, "%s", item->name);
            row++;
        }
    }

    lv_table_set_selected_cell(obj, 0, 0, false);
}

void file_explorer_show_files(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;

    explorer->is_showing_folders = 0;
    refresh_list(obj);
}

void file_explorer_show_folders(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;

    explorer->is_showing_folders = 1;
    refresh_list(obj);
}

bool file_explorer_is_showing_folders(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;
    return explorer->is_showing_folders;
}

void file_explorer_move_to_folder(lv_obj_t *obj, const char *folder_name, bool is_relative)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;
    DIR *tmp_dir;

    if (is_relative) {
        if (strcmp(folder_name, ".") == 0) {
            /* Nothing to do in this case */
            return;
        } else if (strcmp(folder_name, "..") == 0) {
            size_t curr_path_len = strlen(explorer->current_path);
            char *p = explorer->current_path + curr_path_len - 1;
            if (curr_path_len <= 1) {
                /* Nothing to do */
                return;
            }
            do {
                *p = 0;
                p--;
                curr_path_len--;
            } while ((*p != '/') && (curr_path_len > 1));
        } else {
            strcat(explorer->current_path, folder_name);
            if (explorer->current_path[strlen(explorer->current_path) - 1] != '/') {
                strcat(explorer->current_path, "/");
            }
        }
    } else {
        memset(explorer->current_path, 0, sizeof(explorer->current_path));
        strcpy(explorer->current_path, folder_name);
    }

    LV_LOG_WARN("current path = %s", explorer->current_path);

    /* Check that the specified folder exists */
    tmp_dir = opendir(explorer->current_path);
    if (tmp_dir == NULL) {
        LV_LOG_ERROR("Unable to access path %s", explorer->current_path);
        return;
    }
    /* Close current_dir in case it was already opened */
    if (explorer->current_dir != NULL) {
        closedir(explorer->current_dir);
    }
    explorer->current_dir = tmp_dir;

    if (update_file_list(obj) != 0) {
        LV_LOG_ERROR("Failed to update file/folder list");
        return;
    }

    refresh_list(obj);
}

char *file_explorer_get_current_folder(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;
    return explorer->current_path;
}

char *file_explorer_get_selected_file(lv_obj_t *obj)
{
    file_explorer_t *explorer = (file_explorer_t *) obj;
    return explorer->selected_file;
}

void file_explorer_constructor_cb(const lv_obj_class_t *class_p, lv_obj_t *obj)
{
    (void) class_p;

    file_explorer_move_to_folder(obj, FILE_EXPLORER_DEFAULT_PATH, true);
}

void file_explorer_event_cb(const lv_obj_class_t *class_p, lv_event_t *e)
{
    lv_result_t res = lv_obj_event_base(class_p, e);
    if (res != LV_RESULT_OK) {
        return;
    }

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *target_obj = lv_event_get_target_obj(e);

    if (code == LV_EVENT_DRAW_TASK_ADDED) {
        lv_draw_task_t *draw_task = lv_event_get_draw_task(e);
        lv_draw_dsc_base_t *draw_dsc = lv_draw_task_get_draw_dsc(draw_task);
        /* Only act if cells are drawn */
        if (draw_dsc->part == LV_PART_ITEMS) {
            if (draw_task->type == LV_DRAW_TASK_TYPE_BORDER) {
                uint32_t selected_row, selected_col;
                lv_table_get_selected_cell(target_obj, &selected_row, &selected_col);
                /* draw_dsc->id1 is the row being drawn */
                if (draw_dsc->id1 == selected_row) {
                    lv_draw_border_dsc_t *border_dsc = lv_draw_task_get_border_dsc(draw_task);
                    border_dsc->width = 5;
                    border_dsc->color = lv_palette_main(LV_PALETTE_BLUE);
                    border_dsc->opa = LV_OPA_COVER;
                }
            }
        }
    } else if (code == LV_EVENT_SIZE_CHANGED) {
        int32_t width = lv_obj_get_content_width(target_obj);
        lv_table_set_column_width(target_obj, 0, width);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        file_explorer_t *explorer = (file_explorer_t *) target_obj;
        uint32_t row, col;
        lv_table_get_selected_cell(target_obj, &row, &col);
        const char *cell_text = lv_table_get_cell_value(target_obj, row, col);
        memset(explorer->selected_file, 0, sizeof(explorer->selected_file));
        char *p = explorer->selected_file;
        strcpy(p, explorer->current_path);
        p += strlen(explorer->current_path);
        strcpy(p, cell_text);
    }
}

const lv_obj_class_t file_explorer_class = {
    .base_class = &lv_table_class,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(file_explorer_t),
    .constructor_cb = file_explorer_constructor_cb,
    .event_cb = file_explorer_event_cb,
    .theme_inheritable = LV_OBJ_CLASS_THEME_INHERITABLE_TRUE,
    .name = "file_explorer",
};

lv_obj_t * file_explorer_create(lv_obj_t *parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(&file_explorer_class, parent);
    lv_obj_class_init_obj(obj);

    lv_obj_add_flag(obj, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);

    return obj;
}
