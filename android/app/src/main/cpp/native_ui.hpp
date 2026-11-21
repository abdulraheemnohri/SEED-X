#pragma once
#include <android/native_window.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
class NativeUi final {
public:
 bool initialize(ANativeWindow* window); void resize(int width,int height); void render(bool ready,bool running); void shutdown();
private:
 EGLDisplay display_{EGL_NO_DISPLAY}; EGLSurface surface_{EGL_NO_SURFACE}; EGLContext context_{EGL_NO_CONTEXT}; int width_{0},height_{0};
};
