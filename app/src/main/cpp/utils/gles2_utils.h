#pragma once

#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

GLuint esLoadShader(GLenum type, GLchar *shaderSrc);
GLuint esLoadProgram(GLchar *vertShaderSrc, GLchar *fragShaderSrc);