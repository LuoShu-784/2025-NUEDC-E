/**
 * @file app_music.h
 * @brief 蜂鸣器乐谱数据接口
 */

#ifndef __APP_MUSIC_H_
#define __APP_MUSIC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "buzzer.h"

extern const music_note_t music_heroism[];
extern const uint16_t music_heroism_length;

extern const music_note_t music_the_king[];
extern const uint16_t music_the_king_length;

#ifdef __cplusplus
}
#endif

#endif /* __APP_MUSIC_H_ */
