#include <android/log.h>
#include <android/input.h>
#include <android_native_app_glue.h>
#include "native_ui.hpp"
#include "seed/agent_kernel.hpp"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"SEED-X",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"SEED-X",__VA_ARGS__)
namespace {
struct AppState{seed::AgentKernel kernel;NativeUi ui;bool hasWindow{false};bool running{false};};
void handleCommand(android_app* app,int32_t cmd){auto* s=static_cast<AppState*>(app->userData);if(!s)return;switch(cmd){case APP_CMD_INIT_WINDOW:if(app->window){s->hasWindow=s->ui.initialize(app->window);if(s->hasWindow)s->ui.resize(ANativeWindow_getWidth(app->window),ANativeWindow_getHeight(app->window));}break;case APP_CMD_TERM_WINDOW:s->ui.shutdown();s->hasWindow=false;break;case APP_CMD_WINDOW_RESIZED:if(app->window&&s->hasWindow)s->ui.resize(ANativeWindow_getWidth(app->window),ANativeWindow_getHeight(app->window));break;case APP_CMD_PAUSE:s->kernel.pause();s->running=false;break;case APP_CMD_RESUME:if(s->kernel.status()==seed::AgentStatus::Paused){s->kernel.start();s->running=true;}break;case APP_CMD_DESTROY:s->kernel.stop();s->ui.shutdown();s->hasWindow=false;break;default:break;}}
int32_t handleInput(android_app*,AInputEvent* e){return AInputEvent_getType(e)==AINPUT_EVENT_TYPE_MOTION?1:0;}
}
void android_main(android_app* app){app_dummy();AppState s;app->userData=&s;app->onAppCmd=handleCommand;app->onInputEvent=handleInput;if(!s.kernel.initialize()){LOGE("Kernel initialization failed");return;}LOGI("SEED-X Phase 1 native runtime ready");while(!app->destroyRequested){int events=0;android_poll_source* source=nullptr;int timeout=s.hasWindow?0:-1;while(ALooper_pollOnce(timeout,nullptr,&events,reinterpret_cast<void**>(&source))>=0){if(source)source->process(app,source);if(app->destroyRequested)break;if(timeout==0)break;}if(s.hasWindow)s.ui.render(true,s.running);}s.kernel.stop();s.ui.shutdown();}
