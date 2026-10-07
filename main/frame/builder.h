#ifndef BUILDER_H
#define BUILDER_H

#include "parser.h"

#define STATS_SIZE 4

Frame output(float command);
Frame stats(const uint32_t stats[STATS_SIZE]);
Frame alarm(const char *msg);
Frame debug(const char *msg);
const uint8_t *float_to_bytes(const float *f);
float bytes_to_float(const uint8_t bytes[sizeof(float)]);

#endif // BUILDER_H
