// ============================
// download.h
// ============================
#ifndef DOWNLOAD_H
#define DOWNLOAD_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include "config.h"
#include "savefile.h"
#include "encoding.h"


// Function wrapper that only requires the save file's name
void downloadFileByName(const char *saveFilename);
/// Shared HTTP download logic
static Result http_download_to_buffer(const char *url, u8 **outBuf, u32 *outSize);
// File download
Result http_download(const char *url, const char *saveFilename);

#endif