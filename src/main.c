#include <stdio.h>
#include "time_manager.h"
#include "wifi/wifi_manager.h"
#include "app_switcher.h"

void app_main(void)
{
    wifi_manager_init();
    time_manager_init();

    app_switcher_init();

    while (1)
    {
        app_switcher_loop();
    }
}