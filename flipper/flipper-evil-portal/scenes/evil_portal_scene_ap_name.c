#include <furi.h>
#include "../evil_portal_app_i.h"

#define TAG "AP Name scene"

char AP_NAME_BUFFER[33];

static void evil_portal_scene_change_text_input_callback(void* context) {
    Evil_PortalApp *app = context;

    view_dispatcher_send_custom_event(app->view_dispatcher, Evil_PortalTextEditResult);
}

void evil_portal_scene_ap_name_on_enter(void *context) {
    Evil_PortalApp *app = context;
    TextInput* text_input = app->text_input;

    text_input_set_header_text(text_input, "AP Name/SSID");
    text_input_set_minimum_length(text_input, 2);

    // Initialize the buffer with the current AP name.
    snprintf(AP_NAME_BUFFER, sizeof(AP_NAME_BUFFER), "%s", furi_string_get_cstr(app->config->ap_name));

    text_input_set_result_callback(
        text_input,
        evil_portal_scene_change_text_input_callback,
        app,
        AP_NAME_BUFFER,
        sizeof(AP_NAME_BUFFER),
        true);

    view_dispatcher_switch_to_view(app->view_dispatcher, Evil_PortalAppViewApName);
}

bool evil_portal_scene_ap_name_on_event(void *context, SceneManagerEvent event) {
    Evil_PortalApp *app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        consumed = true;
        if(event.event == Evil_PortalTextEditResult) {
            if (strlen(AP_NAME_BUFFER) >= 2) {
                furi_string_set_str(app->config->ap_name, AP_NAME_BUFFER);
                app->config->changed = true;
                scene_manager_search_and_switch_to_previous_scene(
                    app->scene_manager, Evil_PortalAppViewVarItemList);
            }
        }
    }
    return consumed;
}

void evil_portal_scene_ap_name_on_exit(void *context) {
    Evil_PortalApp *app = context;
    TextInput* text_input = app->text_input;

    text_input_reset(text_input);
    if (app->config->changed) {
        evil_portal_config_save(app->storage, app->config);
        app->config->changed = false;
    }
}