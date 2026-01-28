#ifndef CONFIG
#define CONFIG

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

extern uint16_t port;
extern uint16_t backlog;

bool init_config(void);

#endif