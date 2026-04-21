#ifndef LAN_MANAGER_H
#define LAN_MANAGER_H

#include <Arduino.h>

void lan_init();
void lan_loop();
bool is_lan_connected();
void lan_manager_save_config();
void lan_manager_load_config();
void lan_check_internet();

// Non-blocking Task for Core 0
void lan_task_init();

#endif
