#include "buzzer.h"

// 《英雄主义》乐谱数据
// BPM = 195, 1=G, 4/4拍
// 一拍时长 = 60000ms / 195 ≈ 308ms

#define BEAT_1    308   // 一拍
#define BEAT_2    616   // 二拍
#define BEAT_4    1232  // 四拍
#define BEAT_1_2  154   // 半拍
#define BEAT_1_4  77    // 四分之一拍
#define BEAT_1_8  38    // 八分音符

// 1=G调音符映射
#define N_1_L  NOTE_G4   // 低音1
#define N_2_L  NOTE_A4   // 低音2
#define N_3_L  NOTE_B4   // 低音3
#define N_4_L  NOTE_C5   // 低音4
#define N_5_L  NOTE_D5   // 低音5
#define N_6_L  NOTE_E5   // 低音6
#define N_7_L  NOTE_FS5  // 低音7
#define N_1    NOTE_G5   // 中音1
#define N_2    NOTE_A5   // 中音2
#define N_3    NOTE_B5   // 中音3
#define N_4    NOTE_C6   // 中音4
#define N_5    NOTE_D6   // 中音5
#define N_6    NOTE_E6   // 中音6
#define N_7    NOTE_FS6  // 中音7
#define N_1_H  NOTE_G6   // 高音1
#define N_2_H  NOTE_A6   // 高音2
#define N_3_H  NOTE_B6   // 高音3
#define N_5_H  NOTE_D7   // 高音5

#define N_4S_L NOTE_CS5  // 低音#4
#define N_5N_L NOTE_D5   // 低音还原5
#define N_5S   NOTE_DS6  // 中音#5

#define REST   NOTE_SILENT

const music_note_t music_heroism[] = {
    // 第1小节: 3 - 5 -
    {N_3, BEAT_2}, {N_5, BEAT_2},

    // 第2小节: 6 <7 1'> 7 <0 5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_1_H, BEAT_1_2},
    {N_7, BEAT_1}, {REST, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第3小节: 6 3' 3' -
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_3_H, BEAT_2},

    // 第4小节: 6 <7 1'> 7 <0 5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_1_H, BEAT_1_2},
    {N_7, BEAT_1}, {REST, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第5小节: 6 5' 3' -
    {N_6, BEAT_1}, {N_5_H, BEAT_1}, {N_3_H, BEAT_2},

    // 第6小节: 6 <7 1'> 7 <0 5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_1_H, BEAT_1_2},
    {N_7, BEAT_1}, {REST, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第7小节: 6 3' 3' <2' 1'>
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_3_H, BEAT_1},
    {N_2_H, BEAT_1_2}, {N_1_H, BEAT_1_2},

    // 第8小节: 2' 3' 7 5
    {N_2_H, BEAT_1}, {N_3_H, BEAT_1}, {N_7, BEAT_1}, {N_5, BEAT_1},

    // 第9小节: 6 <7 6> #5 <3 还原5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_6, BEAT_1_2},
    {N_5S, BEAT_1}, {N_3, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第10小节: 6 <7 1'> 7 <0 5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_1_H, BEAT_1_2},
    {N_7, BEAT_1}, {REST, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第11小节: 6 3' 3' -
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_3_H, BEAT_2},

    // 第12小节: 6 <7 1'> 7 <0 5>
    {N_6, BEAT_1}, {N_7, BEAT_1_2}, {N_1_H, BEAT_1_2},
    {N_7, BEAT_1}, {REST, BEAT_1_2}, {N_5, BEAT_1_2},

    // 第13小节: 6 5' 3' -
    {N_6, BEAT_1}, {N_5_H, BEAT_1}, {N_3_H, BEAT_2},

    // 第14小节: 6 <5' 3'> 2' <1' 7>
    {N_6, BEAT_1}, {N_5_H, BEAT_1_2}, {N_3_H, BEAT_1_2},
    {N_2_H, BEAT_1}, {N_1_H, BEAT_1_2}, {N_7, BEAT_1_2},

    // 第15小节: 6 3' 2' <1' 7>
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_2_H, BEAT_1},
    {N_1_H, BEAT_1_2}, {N_7, BEAT_1_2},

    // 第16小节: 6 3' 7 5
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_7, BEAT_1}, {N_5, BEAT_1},

    // 第17小节: 6 3' 5 7
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_5, BEAT_1}, {N_7, BEAT_1},

    // 第18小节: 6 3' 7 5
    {N_6, BEAT_1}, {N_3_H, BEAT_1}, {N_7, BEAT_1}, {N_5, BEAT_1},

    // 第19-20小节: 6 5 6 - | 0 0 0 0
    {N_6, BEAT_1}, {N_5, BEAT_1}, {N_6, BEAT_2},
    {REST, BEAT_4},
};

const uint16_t music_heroism_length = sizeof(music_heroism) / sizeof(music_note_t);
