#pragma once

#include <furi.h>
#include <storage/storage.h>

typedef struct {
    FuriString *html_file;
    FuriString *ap_name;
} Evil_PortalConfig_t;

void write_logs(Storage *storage, FuriString* portal_logs);

/**
 * Creates a new instance of `EvilPortalConfig_t`.
 * @return Returns a pointer to a new `EvilPortalConfig_t` instance.
 */
Evil_PortalConfig_t * evil_portal_config_alloc();

/**
 * Destroy an insatnce of `EvilPortalConfig_t`
 * @param config - The instance to destroy.
 */
void evil_portal_config_free(Evil_PortalConfig_t *config);

/**
 * Loads the application config from the storage.
 * @param storage - The storage instance.
 * @param config - Pointer where will be store the configurations.
 */
void evil_portal_config_load(Storage *storage, Evil_PortalConfig_t *config);

/**
 * Loads the application config on the storage.
 * @param storage - The storage instance.
 * @param config - Pointer to the configurations to save.
 */
bool evil_portal_config_save(Storage *storage, const Evil_PortalConfig_t *config);