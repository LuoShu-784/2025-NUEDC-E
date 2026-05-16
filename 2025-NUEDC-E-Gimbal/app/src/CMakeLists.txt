target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # 如果有其他 .c 文件也直接写名字
    app_main.c
    app_led.c
    app_motor.c
    app_callback.c
)