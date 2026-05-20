#ifndef __APP_BUZZER_H_
#define __APP_BUZZER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "cmsis_os2.h"

#define APP_BUZZER_INTERVAL_MS      10U
#define APP_BUZZER_RATED_FREQ_HZ    4000U

#define BUZZER_BEEP_MS              150U
#define BUZZER_REPEAT_ON_MS         150U
#define BUZZER_REPEAT_OFF_MS        150U

#define BUZZER_MOD_OFF              (1U << 0)
#define BUZZER_MOD_BEEP             (1U << 1)
#define BUZZER_MOD_LONG             (1U << 2)
#define BUZZER_MOD_REPEAT           (1U << 3)
#define BUZZER_MOD_MUSIC            (1U << 4)
#define BUZZER_MOD_MUSIC_HEROISM    BUZZER_MOD_MUSIC
#define BUZZER_MOD_MUSIC_THE_KING   (1U << 5)
#define BUZZER_EVT_ALL              (0x3FU)

void buzzer_ctrl(uint32_t mode);
void buzzer_task(void *argument);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __APP_BUZZER_H_ */
