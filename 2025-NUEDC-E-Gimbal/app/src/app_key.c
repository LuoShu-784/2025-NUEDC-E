#include "app_key.h"
#include "key.h"

void key_task(void *argument)
{
    osDelay(100);
    UNUSED(argument);

    while (1)
    {
        key_process();
        osDelay(1000);
    }
}