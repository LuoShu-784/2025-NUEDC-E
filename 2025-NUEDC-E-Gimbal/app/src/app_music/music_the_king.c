/**
 * @file music_the_king.c
 * @brief 《The King》乐谱数据
 */

#include "app_music.h"

// 《The King》乐谱数据
// 原始 MIDI：The King_Melody_Chords_32s.mid
// BPM = 120, 4/4拍
// 一拍时长 = 60000ms / 120 = 500ms
// 按当前蜂鸣器单音播放顺序整理

#define BEAT_1  500      // 一拍
#define BEAT_2  1000     // 二拍
#define BEAT_1_2 250      // 半拍
#define BEAT_3_4 375      // 四分之三拍
#define BEAT_1_4 125      // 四分之一拍
#define N_G2    98       // 低音G2
#define N_AS2   117      // 低音A#2
#define N_C3    131      // 低音C3
#define N_DS3   156      // 低音D#3
#define N_G3    196      // 低音G3
#define N_AS3   233      // 低音A#3
#define N_C4    NOTE_C4  // 中音C
#define N_D4    NOTE_D4  // 中音D
#define N_DS4   NOTE_DS4 // 中音D#
#define N_F4    NOTE_F4  // 中音F
#define N_G4    NOTE_G4  // 中音G
#define N_C5    NOTE_C5  // 高音C
#define N_F5    NOTE_F5  // 高音F
#define N_G5    NOTE_G5  // 高音G
#define N_AS5   NOTE_AS5 // 高音A#5
#define N_C6    NOTE_C6  // 高音C

const music_note_t music_the_king[] = {
    // 第1句
    {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_C6, BEAT_1_2},

    // 第2句
    {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4},
    {N_F5, BEAT_1_4}, {N_G5, BEAT_1_2}, {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4},

    // 第3句
    {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4},
    {N_G5, BEAT_1_4}, {N_C6, BEAT_1_2}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4},

    // 第4句
    {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},
    {N_AS5, BEAT_1_4}, {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4},

    // 第5句
    {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},
    {N_C6, BEAT_1_2}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},

    // 第6句
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_2}, {N_C5, BEAT_1_4},
    {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4},

    // 第7句
    {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_C6, BEAT_1_2}, {N_G3, BEAT_1_4},
    {N_AS3, BEAT_1_4}, {N_C4, BEAT_1_4}, {N_DS4, BEAT_1_4}, {N_D4, BEAT_1_4},

    // 第8句
    {N_C4, BEAT_1_4}, {N_AS3, BEAT_1_4}, {N_C4, BEAT_1_2}, {N_G2, BEAT_1_2},
    {N_C3, BEAT_1_2}, {N_G2, BEAT_1_2}, {N_C3, BEAT_1_4}, {N_G3, BEAT_1_4},

    // 第9句
    {N_AS3, BEAT_1_4}, {N_C4, BEAT_1_4}, {N_DS4, BEAT_1_4}, {N_D4, BEAT_1_4},
    {N_C4, BEAT_1_4}, {N_AS3, BEAT_1_4}, {N_C4, BEAT_1_2}, {N_AS2, BEAT_1_2},

    // 第10句
    {N_DS3, BEAT_1_2}, {N_AS2, BEAT_1_2}, {N_G3, BEAT_3_4}, {N_AS3, BEAT_1_2},
    {N_C4, BEAT_1_2}, {N_DS4, BEAT_1_4}, {N_DS4, BEAT_1_2}, {N_F4, BEAT_1_2},

    // 第11句
    {N_F4, BEAT_1_2}, {N_DS4, BEAT_1_4}, {N_G4, BEAT_1_2}, {N_F4, BEAT_1_2},
    {N_DS4, BEAT_1_2}, {N_F4, BEAT_1_2}, {N_DS4, BEAT_1_4}, {N_G4, BEAT_1_2},

    // 第12句
    {N_DS4, BEAT_1_2}, {N_C4, BEAT_1_2}, {N_DS4, BEAT_1_4}, {N_G4, BEAT_1_2},
    {N_G4, BEAT_1_4}, {N_DS4, BEAT_1_2}, {N_C4, BEAT_1_2}, {N_AS3, BEAT_1_2},

    // 第13句
    {N_C4, BEAT_1_2}, {N_G2, BEAT_1_2}, {N_C3, BEAT_1_2}, {N_G2, BEAT_1_2},
    {N_C3, BEAT_1_4}, {N_G3, BEAT_1_4}, {N_AS3, BEAT_1_4}, {N_C4, BEAT_1_4},

    // 第14句
    {N_DS4, BEAT_1_4}, {N_D4, BEAT_1_4}, {N_C4, BEAT_1_4}, {N_AS3, BEAT_1_4},
    {N_C4, BEAT_1_2}, {N_AS2, BEAT_1_2}, {N_DS3, BEAT_1_2}, {N_AS2, BEAT_1_2},

    // 第15句
    {N_G3, BEAT_3_4}, {N_AS3, BEAT_1_2}, {N_C4, BEAT_1_2}, {N_DS4, BEAT_1_4},
    {N_DS4, BEAT_1_2}, {N_F4, BEAT_1_2}, {N_F4, BEAT_1_2}, {N_DS4, BEAT_1_4},

    // 第16句
    {N_G4, BEAT_1_2}, {N_F4, BEAT_1_2}, {N_DS4, BEAT_1_2}, {N_F4, BEAT_1_2},
    {N_DS4, BEAT_1_4}, {N_G4, BEAT_1_2}, {N_DS4, BEAT_1_2}, {N_C4, BEAT_1_2},

    // 第17句
    {N_DS4, BEAT_1_4}, {N_G4, BEAT_1_2}, {N_G4, BEAT_1_4}, {N_DS4, BEAT_1_2},
    {N_C4, BEAT_1_2}, {N_AS3, BEAT_1_2}, {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4},

    // 第18句
    {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4},
    {N_G5, BEAT_1_4}, {N_C6, BEAT_1_2}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4},

    // 第19句
    {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_2},
    {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},

    // 第20句
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_C6, BEAT_1_4},
    {N_C3, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},

    // 第21句
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4},
    {N_C5, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},

    // 第22句
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_C6, BEAT_1_4},
    {N_C3, BEAT_1_4}, {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4},

    // 第23句
    {N_AS5, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_2}, {N_C5, BEAT_1_4},
    {N_C6, BEAT_1_4}, {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_AS5, BEAT_1_4},

    // 第24句
    {N_F5, BEAT_1_4}, {N_G5, BEAT_1_4}, {N_C6, BEAT_2 + BEAT_1_4}, {N_C3, BEAT_1}

};

const uint16_t music_the_king_length = sizeof(music_the_king) / sizeof(music_note_t);
