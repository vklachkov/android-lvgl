#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include "panel.h"

void panel_main(int width, int height) {
    static lv_coord_t w = width - 220;
    static lv_coord_t h = height - 420;

    static int col_count = 5;
    static int row_count = 5;

    static int padding = 18;

    static lv_coord_t col_dsc[] = {
            static_cast<lv_coord_t>(w / col_count - padding),
            static_cast<lv_coord_t>(w / col_count - padding),
            static_cast<lv_coord_t>(w / col_count - padding),
            static_cast<lv_coord_t>(w / col_count - padding),
            static_cast<lv_coord_t>(w / col_count - padding),
            LV_COORD_MAX
    };

    static lv_coord_t row_dsc[] = {
            static_cast<lv_coord_t>(h / row_count - padding),
            static_cast<lv_coord_t>(h / row_count - padding),
            static_cast<lv_coord_t>(h / row_count - padding),
            static_cast<lv_coord_t>(h / row_count - padding),
            static_cast<lv_coord_t>(h / row_count - padding),
            LV_COORD_MAX
    };

    /*Create a container with grid*/
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, 0);
    lv_obj_set_size(cont, w, h);
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);

    lv_obj_t *obj;
    for (int y = 0; y < row_count; y++) {
        for (int x = 0; x < col_count; x++) {
            obj = lv_btn_create(cont);
            lv_obj_add_flag(obj, LV_OBJ_FLAG_CHECKABLE);
            lv_obj_set_grid_cell(
                    obj,
                    LV_GRID_ALIGN_STRETCH,
                    x, 1,
                    LV_GRID_ALIGN_STRETCH,
                    y, 1
            );
        }
    }
}