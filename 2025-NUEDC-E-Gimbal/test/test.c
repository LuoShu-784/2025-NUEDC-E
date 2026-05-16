#include "main.h"
#include "cmsis_os.h"
#include "test_softuart.h"

void test_task(void *argument)
{
    osDelay(100);
    UNUSED(argument);

    test_softuart_init();

    while (1)
    {
        test_softuart_process();
        osDelay(10);
    }
}