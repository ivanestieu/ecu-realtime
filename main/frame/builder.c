#include "builder.h"

#include "utils/safe_esp_log.h"

uint8_t *float_to_bytes(float f)
{
    void *f_ptr = &f;
    return f_ptr;
}

float bytes_to_float(const uint8_t bytes[sizeof(float)])
{
    const void *b_ptr = bytes;
    const float *f_ptr = b_ptr;
    return *f_ptr;
}

Frame output(const float command)
{
    Frame frame = init_frame(OUTPUT, sizeof(float));
    const uint8_t *bytes = float_to_bytes(command);
    for (size_t i = 0; i < sizeof(float); i++)
    {
        payload_append(&frame, bytes[i]);
    }
    compute_crc(&frame);
    ESP_LOGI(__FILE_NAME__, "output: frame built");
    return frame;
}

Frame stats(const uint32_t stats[STATS_SIZE])
{
    Frame frame = init_frame(STATS, STATS_SIZE);

    for (size_t i = 0; i < STATS_SIZE; i++)
    {
        const uint32_t val = stats[i];
        payload_append(&frame, (val >> 24) & 0xFF);
        payload_append(&frame, (val >> 16) & 0xFF);
        payload_append(&frame, (val >> 8) & 0xFF);
        payload_append(&frame, val & 0xFF);
    }
    compute_crc(&frame);
    ESP_LOGI(__FILE_NAME__, "stats: frame built");
    return frame;
}

static Frame message(const char *msg, const enum MsgType type)
{
    unsigned int expected_size = sizeof(char) * strlen(msg);
    if (expected_size >= FRAME_MAX_SIZE)
    {
        ESP_LOGI(__FILE_NAME__,
                 "message: trimming payload size to stay "
                 "underneath FRAME_MAX_SIZE (%d)",
                 FRAME_MAX_SIZE);
        expected_size = FRAME_MAX_SIZE;
    }
    Frame frame = init_frame(type, expected_size);

    for (size_t i = 0; i < expected_size; i++)
    {
        payload_append(&frame, msg[i]);
    }
    compute_crc(&frame);
    ESP_LOGI(__FILE_NAME__, "builder::message: frame built");
    return frame;
}

Frame alarm(const char *msg)
{
    return message(msg, ALARM);
}
Frame debug(const char *msg)
{
    return message(msg, DEBUG);
}