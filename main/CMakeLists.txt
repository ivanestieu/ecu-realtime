idf_component_register(
        SRCS
        "frame/builder.c"
        "frame/parser.c"
        "shared_memory/shared_memory.c"
        "tasks/task_failsafe.c"
        "tasks/task_pid.c"
        "tasks/task_uart_emitter.c"
        "tasks/task_rx.c"
        "tasks/task_telemetry.c"
        "app_main.c"

        INCLUDE_DIRS
        "."
        "./utils/"
)

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wvla
)