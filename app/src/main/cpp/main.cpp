#include "main.h"

/* -----------------------------------------------------------------------------
 * OPEN GL
 * WARNING: MODIFY ENGINE STATE
 */

static void GlInfo() {
    //auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    //for (auto name : opengl_info) {
    //    LOGI("OpenGL Info: %s", glGetString(name));
    //}
}

static int GlInitialize(struct engine *engine) {
    EGLint major, minor, count, n, size, w, h, format;
    EGLDisplay display = nullptr;
    EGLSurface surface;
    EGLContext context;
    EGLConfig *configs;
    EGLConfig config = nullptr;
    EGLBoolean result = EGL_FALSE;

    static const EGLint config_attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    static const EGLint context_attribs[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    display = eglGetDisplay((EGLNativeDisplayType) display);
    if (display == EGL_NO_DISPLAY) {
        // TODO
        return -1;
    }

    result = eglInitialize(display, &major, &minor);
    if (result != EGL_TRUE) {
        // TODO
        return -1;
    }

    LOGI("EGL successfuly initialized!");
    LOGI("EGL major: %d, minor %d", major, minor);

    eglGetConfigs(display, nullptr, 0, &count);
    printf("EGL has %d configs", count);

    configs = new EGLConfig[count]();
    eglChooseConfig(display, config_attribs, configs, count, &n);

    for (int i = 0; i < n; i++) {
        eglGetConfigAttrib(display, configs[i], EGL_BUFFER_SIZE, &size);
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &size);

        LOGD("Buffer size for config %d is %d", i, size);
        LOGD("Red size for config %d is %d", i, size);

        // just choose the first one
        config = configs[i];
        break;
    }

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribs);

    result = eglMakeCurrent(display, surface, surface, context);
    if (result == EGL_FALSE) {
        // TODO
        return -1;
    }

    GlInfo();

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->width = w;
    engine->height = h;
    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->pixels = new GLubyte[w * h * BytesPerPixel];

    return 0;
}

static GLuint GlCreateTexture() {
    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return texId;
}

static void GlCreateShader(struct engine *engine) {
    GLchar vShaderStr[] =
            "attribute vec4 a_position;   \n"
            "attribute vec2 a_texCoord;   \n"
            "varying vec2 v_texCoord;     \n"
            "void main()                  \n"
            "{                            \n"
            "   gl_Position = a_position; \n"
            "   v_texCoord = a_texCoord;  \n"
            "}                            \n";

    GLchar fShaderStr[] =
            "precision mediump float;                            \n"
            "varying vec2 v_texCoord;                            \n"
            "uniform sampler2D s_texture;                        \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
            "}                                                   \n";

    // Load the shaders and get a linked program object
    engine->programObject = esLoadProgram(vShaderStr, fShaderStr);

    // Get the attribute locations
    engine->positionLoc = glGetAttribLocation(engine->programObject, "a_position");
    engine->texCoordLoc = glGetAttribLocation(engine->programObject, "a_texCoord");

    // Get the sampler location
    engine->samplerLoc = glGetUniformLocation(engine->programObject, "s_texture");

    // Load the texture
    engine->textureId = GlCreateTexture();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
}

static void GlDrawTextureFull(struct engine *engine) {
    LOGI("Draw width=%d, height=%d...\n", engine->width, engine->height);

    glViewport(0, 0, engine->width, engine->height);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(engine->programObject);

    // Load the vertex position
    glVertexAttribPointer(engine->positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), Vertices);

    // Load the texture coordinate
    glVertexAttribPointer(engine->texCoordLoc, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &Vertices[3]);

    glEnableVertexAttribArray(engine->positionLoc);
    glEnableVertexAttribArray(engine->texCoordLoc);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, engine->textureId);

    // Pass texture
    glTexImage2D(
            GL_TEXTURE_2D,
            0,  //  Level
            GL_RGB,
            engine->width,  //  Width
            engine->height,  //  Height
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            engine->pixels
    );

    glUniform1i(engine->samplerLoc, 0);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    eglSwapBuffers(engine->display, engine->surface);
}

static void GlDrawTexturePart(
        struct engine *engine,
        GLint width,
        GLint height,
        GLint xOffset,
        GLint yOffset
) {
    LOGI("Draw part xOffset=%d, yOffset=%d, width=%d, height=%d...\n", xOffset, yOffset, width,
         height);

    glUseProgram(engine->programObject);

    // Load the vertex position
    glVertexAttribPointer(engine->positionLoc, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), Vertices);

    // Load the texture coordinate
    glVertexAttribPointer(engine->texCoordLoc, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &Vertices[3]);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, engine->textureId);

    // Pass texture
    glTexSubImage2D(
            GL_TEXTURE_2D,
            0,  //  Level
            xOffset,
            yOffset,
            width,
            height,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            engine->pixels
    );

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    eglSwapBuffers(engine->display, engine->surface);
}


/* -----------------------------------------------------------------------------
 * ANDROID SPECIFIC
 */

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine *engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    auto *engine = (struct engine *) app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        engine->state.x = AMotionEvent_getX(event, 0);
        engine->state.y = AMotionEvent_getY(event, 0);
        engine->state.is_touched = !engine->state.is_touched;
        return 1;
    }

    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    auto *engine = (struct engine *) app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state *) engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;

        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                GlInitialize(engine);
                GlCreateShader(engine);
                GlDrawTextureFull(engine);

                LvInit(engine, GlDrawTexturePart);
                LvCreateGui();
            }
            break;

        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
            break;

        case APP_CMD_LOST_FOCUS:
            break;

        default:
            break;
    }
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app *state) {
    struct engine engine{};

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    if (state->savedState != nullptr) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state *) state->savedState;
    }

    // loop waiting for stuff to do.
    while (true) {
        // Read all pending events.
        //int ident;
        int events;
        struct android_poll_source *source;

        lv_task_handler();

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((/*ident = */ALooper_pollAll(0, nullptr, &events, (void **) &source)) >= 0) {

            // Process this event.
            if (source != nullptr) {
                source->process(state, source);
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
    }
}