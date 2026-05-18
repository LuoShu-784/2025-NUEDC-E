#ifndef __JY901S_H_
#define __JY901S_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define JY901S_FRAME_HEADER      0x55U
#define JY901S_FRAME_TYPE_ANGLE  0x53U
#define JY901S_FRAME_LENGTH      11U
#define JY901S_ANGLE_SCALE       (180.0f / 32768.0f)

#ifndef JY901S_RX_BUF_SIZE
#define JY901S_RX_BUF_SIZE       64U
#endif

typedef struct {
    float roll;
    float pitch;
    float yaw;
    uint16_t version;
    bool valid;
} jy901s_data_t;

typedef void (*jy901s_callback_t)(const jy901s_data_t *attitude);

bool jy901s_init(void);
void jy901s_set_callback(jy901s_callback_t callback);
void jy901s_irq_handler(uint16_t size);
bool jy901s_get_attitude(jy901s_data_t *attitude);
void jy901s_get_statistics(uint32_t *frame_count, uint32_t *error_count);

#ifdef __cplusplus
}
#endif

#endif /* __JY901S_H_ */
