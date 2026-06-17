idf_component_register(
        SRCS
        "app_main.cc"
        "frame/builder.cc"
        "frame/parser.cc"
        "shared_memory/shared_memory.cc"
        "tasks/task_failsafe.cc"
        "tasks/task_pid.cc"
        "tasks/task_rx.cc"
        "tasks/task_telemetry.cc"
        "tasks/task_uart_emitter.cc"

        INCLUDE_DIRS
        "."
)

target_compile_options(${COMPONENT_LIB} PRIVATE
        -Wall
        -Wextra
        -Werror
)