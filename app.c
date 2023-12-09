#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/scene_manager.h>
#include <gui/modules/button.h>

#define TAG "autoclicker-app"

typedef enum {
    AutoClickerScene_Main,
    AutoClickerScene_count
} AutoClickerScene;

typedef enum {
    AutoClickerView_Button,
    AutoClickerView_count
} AutoClickerView;

typedef struct {
    SceneManager* scene_manager;
    ViewDispatcher* view_dispatcher;
    Button* click_button;
    int click_count;
    bool autoclicking;
} AutoClickerApp;

typedef enum {
    AutoClickerEvent_StartClicking,
    AutoClickerEvent_StopClicking,
} AutoClickerEvent;

void autoclicker_button_callback(void* context, uint32_t index) {
    AutoClickerApp* app = context;
    switch(index) {
        case 0: // Click button pressed
            if (app->autoclicking) {
                scene_manager_handle_custom_event(app->scene_manager, AutoClickerEvent_StopClicking);
            } else {
                scene_manager_handle_custom_event(app->scene_manager, AutoClickerEvent_StartClicking);
            }
            break;
    }
}

void autoclicker_scene_on_enter_main(void* context) {
    AutoClickerApp* app = context;
    button_set_text(app->click_button, "Start Clicking");
    view_dispatcher_switch_to_view(app->view_dispatcher, AutoClickerView_Button);
}

bool autoclicker_scene_on_event_main(void* context, SceneManagerEvent event) {
    AutoClickerApp* app = context;
    bool consumed = false;

    switch(event.type) {
        case SceneManagerEventTypeCustom:
            switch(event.event) {
                case AutoClickerEvent_StartClicking:
                    app->autoclicking = true;
                    button_set_text(app->click_button, "Stop Clicking");
                    break;
                case AutoClickerEvent_StopClicking:
                    app->autoclicking = false;
                    button_set_text(app->click_button, "Start Clicking");
                    break;
            }
            consumed = true;
            break;
        default:
            consumed = false;
            break;
    }

    return consumed;
}

void autoclicker_scene_on_exit_main(void* context) {
    // Cleanup or additional actions upon exiting the main scene
}

void autoclicker_scene_manager_init(AutoClickerApp* app) {
    app->scene_manager = scene_manager_alloc(NULL, app); // No specific scene handlers for simplicity
}

void autoclicker_view_dispatcher_init(AutoClickerApp* app) {
    app->view_dispatcher = view_dispatcher_alloc();

    app->click_button = button_alloc();
    button_set_callback(app->click_button, autoclicker_button_callback, app);

    view_dispatcher_add_view(app->view_dispatcher, AutoClickerView_Button, button_get_view(app->click_button));
}

AutoClickerApp* autoclicker_app_init() {
    AutoClickerApp* app = malloc(sizeof(AutoClickerApp));
    autoclicker_scene_manager_init(app);
    autoclicker_view_dispatcher_init(app);
    return app;
}

void autoclicker_app_free(AutoClickerApp* app) {
    scene_manager_free(app->scene_manager);
    view_dispatcher_remove_view(app->view_dispatcher, AutoClickerView_Button);
    view_dispatcher_free(app->view_dispatcher);
    button_free(app->click_button);
    free(app);
}

void autoclicker_app_set_log_level() {
#ifdef FURI_DEBUG
    furi_log_set_level(FuriLogLevelTrace);
#else
    furi_log_set_level(FuriLogLevelInfo);
#endif
}

int32_t autoclicker_app(void* p) {
    UNUSED(p);
    autoclicker_app_set_log_level();

    FURI_LOG_I(TAG, "AutoClicker App starting...");
    AutoClickerApp* app = autoclicker_app_init();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);
    scene_manager_next_scene(app->scene_manager, AutoClickerScene_Main);
    FURI_LOG_D(TAG, "Starting dispatcher...");
    view_dispatcher_run(app->view_dispatcher);

    FURI_LOG_I(TAG, "AutoClicker App finishing...");
    furi_record_close(RECORD_GUI);
    autoclicker_app_free(app);
    return 0;
}
