#include "test_softuart.h"
#include "soft_uart.h"
#include "led.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define SOFTUART_TEST_TX_INTERVAL_MS        50U
#define SOFTUART_TEST_REPORT_INTERVAL_MS    1000U
#define SOFTUART_TEST_LED_INTERVAL_MS       500U
#define SOFTUART_TEST_RX_LINE_MAX           96U
#define SOFTUART_TEST_TX_LINE_MAX           160U
#define SOFTUART_TEST_ECHO_ENABLE           1U
#define SOFTUART_TEST_PAYLOAD               "0123456789ABCDEF"

static uint32_t s_tx_seq = 0;
static uint32_t s_tx_bytes = 0;
static uint32_t s_rx_bytes = 0;
static uint32_t s_rx_ok_frames = 0;
static uint32_t s_rx_bad_frames = 0;
static uint32_t s_rx_other_lines = 0;
static uint32_t s_echo_bytes = 0;
static uint32_t s_last_tx_bytes = 0;
static uint32_t s_last_rx_bytes = 0;
static uint32_t s_last_tx_tick = 0;
static uint32_t s_last_report_tick = 0;
static uint32_t s_last_led_tick = 0;
static char s_rx_line[SOFTUART_TEST_RX_LINE_MAX];
static char s_tx_line[SOFTUART_TEST_TX_LINE_MAX];
static uint16_t s_rx_line_len = 0;

static uint16_t test_str_len(const char *str)
{
    uint16_t len = 0;

    while (str[len] != '\0')
    {
        len++;
    }

    return len;
}

static void test_append_char(char **pos, char *end, char ch)
{
    if (*pos < end)
    {
        **pos = ch;
        (*pos)++;
    }
}

static void test_append_str(char **pos, char *end, const char *str)
{
    while (*str != '\0')
    {
        test_append_char(pos, end, *str++);
    }
}

static void test_append_u32(char **pos, char *end, uint32_t value)
{
    char tmp[10];
    uint8_t len = 0;

    if (value == 0U)
    {
        test_append_char(pos, end, '0');
        return;
    }

    while (value > 0U && len < sizeof(tmp))
    {
        tmp[len++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (len > 0U)
    {
        test_append_char(pos, end, tmp[--len]);
    }
}

static void test_append_hex_u8(char **pos, char *end, uint8_t value)
{
    static const char hex[] = "0123456789ABCDEF";

    test_append_char(pos, end, hex[(value >> 4) & 0x0F]);
    test_append_char(pos, end, hex[value & 0x0F]);
}

static uint8_t test_checksum_range(const char *start, const char *end)
{
    uint8_t checksum = 0;

    while (start < end)
    {
        checksum ^= (uint8_t)*start++;
    }

    return checksum;
}

static uint8_t test_checksum_text(const char *data)
{
    uint8_t checksum = 0;

    while (*data != '\0' && *data != '*')
    {
        checksum ^= (uint8_t)*data++;
    }

    return checksum;
}

static int test_hex_value(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }

    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }

    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }

    return -1;
}

static bool test_hex_to_u8(const char *str, uint8_t *value)
{
    if (str == NULL || value == NULL || str[0] == '\0' || str[1] == '\0')
    {
        return false;
    }

    int high = test_hex_value(str[0]);
    int low = test_hex_value(str[1]);

    if (high < 0 || low < 0)
    {
        return false;
    }

    *value = (uint8_t)((high << 4) | low);
    return true;
}

static void test_send_string(const char *str)
{
    HAL_SOFT_UART_Transmit(&hsuart, (uint8_t *)str, test_str_len(str), HAL_MAX_DELAY);
    s_tx_bytes += test_str_len(str);
}

static void test_send_frame(void)
{
    char *pos = s_tx_line;
    char *end = &s_tx_line[sizeof(s_tx_line) - 1U];
    char *body_start;
    uint8_t checksum;

    test_append_char(&pos, end, '$');
    body_start = pos;
    test_append_str(&pos, end, "SU,");
    test_append_u32(&pos, end, s_tx_seq);
    test_append_char(&pos, end, ',');
    test_append_str(&pos, end, SOFTUART_TEST_PAYLOAD);

    checksum = test_checksum_range(body_start, pos);
    test_append_char(&pos, end, '*');
    test_append_hex_u8(&pos, end, checksum);
    test_append_str(&pos, end, "\r\n");
    *pos = '\0';

    test_send_string(s_tx_line);
    s_tx_seq++;
}

