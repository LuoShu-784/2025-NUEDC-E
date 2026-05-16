#ifndef __BUZZER_H
#define __BUZZER_H

#include <stdint.h>
#include <stdbool.h>

// 音符定义（频率 Hz）
typedef enum {
    NOTE_SILENT = 0,
    NOTE_C4  = 262,
    NOTE_D4  = 294,
    NOTE_E4  = 330,
    NOTE_F4  = 349,
    NOTE_G4  = 392,
    NOTE_A4  = 440,
    NOTE_B4  = 494,
    NOTE_C5  = 523,
    NOTE_D5  = 587,
    NOTE_E5  = 659,
    NOTE_F5  = 698,
    NOTE_G5  = 784,
    NOTE_A5  = 880,
    NOTE_B5  = 988,
    NOTE_C6  = 1047,
    NOTE_D6  = 1175,
    NOTE_E6  = 1319,
    NOTE_F6  = 1397,
    NOTE_G6  = 1568,
    NOTE_A6  = 1760,
    NOTE_B6  = 1976,
    NOTE_C7  = 2093,
    NOTE_D7  = 2349,

    // 升半音
    NOTE_CS4 = 277,
    NOTE_DS4 = 311,
    NOTE_FS4 = 370,
    NOTE_GS4 = 415,
    NOTE_AS4 = 466,
    NOTE_CS5 = 554,
    NOTE_DS5 = 622,
    NOTE_FS5 = 740,
    NOTE_GS5 = 831,
    NOTE_AS5 = 932,
    NOTE_CS6 = 1109,
    NOTE_DS6 = 1245,
    NOTE_FS6 = 1480,
    NOTE_GS6 = 1661,
    NOTE_AS6 = 1865,
} note_t;

typedef struct {
    note_t note;        // 音符频率
    uint16_t duration;  // 持续时间（ms）
} music_note_t;

void buzzer_init(void);
void buzzer_play_note(note_t note);
void buzzer_stop(void);
void buzzer_play_music(const music_note_t *music, uint16_t length);
bool buzzer_is_playing(void);
void buzzer_update(void);

#endif
