#ifndef H_LVGL_GL
#define H_LVGL_GL

/* -----------------------------------------------------------------------------
 * INCLUDES
 */

#include "lvgl/lvgl.h"
#include "ui/panel.h"
#include "global.h"


/* -----------------------------------------------------------------------------
 * PROTOTYPES
 */

void LvInit(struct engine *engine, void (*draw_callback)(struct engine*, GLint, GLint, GLint, GLint));

void LvCreateGui();

#endif