#ifndef CONFIG_FILE_HANDLER_H
#define CONFIG_FILE_HANDLER_H

#include "../main.h"

// Returns 0 on success, nonzero on error
int read_config(GlobalConfig *cfg, const char *filename);
int write_config(const GlobalConfig *cfg, const char *filename);
int delete_config(GlobalConfig *cfg, const char *filename);
// Note: sqlDBname has been renamed to dbName throughout the codebase.

#endif // CONFIG_FILE_HANDLER_H
