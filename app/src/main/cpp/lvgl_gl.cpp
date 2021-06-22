#include "lvgl_gl.h"


/* -----------------------------------------------------------------------------
 * INTERNAL GLOBALS
 */

const uint32_t lv_buf_size = 4096 * 10;

static lv_disp_draw_buf_t lv_disp_buf;
static lv_color_t lv_buf_1[lv_buf_size];
static lv_color_t lv_buf_2[lv_buf_size];

static struct engine *engine = nullptr;

static void (*draw_func)(struct engine *, GLint, GLint, GLint, GLint);

/* -----------------------------------------------------------------------------
 * INTERNAL PROTOTYPES
 */

static void LvDisplayFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

static void PutPx(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);

static void LvTouchRead(lv_indev_drv_t *drv, lv_indev_data_t *data);


/* -----------------------------------------------------------------------------
 * IMPLEMENTATION
 */

void
LvInit(struct engine *eng, void (*draw_callback)(struct engine *, GLint, GLint, GLint, GLint)) {
    engine = eng;
    draw_func = draw_callback;

    // Initialize lvgl: memory manager, internal structs etc
    lv_init();

    // Initialize buffer
    lv_disp_draw_buf_init(&lv_disp_buf, lv_buf_1, lv_buf_2, lv_buf_size);

    // Initialize display driver
    {
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);

        disp_drv.hor_res = engine->width;
        disp_drv.ver_res = engine->height;
        disp_drv.draw_buf = &lv_disp_buf;

        // TODO Use acceleration structure for drawing
        disp_drv.gpu_fill_cb = nullptr;
        disp_drv.gpu_wait_cb = nullptr;
        disp_drv.flush_cb = LvDisplayFlush;

        lv_disp_drv_register(&disp_drv);
    }

    // Initialize input driver
    {
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);

        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = LvTouchRead;

        lv_indev_drv_register(&indev_drv);
    }
}

void LvCreateGui() {
    panel_main(engine->width, engine->height);
}


/* -----------------------------------------------------------------------------
 * INTERNAL
 */

static void LvDisplayFlush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    LOGI("LvDisplayFlush");
    LOGD("Y = %d..%d, X = %d..%d", area->y1, area->y2, area->x1, area->x2);

    auto h = area->y2 - area->y1;
    auto w = area->x2 - area->x1;

    // This is slowest method, but working properly :)
    for (int32_t y = 0; y <= h; y++) {
        for (int32_t x = 0; x <= w; x++) {
            PutPx(x, y,
                  color_p->ch.red,
                  color_p->ch.green,
                  color_p->ch.blue
            );
            color_p++;
        }
    }

    draw_func(engine, w, h + 1, area->x1, area->y1);

    // Inform the graphics library that we are ready with the flushing
    lv_disp_flush_ready(disp_drv);
}

static void PutPx(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    int i = (y * engine->width * BytesPerPixel) + (x * BytesPerPixel);
    auto *buffer = static_cast<unsigned char *>(engine->pixels);

    buffer[i] = r;  //  Red
    i++;
    buffer[i] = g;  //  Green
    i++;
    buffer[i] = b;  //  Blue
}

static void LvTouchRead(lv_indev_drv_t *drv, lv_indev_data_t *data) {
    if(engine->state.is_touched) {
        data->point.x = engine->state.x;
        data->point.y = engine->state.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}