#include "key.h"

// 常量定义
#define FIRST_CLICK_COUNT       1U
#define CLICK_COUNT_RESET       0U
#define LONG_PRESS_NOT_REPORTED 0U
#define LONG_PRESS_REPORTED     1U

typedef struct
{
    GPIO_PinState stable_level;
    GPIO_PinState last_level;
    uint32_t debounce_tick;
    uint32_t press_tick;
    uint32_t click_tick;
    uint8_t click_count;
    bool is_long_press_reported;
    uint8_t event_flags;
#if KEY_WORK_MODE == KEY_MODE_INTERRUPT
    volatile bool is_irq_pending;
#endif
} key_context_t;

static gpio_t key_gpio[KEY_NUM] = {
    KEY_GPIO_CONFIG_TABLE
};

static key_context_t key_ctx[KEY_NUM];
static key_callback_t key_callback;
static bool is_initialized;

// 内联辅助函数
static inline GPIO_PinState key_read_pin(uint8_t id)
{
    return HAL_GPIO_ReadPin((GPIO_TypeDef *)key_gpio[id].port, key_gpio[id].pin);
}

static inline key_state_t key_pin_to_state(GPIO_PinState level)
{
    return (level == KEY_PRESS_STATE) ? KEY_PRESSED : KEY_RELEASED;
}

static inline bool key_is_pressed(key_context_t *ctx)
{
    return key_pin_to_state(ctx->stable_level) == KEY_PRESSED;
}

static inline bool key_is_released(key_context_t *ctx)
{
    return key_pin_to_state(ctx->stable_level) == KEY_RELEASED;
}

// 事件发射
static void key_emit(uint8_t id, key_event_t event)
{
    if (id >= KEY_NUM)
    {
        return;
    }

    key_ctx[id].event_flags |= (uint8_t)event;

    if (key_callback != NULL)
    {
        key_callback(id, event);
    }
}

// 双击超时检测
static void key_check_click_timeout(uint8_t id, uint32_t now_tick)
{
#if KEY_DOUBLE_CLICK_ENABLE
    key_context_t *ctx = &key_ctx[id];

    if ((ctx->click_count == FIRST_CLICK_COUNT) &&
        ((now_tick - ctx->click_tick) >= KEY_DOUBLE_CLICK_TIME_MS))
    {
#if KEY_CLICK_ENABLE
        key_emit(id, KEY_EVENT_CLICK);
#endif
        ctx->click_count = CLICK_COUNT_RESET;
    }
#else
    (void)id;
    (void)now_tick;
#endif
}

// 处理按键释放事件
static void key_handle_release(uint8_t id, uint32_t now_tick)
{
    key_context_t *ctx = &key_ctx[id];

    // 长按释放不触发点击事件
    if (ctx->is_long_press_reported)
    {
        ctx->is_long_press_reported = false;
        ctx->click_count = CLICK_COUNT_RESET;
        return;
    }

#if KEY_DOUBLE_CLICK_ENABLE
    // 检查双击超时（必须在双击检测前执行）
    if ((ctx->click_count == FIRST_CLICK_COUNT) &&
        ((now_tick - ctx->click_tick) >= KEY_DOUBLE_CLICK_TIME_MS))
    {
#if KEY_CLICK_ENABLE
        key_emit(id, KEY_EVENT_CLICK);
#endif
        ctx->click_count = CLICK_COUNT_RESET;
    }

    // 双击检测逻辑
    if (ctx->click_count == CLICK_COUNT_RESET)
    {
        // 第一次点击，记录时间戳
        ctx->click_count = FIRST_CLICK_COUNT;
        ctx->click_tick = now_tick;
    }
    else
    {
        // 第二次点击，触发双击事件
        key_emit(id, KEY_EVENT_DOUBLE_CLICK);
        ctx->click_count = CLICK_COUNT_RESET;
    }
#elif KEY_CLICK_ENABLE
    // 单击模式，直接触发点击事件
    key_emit(id, KEY_EVENT_CLICK);
#else
    (void)now_tick;
#endif
}

// 处理按键按下事件
static void key_handle_press(uint8_t id, uint32_t now_tick)
{
    key_context_t *ctx = &key_ctx[id];

    ctx->press_tick = now_tick;
    ctx->is_long_press_reported = false;
}

// 检测长按事件
static void key_check_long_press(uint8_t id, uint32_t now_tick)
{
#if KEY_LONG_PRESS_ENABLE
    key_context_t *ctx = &key_ctx[id];

    if (key_is_pressed(ctx) &&
        !ctx->is_long_press_reported &&
        ((now_tick - ctx->press_tick) >= KEY_LONG_PRESS_TIME_MS))
    {
        ctx->is_long_press_reported = true;
        ctx->click_count = CLICK_COUNT_RESET;
        key_emit(id, KEY_EVENT_LONG_PRESS);
    }
#else
    (void)id;
    (void)now_tick;
#endif
}

