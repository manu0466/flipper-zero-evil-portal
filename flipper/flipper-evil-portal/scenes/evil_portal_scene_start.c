#include "../evil_portal_app_i.h"
#include <dialogs/dialogs.h>

// For each command, define whether additional arguments are needed
// (enabling text input to fill them out), and whether the console
// text box should focus at the start of the output or the end
typedef enum { NO_ARGS = 0, INPUT_ARGS, TOGGLE_ARGS } InputArgs;

typedef enum {
  FOCUS_CONSOLE_END = 0,
  FOCUS_CONSOLE_START,
  FOCUS_CONSOLE_TOGGLE
} FocusConsole;

#define SHOW_STOPSCAN_TIP (true)
#define NO_TIP (false)

#define MAX_OPTIONS (9)
typedef struct {
  const char *item_string;
  const char *options_menu[MAX_OPTIONS];
  int num_options_menu;
  const char *actual_commands[MAX_OPTIONS];
  InputArgs needs_keyboard;
  FocusConsole focus_console;
  bool show_stopscan_tip;
  Evil_PortalCustomEvent event;
} Evil_PortalItem;

// NUM_MENU_ITEMS defined in evil_portal_app_i.h - if you add an entry here,
// increment it!
const Evil_PortalItem items[NUM_MENU_ITEMS] = {
    // send command
    {"Start portal",
     {""},
     1,
     {SET_HTML_CMD},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     SHOW_STOPSCAN_TIP,
     Evil_PortalEventStartConsole},

    // stop portal
    {"Stop portal",
     {""},
     1,
     {RESET_CMD},
     NO_ARGS,
     FOCUS_CONSOLE_START,
     SHOW_STOPSCAN_TIP,
     Evil_PortalEventStartConsole},

    // console
    {"Save logs",
     {""},
     1,
     {"savelogs"},
     NO_ARGS,
     FOCUS_CONSOLE_START,
     SHOW_STOPSCAN_TIP,
     Evil_PortalEventStartConsole},

    // select html
    {"Select HTML",
     {""},
     1,
     {},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP,
     Evil_PortalEventSelectHTML},

    // select html
    {"Select AP Name",
     {""},
     1,
     {},
     NO_ARGS,
     FOCUS_CONSOLE_END,
     NO_TIP,
     Evil_PortalEventSelectAPName},

    // help
    {"Help",
     {""},
     1,
     {"help"},
     NO_ARGS,
     FOCUS_CONSOLE_START,
     SHOW_STOPSCAN_TIP,
     Evil_PortalEventStartConsole},
};

static void evil_portal_scene_start_var_list_enter_callback(void *context,
                                                            uint32_t index) {
  furi_assert(context);
  Evil_PortalApp *app = context;

  furi_assert(index < NUM_MENU_ITEMS);
  const Evil_PortalItem *item = &items[index];

  const int selected_option_index = app->selected_option_index[index];
  furi_assert(selected_option_index < item->num_options_menu);
  app->selected_tx_string = item->actual_commands[selected_option_index];
  app->is_command = true;
  app->is_custom_tx_string = false;
  app->selected_menu_index = index;
  app->focus_console_start = (item->focus_console == FOCUS_CONSOLE_TOGGLE)
                                 ? (selected_option_index == 0)
                                 : item->focus_console;
  app->show_stopscan_tip = item->show_stopscan_tip;

  view_dispatcher_send_custom_event(app->view_dispatcher, item->event);
}

static void
evil_portal_scene_start_var_list_change_callback(VariableItem *item) {
  furi_assert(item);

  Evil_PortalApp *app = variable_item_get_context(item);
  furi_assert(app);

  const Evil_PortalItem *menu_item = &items[app->selected_menu_index];
  uint8_t item_index = variable_item_get_current_value_index(item);
  furi_assert(item_index < menu_item->num_options_menu);
  variable_item_set_current_value_text(item,
                                       menu_item->options_menu[item_index]);
  app->selected_option_index[app->selected_menu_index] = item_index;
}

