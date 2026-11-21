#include "native_ui.hpp"
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"SEED-X",__VA_ARGS__)
bool NativeUi::initialize(ANativeWindow* window){
 display_=eglGetDisplay(EGL_DEFAULT_DISPLAY);if(display_==EGL_NO_DISPLAY||!eglInitialize(display_,nullptr,nullptr)){LOGE("EGL initialization failed");return false;}
 const EGLint attrs[]={EGL_RENDERABLE_TYPE,EGL_OPENGL_ES3_BIT,EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_NONE};EGLConfig config{};EGLint count=0;
 if(!eglChooseConfig(display_,attrs,&config,1,&count)||count<1){LOGE("No EGL config");shutdown();return false;}
 const EGLint ctx[]={EGL_CONTEXT_CLIENT_VERSION,3,EGL_NONE};context_=eglCreateContext(display_,config,EGL_NO_CONTEXT,ctx);surface_=eglCreateWindowSurface(display_,config,window,nullptr);
 if(context_==EGL_NO_CONTEXT||surface_==EGL_NO_SURFACE||!eglMakeCurrent(display_,surface_,surface_,context_)){LOGE("EGL context creation failed");shutdown();return false;}
 eglSwapInterval(display_,1);glDisable(GL_DEPTH_TEST);return true;
}
void NativeUi::resize(int w,int h){width_=w;height_=h;glViewport(0,0,w,h);}
void NativeUi::render(bool ready,bool running){if(display_==EGL_NO_DISPLAY||surface_==EGL_NO_SURFACE)return;glClearColor(.035f,.055f,.075f,1.f);glClear(GL_COLOR_BUFFER_BIT);glEnable(GL_SCISSOR_TEST);glScissor(0,height_-12,width_,12);glClearColor(.12f,.78f,.48f,1.f);glClear(GL_COLOR_BUFFER_BIT);int margin=width_/14,cardW=width_-2*margin,cardH=height_/9;float tint=ready?.10f:.22f,state=running?.72f:.36f;for(int i=0;i<3;++i){int y=height_-height_/5-(i+1)*(cardH+margin/2);glScissor(margin,y,cardW,cardH);glClearColor(tint+(i==0?state*.12f:0.f),.12f+(i==1?state*.16f:0.f),.17f+(i==2?.12f:0.f),1.f);glClear(GL_COLOR_BUFFER_BIT);}glDisable(GL_SCISSOR_TEST);eglSwapBuffers(display_,surface_);}
void NativeUi::shutdown(){if(display_!=EGL_NO_DISPLAY){eglMakeCurrent(display_,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);if(context_!=EGL_NO_CONTEXT)eglDestroyContext(display_,context_);if(surface_!=EGL_NO_SURFACE)eglDestroySurface(display_,surface_);eglTerminate(display_);}display_=EGL_NO_DISPLAY;surface_=EGL_NO_SURFACE;context_=EGL_NO_CONTEXT;}
