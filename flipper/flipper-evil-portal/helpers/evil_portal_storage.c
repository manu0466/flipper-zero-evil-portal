#include "evil_portal_storage.h"
#include <flipper_format/flipper_format.h>
#include <storage/storage.h>

#define EVIL_PORTAL_INDEX_SAVE_PATH PORTAL_FILE_DIRECTORY_PATH "/index.html"
#define EVIL_PORTAL_CONFIG_PATH PORTAL_FILE_DIRECTORY_PATH "/evil_portal.conf"

#define CONFIG_KEY_HTML "html"

static char *sequential_file_resolve_path(Storage *storage, const char *dir,
                                   const char *prefix, const char *extension) {
  if (storage == NULL || dir == NULL || prefix == NULL || extension == NULL) {
    return NULL;
  }

  char file_path[256];
  int file_index = 0;

  do {
    if (snprintf(file_path, sizeof(file_path), "%s/%s_%d.%s", dir, prefix,
                 file_index, extension) < 0) {
      return NULL;
    }
    file_index++;
  } while (storage_file_exists(storage, file_path));

  return strdup(file_path);
}

void write_logs(Storage *storage, FuriString *portal_logs) {
  if (!storage_file_exists(storage, EVIL_PORTAL_LOG_SAVE_PATH)) {
    storage_simply_mkdir(storage, EVIL_PORTAL_LOG_SAVE_PATH);
  }

  char *seq_file_path = sequential_file_resolve_path(
      storage, EVIL_PORTAL_LOG_SAVE_PATH, "log", "txt");

  File *file = storage_file_alloc(storage);

  if (storage_file_open(file, seq_file_path, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
    storage_file_write(file, furi_string_get_cstr(portal_logs), furi_string_utf8_length(portal_logs));
  }
  storage_file_close(file);
  storage_file_free(file);
}

Evil_PortalConfig_t * evil_portal_config_alloc() {
  Evil_PortalConfig_t *config = malloc(sizeof(Evil_PortalConfig_t));
  // Ensure that we have a valid instance.
  if (config == NULL) {
    return config;
  }

  config->html_file = furi_string_alloc();
  furi_assert(config->html_file, "html file string alloc failed");

  return config;
}

void evil_portal_config_free(Evil_PortalConfig_t *config) {
  if (config == NULL) {
    return;
  }

  furi_string_free(config->html_file);

  free(config);
}

void evil_portal_config_load(Storage *storage, Evil_PortalConfig_t *config) {
  furi_assert(storage, "storage is null");
  furi_assert(config, "config is null");

  FlipperFormat *flipperFormat = flipper_format_file_alloc(storage);

  if (storage_file_exists(storage, EVIL_PORTAL_CONFIG_PATH)) {
    flipper_format_file_open_existing(flipperFormat, EVIL_PORTAL_CONFIG_PATH);
  } else {
    flipper_format_file_open_new(flipperFormat, EVIL_PORTAL_CONFIG_PATH);
  }

  if (!flipper_format_read_string(flipperFormat, CONFIG_KEY_HTML, config->html_file)) {
    // Fallback to the default file
    furi_string_set_str(config->html_file, EVIL_PORTAL_INDEX_SAVE_PATH);
  }

  flipper_format_file_close(flipperFormat);
  flipper_format_free(flipperFormat);
}

bool evil_portal_config_save(Storage *storage, const Evil_PortalConfig_t *config) {
  furi_assert(storage, "storage is null");
  furi_assert(config, "config is null");

  FlipperFormat *flipperFormat = flipper_format_file_alloc(storage);

  if (!flipper_format_file_open_always(flipperFormat, EVIL_PORTAL_CONFIG_PATH)) {
    return false;
  }

  flipper_format_write_string(flipperFormat, CONFIG_KEY_HTML, config->html_file);

  flipper_format_file_close(flipperFormat);
  flipper_format_free(flipperFormat);

  return true;
}