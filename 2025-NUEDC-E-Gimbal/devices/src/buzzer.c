#include "buzzer.h"
#include "main.h"
#include <stdbool.h>

extern TIM_HandleTypeDef htim4;

static struct {
    const music_note_t *music;
    uint16_t length;
    uint16_t current_index;
    uint32_t note_start_tick;
    bool playing;
} buzzer_state = {0};

void buzzer_init(void)
{
    HAL_TIM_Base_Start(&htim4);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
 }

void buzzer_play_note(note_t note)
{
    if (note == NOTE_SILENT || note == 0) {
        buzzer_stop();
        return;
    }

    // TIM4时钟：84MHz，预分频后：1MHz
    // ARR = 1000000 / freq - 1
    uint32_t arr = 1000000 / note - 1;

    // 停止PWM输出
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);

    // 修改ARR和CCR
    __HAL_TIM_SET_AUTORELOAD(&htim4, arr);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, arr / 2);

    // 生成更新事件并重启PWM
    htim4.Instance->EGR = TIM_EGR_UG;
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

void buzzer_stop(void)
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 0);
}

void buzzer_play_music(const music_note_t *music, uint16_t length)
{
    buzzer_state.music = music;
    buzzer_state.length = length;
    buzzer_state.current_index = 0;
    buzzer_state.note_start_tick = HAL_GetTick();
    buzzer_state.playing = true;

    if (length > 0) {
        buzzer_play_note(music[0].note);
    }
}

bool buzzer_is_playing(void)
{
    return buzzer_state.playing;
}

void buzzer_update(void)
{
    if (!buzzer_state.playing) {
        return;
    }

    uint32_t current_tick = HAL_GetTick();
    uint32_t elapsed = current_tick - buzzer_state.note_start_tick;

    if (elapsed >= buzzer_state.music[buzzer_state.current_index].duration) {
        buzzer_state.current_index++;

        if (buzzer_state.current_index >= buzzer_state.length) {
            buzzer_stop();
            buzzer_state.playing = false;
            return;
        }

        buzzer_state.note_start_tick = current_tick;
        buzzer_play_note(buzzer_state.music[buzzer_state.current_index].note);
    }
}
