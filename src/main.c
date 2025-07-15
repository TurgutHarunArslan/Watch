#include <stdio.h>
#include "time_manager.h"
#include "wifi_manager.h"

void app_main(void)
{
    wifi_manager_init();
    time_manager_init();
}