// 消抖和状态更新
static bool key_debounce_and_update(uint8_t id, uint32_t now_tick)
{
    key_context_t *ctx = &key_ctx[id];
    GPIO_PinState current_level = key_read_pin(id);

    // 电平变化，重置消抖计时器
    if (current_level != ctx->last_level)
    {
        ctx->last_level = current_level;
        ctx->debounce_tick = now_tick;
    }

    // 消抖时间未到或电平未变化
    if (((now_tick - ctx->debounce_tick) < KEY_DEBOUNCE_TIME_MS) ||
        (current_level == ctx->stable_level))
    {
        return false;
    }

    // 稳定电平更新
    ctx->stable_level = current_level;

    if (key_is_pressed(ctx))
    {
        key_handle_press(id, now_tick);
    }
    else
    {
        key_handle_release(id, now_tick);
    }

    return true;  // 返回 true 表示状态已更新
}

// 中断模式优化：判断是否需要处理按键
static bool key_needs_processing(uint8_t id)
{
#if KEY_WORK_MODE == KEY_MODE_INTERRUPT
    key_context_t *ctx = &key_ctx[id];

    // 有中断待处理
    if (ctx->is_irq_pending)
    {
        return true;
    }

    // 按键按下状态（需要检测长按和释放）
    if (key_is_pressed(ctx))
    {
        return true;
    }

    // 双击检测期间（需要检测超时）
    if (ctx->click_count != CLICK_COUNT_RESET)
    {
        return true;
    }

    return false;
#else
    (void)id;
    return true;  // 轮询模式始终处理
#endif
}

// 按键状态更新主逻辑
static void key_update(uint8_t id, uint32_t now_tick)
{
    if (id >= KEY_NUM)
    {
        return;
    }

    bool state_changed = key_debounce_and_update(id, now_tick);
    key_check_long_press(id, now_tick);

    // 仅在释放状态下检查双击超时（按下状态由 key_handle_release 处理）
    if (!state_changed && key_is_released(&key_ctx[id]))
    {
        key_check_click_timeout(id, now_tick);
    }

#if KEY_WORK_MODE == KEY_MODE_INTERRUPT
    // 只有在释放消抖完成后才清除中断标志（完成一个完整的按键周期）
    if (state_changed && key_is_released(&key_ctx[id]))
    {
        key_ctx[id].is_irq_pending = false;
    }
#endif
}

// 公共API实现
void key_init(void)
{
    uint32_t now_tick = HAL_GetTick();

    for (uint8_t i = 0U; i < KEY_NUM; ++i)
    {
        key_ctx[i].stable_level = key_read_pin(i);
        key_ctx[i].last_level = key_ctx[i].stable_level;
        key_ctx[i].debounce_tick = now_tick;
        key_ctx[i].press_tick = 0U;
        key_ctx[i].click_tick = 0U;
        key_ctx[i].click_count = CLICK_COUNT_RESET;
        key_ctx[i].is_long_press_reported = false;
        key_ctx[i].event_flags = (uint8_t)KEY_EVENT_NONE;
#if KEY_WORK_MODE == KEY_MODE_INTERRUPT
        key_ctx[i].is_irq_pending = false;
#endif
    }

    key_callback = NULL;
    is_initialized = true;
}

void key_process(void)
{
    if (!is_initialized)
    {
        key_init();
    }

    uint32_t now_tick = HAL_GetTick();

    for (uint8_t i = 0U; i < KEY_NUM; ++i)
    {
        if (!key_needs_processing(i))
        {
            continue;
        }

        key_update(i, now_tick);
    }
}

void key_irq_handler(uint16_t gpio_pin)
{
#if KEY_WORK_MODE == KEY_MODE_INTERRUPT
    if (!is_initialized)
    {
        key_init();
    }

    for (uint8_t i = 0U; i < KEY_NUM; ++i)
    {
        if (key_gpio[i].pin == gpio_pin)
        {
            key_ctx[i].is_irq_pending = true;
            break;
        }
    }
#else
    UNUSED(gpio_pin);
#endif
}

key_state_t key_get_state(uint8_t id)
{
    if ((id >= KEY_NUM) || !is_initialized)
    {
        return KEY_RELEASED;
    }

    return key_pin_to_state(key_ctx[id].stable_level);
}

uint8_t key_get_events(uint8_t id)
{
    if (id >= KEY_NUM)
    {
        return (uint8_t)KEY_EVENT_NONE;
    }

    uint8_t events = key_ctx[id].event_flags;
    key_ctx[id].event_flags = (uint8_t)KEY_EVENT_NONE;

    return events;
}

void key_clear_events(uint8_t id, uint8_t event_mask)
{
    if (id >= KEY_NUM)
    {
        return;
    }

    key_ctx[id].event_flags &= (uint8_t)(~event_mask);
}

void key_set_callback(key_callback_t callback)
{
    key_callback = callback;
}
