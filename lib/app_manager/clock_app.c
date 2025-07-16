#include <app.h>
#include <sys/time.h>

static lv_obj_t *screen;
static lv_obj_t *label;

static void clock_init(App *self)
{
    screen = lv_obj_create(NULL);
    lv_scr_load(screen);

    label = lv_label_create(screen);
    lv_label_set_text(label, "2025-07-16");
    lv_obj_center(label);
}

static void clock_loop(App *self)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    time_t now = tv.tv_sec;
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d", &timeinfo);
    lv_label_set_text(label,strftime_buf);
}

static void clock_deinit(App *self)
{
    if (screen)
    {
        lv_obj_del(screen);
        screen = NULL;
    }
}

App ClockApp = {
    .init = clock_init,
    .deinit = clock_deinit,
    .loop = clock_loop,
};