void evil_portal_scene_start_on_enter(void *context) {
  Evil_PortalApp *app = context;
  VariableItemList *var_item_list = app->var_item_list;

  variable_item_list_set_enter_callback(
      var_item_list, evil_portal_scene_start_var_list_enter_callback, app);

  VariableItem *item;
  for (int i = 0; i < NUM_MENU_ITEMS; ++i) {
    item = variable_item_list_add(
        var_item_list, items[i].item_string, items[i].num_options_menu,
        evil_portal_scene_start_var_list_change_callback, app);
    variable_item_set_current_value_index(item, app->selected_option_index[i]);
    variable_item_set_current_value_text(
        item, items[i].options_menu[app->selected_option_index[i]]);
  }

  variable_item_list_set_selected_item(
      var_item_list,
      scene_manager_get_scene_state(app->scene_manager, Evil_PortalSceneStart));

  view_dispatcher_switch_to_view(app->view_dispatcher,
                                 Evil_PortalAppViewVarItemList);
}

bool evil_portal_scene_start_on_event(void *context, SceneManagerEvent event) {
  UNUSED(context);
  Evil_PortalApp *app = context;
  bool consumed = false;

  if (event.type == SceneManagerEventTypeCustom) {
    if (event.event == Evil_PortalEventStartPortal) {
      scene_manager_set_scene_state(app->scene_manager, Evil_PortalSceneStart,
                                    app->selected_menu_index);
      scene_manager_next_scene(app->scene_manager,
                               Evil_PortalAppViewStartPortal);
    } else if (event.event == Evil_PortalEventStartKeyboard) {
      scene_manager_set_scene_state(app->scene_manager, Evil_PortalSceneStart,
                                    app->selected_menu_index);
    } else if (event.event == Evil_PortalEventStartConsole) {

      if (strcmp(items[app->selected_menu_index].actual_commands[0], SET_HTML_CMD) == 0) {
        // Show an error message if the html file is not present before starting the
        // evil portal.
        if (!storage_file_exists(app->storage, furi_string_get_cstr(app->config->html_file))) {
          FuriString *error_message = furi_string_alloc_printf("can't find html file %s on the sd card", furi_string_get_cstr(app->config->html_file));
          dialog_message_show_storage_error(app->dialogs, furi_string_get_cstr(error_message));
          furi_string_free(error_message);
          return true;
        }
      }

      scene_manager_set_scene_state(app->scene_manager, Evil_PortalSceneStart,
                                    app->selected_menu_index);
      scene_manager_next_scene(app->scene_manager,
                               Evil_PortalAppViewConsoleOutput);
    } else if (event.event == Evil_PortalEventSelectHTML) {
      FuriString* data_folder = furi_string_alloc_set(STORAGE_APP_DATA_PATH_PREFIX);
      DialogsFileBrowserOptions browser_options;

      dialog_file_browser_set_basic_options(&browser_options, ".html",  NULL);
      browser_options.base_path = STORAGE_APP_DATA_PATH_PREFIX;

      bool file_selected = dialog_file_browser_show(
          app->dialogs, app->config->html_file, data_folder, &browser_options);
      if (file_selected) {
        evil_portal_config_save(app->storage, app->config);
      }

      furi_string_free(data_folder);
    } else if (event.event == Evil_PortalEventSelectAPName) {
      scene_manager_set_scene_state(app->scene_manager, Evil_PortalSceneStart, app->selected_menu_index);
      scene_manager_next_scene(app->scene_manager, Evil_PortalSceneApName);
    }
    consumed = true;
  } else if (event.type == SceneManagerEventTypeTick) {
    app->selected_menu_index =
        variable_item_list_get_selected_item_index(app->var_item_list);
    consumed = true;
  }

  return consumed;
}

void evil_portal_scene_start_on_exit(void *context) {
  Evil_PortalApp *app = context;
  variable_item_list_reset(app->var_item_list);
}
