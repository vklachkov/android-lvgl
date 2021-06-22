#ifndef H_GLOBAL
#define H_GLOBAL

/* -----------------------------------------------------------------------------
 * INCLUDES
 */

#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <jni.h>
#include <cerrno>
#include <cassert>
#include <unistd.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "utils/gles2_utils.h"

#include <android_native_app_glue.h>
#include "utils/log.h"

/* -----------------------------------------------------------------------------
 * CONSTS
 */

static const int BytesPerPixel = 3;


/* -----------------------------------------------------------------------------
 * STRUCTS
 */

/**
 * Saved state data
 */
struct saved_state {
    int32_t x;
    int32_t y;
    bool is_touched;
};

/**
 * Shared app state
 */
struct engine {
    struct android_app *app;
    struct saved_state state;

    // Screen dimens
    int32_t width;
    int32_t height;

    // OpenGL data
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    GLuint programObject;
    GLint positionLoc;
    GLint texCoordLoc;
    GLint samplerLoc;
    GLuint textureId;

    // Buffer for all graphics operations
    void *pixels;
};

#endif