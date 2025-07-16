#include <app.h>

extern App ClockApp;

static App *apps[] = {
    &ClockApp,
};

static const int app_count = sizeof(apps) / sizeof(apps[0]);
static App *current_app = NULL;

static void switch_app(int index)
{
    if (index < 0 || index >= app_count)
        return;

    if (current_app)
    {
        current_app->deinit(current_app);
    }

    current_app = apps[index];
    current_app->init(current_app);
}

void app_switcher_init(void)
{
    lv_init();

    switch_app(0);

}

void app_switcher_loop(void)
{
    if (current_app)
    {
        current_app->loop(current_app);
    }
    lv_task_handler();
}