static void test_parse_line(char *line)
{
    char *checksum_pos;
    uint8_t expected_checksum;
    uint8_t actual_checksum;

    if (line[0] != '$')
    {
        s_rx_other_lines++;
        return;
    }

    if (strncmp(&line[1], "SU,", 3) != 0)
    {
        s_rx_other_lines++;
        return;
    }

    checksum_pos = strchr(line, '*');
    if (checksum_pos == NULL || !test_hex_to_u8(checksum_pos + 1, &expected_checksum))
    {
        s_rx_bad_frames++;
        return;
    }

    *checksum_pos = '\0';
    actual_checksum = test_checksum_text(&line[1]);
    *checksum_pos = '*';

    if (actual_checksum == expected_checksum)
    {
        s_rx_ok_frames++;
    }
    else
    {
        s_rx_bad_frames++;
    }
}

static void test_process_rx(void)
{
    uint8_t data;

    while (HAL_SOFT_UART_Receive_IT(&hsuart, &data, 1) == HAL_OK)
    {
        s_rx_bytes++;

#if SOFTUART_TEST_ECHO_ENABLE
        HAL_SOFT_UART_Transmit_IT(&hsuart, &data, 1);
        s_tx_bytes++;
        s_echo_bytes++;
#endif

        if (data == '\r')
        {
            continue;
        }

        if (data == '\n')
        {
            if (s_rx_line_len > 0U)
            {
                s_rx_line[s_rx_line_len] = '\0';
                test_parse_line(s_rx_line);
                s_rx_line_len = 0;
            }
            continue;
        }

        if (s_rx_line_len < (SOFTUART_TEST_RX_LINE_MAX - 1U))
        {
            s_rx_line[s_rx_line_len++] = (char)data;
        }
        else
        {
            s_rx_line_len = 0;
            s_rx_bad_frames++;
        }
    }
}

static void test_report(uint32_t now)
{
    char *pos = s_tx_line;
    char *end = &s_tx_line[sizeof(s_tx_line) - 1U];
    uint32_t elapsed_ms = now - s_last_report_tick;
    uint32_t tx_delta = s_tx_bytes - s_last_tx_bytes;
    uint32_t rx_delta = s_rx_bytes - s_last_rx_bytes;

    if (elapsed_ms == 0U)
    {
        return;
    }

    test_append_str(&pos, end, "#STAT t=");
    test_append_u32(&pos, end, now);
    test_append_str(&pos, end, "ms tx=");
    test_append_u32(&pos, end, s_tx_bytes);
    test_append_char(&pos, end, 'B');
    test_append_char(&pos, end, ' ');
    test_append_u32(&pos, end, tx_delta * 1000U / elapsed_ms);
    test_append_str(&pos, end, "B/s rx=");
    test_append_u32(&pos, end, s_rx_bytes);
    test_append_char(&pos, end, 'B');
    test_append_char(&pos, end, ' ');
    test_append_u32(&pos, end, rx_delta * 1000U / elapsed_ms);
    test_append_str(&pos, end, "B/s ok=");
    test_append_u32(&pos, end, s_rx_ok_frames);
    test_append_str(&pos, end, " bad=");
    test_append_u32(&pos, end, s_rx_bad_frames);
    test_append_str(&pos, end, " other=");
    test_append_u32(&pos, end, s_rx_other_lines);
    test_append_str(&pos, end, " echo=");
    test_append_u32(&pos, end, s_echo_bytes);
    test_append_str(&pos, end, " seq=");
    test_append_u32(&pos, end, s_tx_seq);
    test_append_str(&pos, end, "\r\n");
    *pos = '\0';

    test_send_string(s_tx_line);
    s_last_tx_bytes = s_tx_bytes;
    s_last_rx_bytes = s_rx_bytes;
    s_last_report_tick = now;
}

void test_softuart_init(void)
{
    s_last_tx_tick = HAL_GetTick();
    s_last_report_tick = s_last_tx_tick;
    s_last_led_tick = s_last_tx_tick;

    test_send_string("\r\n# Soft UART simultaneous TX/RX test\r\n");
    test_send_string("# TX: PB14, RX: PB15, baud: 9600 8N1\r\n");
    test_send_string("# Board sends $SU frames every 50ms and counts received frames.\r\n");
    test_send_string("# RX echo is enabled. Do not use PB14-PB15 self-loop with echo enabled.\r\n");
}

void test_softuart_process(void)
{
    uint32_t now = HAL_GetTick();

    test_process_rx();

    if ((uint32_t)(now - s_last_tx_tick) >= SOFTUART_TEST_TX_INTERVAL_MS)
    {
        s_last_tx_tick += SOFTUART_TEST_TX_INTERVAL_MS;
        test_send_frame();
    }

    if ((uint32_t)(now - s_last_report_tick) >= SOFTUART_TEST_REPORT_INTERVAL_MS)
    {
        test_report(now);
    }

    if ((uint32_t)(now - s_last_led_tick) >= SOFTUART_TEST_LED_INTERVAL_MS)
    {
        s_last_led_tick += SOFTUART_TEST_LED_INTERVAL_MS;
        led_toggle(LED_R);
    }
}
