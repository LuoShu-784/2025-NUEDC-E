#include "app_buzzer.h"
#include "app_music.h"
#include "buzzer.h"
#include "main.h"

#include <stdbool.h>
#include <stddef.h>

static osEventFlagsId_t s_event = NULL;

typedef enum {
    APP_BUZZER_MODE_IDLE = 0,
    APP_BUZZER_MODE_BEEP,
    APP_BUZZER_MODE_LONG,
    APP_BUZZER_MODE_REPEAT,
    APP_BUZZER_MODE_MUSIC,
} app_buzzer_mode_t;

static app_buzzer_mode_t s_mode = APP_BUZZER_MODE_IDLE;
static uint32_t s_mode_tick = 0;
static uint32_t s_repeat_tick = 0;
static bool s_repeat_on = false;

static void play_rated_tone(void)
{
    buzzer_play_note((note_t)APP_BUZZER_RATED_FREQ_HZ);
}

static void stop_output(void)
{
    buzzer_stop_music();
    s_mode = APP_BUZZER_MODE_IDLE;
    s_repeat_on = false;
}

static void start_beep(void)
{
    buzzer_stop_music();
    play_rated_tone();
    s_mode = APP_BUZZER_MODE_BEEP;
    s_mode_tick = HAL_GetTick();
}

static void start_long(void)
{
    buzzer_stop_music();
    play_rated_tone();
    s_mode = APP_BUZZER_MODE_LONG;
}

static void start_repeat(void)
{
    buzzer_stop_music();
    play_rated_tone();
    s_mode = APP_BUZZER_MODE_REPEAT;
    s_repeat_on = true;
    s_repeat_tick = HAL_GetTick();
}

static void start_music(const music_note_t *music, uint16_t length)
{
    if (music == NULL || length == 0U) {
        stop_output();
        return;
    }

    buzzer_play_music(music, length);
    s_mode = APP_BUZZER_MODE_MUSIC;
}

static void handle_event(uint32_t flags)
{
    if (flags & BUZZER_MOD_OFF) {
        stop_output();
    } else if (flags & BUZZER_MOD_BEEP) {
        start_beep();
    } else if (flags & BUZZER_MOD_LONG) {
        start_long();
    } else if (flags & BUZZER_MOD_REPEAT) {
        start_repeat();
    } else if (flags & BUZZER_MOD_MUSIC_THE_KING) {
        start_music(music_the_king, music_the_king_length);
    } else if (flags & BUZZER_MOD_MUSIC_HEROISM) {
        start_music(music_heroism, music_heroism_length);
    }
}

static void update_beep(uint32_t now)
{
    if ((now - s_mode_tick) >= BUZZER_BEEP_MS) {
        stop_output();
    }
}

static void update_repeat(uint32_t now)
{
    uint32_t interval = s_repeat_on ? BUZZER_REPEAT_ON_MS : BUZZER_REPEAT_OFF_MS;

    if ((now - s_repeat_tick) < interval) {
        return;
    }

    s_repeat_tick = now;
    s_repeat_on = !s_repeat_on;

    if (s_repeat_on) {
        play_rated_tone();
    } else {
        buzzer_stop();
    }
}

static void update_music(void)
{
    buzzer_update();

    if (!buzzer_is_playing()) {
        s_mode = APP_BUZZER_MODE_IDLE;
    }
}

void buzzer_ctrl(uint32_t mode)
{
    mode &= BUZZER_EVT_ALL;

    if (s_event == NULL || mode == 0U) {
        return;
    }

    osEventFlagsClear(s_event, BUZZER_EVT_ALL);
    osEventFlagsSet(s_event, mode);
}

void buzzer_task(void *argument)
{
    UNUSED(argument);

    s_event = osEventFlagsNew(NULL);
    if (s_event == NULL) {
        while (1) {
            osDelay(1000);
        }
    }

    buzzer_init();
    buzzer_stop_music();

    while (1) {
        uint32_t flags = osEventFlagsWait(s_event, BUZZER_EVT_ALL, osFlagsWaitAny, APP_BUZZER_INTERVAL_MS);
        if ((flags & osFlagsError) == 0U) {
            handle_event(flags);
        }

        uint32_t now = HAL_GetTick();
        if (s_mode == APP_BUZZER_MODE_BEEP) {
            update_beep(now);
        } else if (s_mode == APP_BUZZER_MODE_REPEAT) {
            update_repeat(now);
        } else if (s_mode == APP_BUZZER_MODE_MUSIC) {
            update_music();
        }
    }
}
