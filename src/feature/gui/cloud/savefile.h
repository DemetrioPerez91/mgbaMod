// ============================
// savefile.h
// ============================
#ifndef SAVEFILE_H
#define SAVEFILE_H

#include <stdio.h>
#include "config.h"

typedef struct {
    char filename[512];
    double timestamp;
    char lastModified[64];
    int size;
} SaveFile;

extern SaveFile saveFiles[MAX_SAVES];
extern int saveFileCount;
extern char textBuffer[MAX_LINES][LINE_LENGTH];
extern int totalLines;
extern int scrollOffset;

#endif
