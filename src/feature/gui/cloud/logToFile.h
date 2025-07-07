#ifndef LOG_TO_FILE_H
#define LOG_TO_FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include "config.h"

void log_to_file(const char *filename, const char *message);

#endif