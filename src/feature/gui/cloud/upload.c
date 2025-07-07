// network.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "upload.h"

typedef struct {
    char* data;     // file contents
    u32 size;       // number of bytes
    char* filename; // just the filename, no path
} FileResult;

FileResult getFileData(const char* filepath) {
    log_to_file("sdmc:/roms/gba/myapp.log", "----- Building File Data -----");
    
    FileResult result = {0};

    // Extract filename from path
    const char* lastSlash = strrchr(filepath, '/');
    result.filename = lastSlash ? strdup(lastSlash + 1) : strdup(filepath);
    log_to_file("sdmc:/roms/gba/myapp.log", "----- FILE NAME -----");
    
    log_to_file("sdmc:/roms/gba/myapp.log", result.filename);


    FILE* file = fopen(filepath, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filepath);
        free(result.filename);
        result.filename = NULL;

        log_to_file("sdmc:/roms/gba/myapp.log", "----- FOPEN FAILURE -----");


        return result;

    }

    fseek(file, 0, SEEK_END);
    result.size = ftell(file);
    rewind(file);

    result.data = (char*)memalign(0x1000, result.size); // 4KB-aligned for HTTP
    if (!result.data) {

        printf("Failed to allocate memory\n");
        fclose(file);
        free(result.filename);
        result.filename = NULL;
        result.size = 0;
        log_to_file("sdmc:/roms/gba/myapp.log", "----- ALOCATION FAILURE -----");
        return result;
    }

    fread(result.data, 1, result.size, file);
    fclose(file);
    return result;
}

// Takes the data from a file and uplaods it using funcitons from 3DS.h
Result uploadFileToServer(const char *url, const char* filename, const char* fileData, u32 fileSize) {
    const char* boundary = "----3ds-multipart-boundary";
    char contentType[256];
    snprintf(contentType, sizeof(contentType), "multipart/form-data; boundary=%s", boundary);

    // Build multipart body
    const char* part1_fmt = "--%s\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n";
    const char* part2 = "\r\n--%s--\r\n";

    size_t part1_len = snprintf(NULL, 0, part1_fmt, boundary, filename);
    size_t part2_len = snprintf(NULL, 0, part2, boundary);
    
    size_t totalSize = part1_len + fileSize + part2_len;
    // char* body = (char*)malloc(totalSize);
    char* body = (char*)memalign(0x1000, totalSize); // 4KB aligned allocation
    if (!body) return -1;

    snprintf(body, part1_len + 1, part1_fmt, boundary, filename);
    memcpy(body + part1_len, fileData, fileSize);
    snprintf(body + part1_len + fileSize, part2_len + 1, part2, boundary);

    // HTTP POST
    httpcContext context;
    Result ret = httpcOpenContext(&context, HTTPC_METHOD_POST, url, 0);
    if (ret != 0) {
        free(body);
        return ret;
    }

    httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
    httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");
    httpcAddRequestHeaderField(&context, "Content-Type", contentType);
    httpcAddRequestHeaderField(&context, "User-Agent", "3DSUploader/1.0");
    httpcAddPostDataRaw(&context, (const u32*)body, totalSize);

    ret = httpcBeginRequest(&context);
    if (ret == 0) {
        u32 statusCode = 0;
        httpcGetResponseStatusCode(&context, &statusCode);
        printf("Status Code: %lu\n", statusCode);     
    }

    u32 statusCode = 0;
    httpcGetResponseStatusCode(&context, &statusCode);
    log_to_file("sdmc:/roms/gba/myapp.log", "----- Upload Status Code-----");
    char label[255];
    char buffer[64]; // plenty for a u32
    sprintf(buffer, "%s: %lu", label, (unsigned long)statusCode);
    log_to_file("sdmc:/roms/gba/myapp.log", buffer);

    httpcCloseContext(&context);
    free(body);
    return ret;
}


// Convenience wrapper for loading + uploading
Result httpUploadFile(const char *url, const char *filepath) {
    FileResult file = getFileData(filepath);
    return uploadFileToServer(url, file.filename, file.data, file.size);
}

void uploadFileByName(const char *saveFilename) 
{
    // Initialize HTTP
    httpcInit(4 * 1024 * 1024);
    log_to_file("sdmc:/roms/gba/myapp.log", "----- Uploading File -----");
    log_to_file("sdmc:/roms/gba/myapp.log", saveFilename);

    char cleanName[512];
    cleanupFileName(saveFilename, cleanName);
    log_to_file("sdmc:/roms/gba/myapp.log", "----- CLEAN NAME -----");
    log_to_file("sdmc:/roms/gba/myapp.log", cleanName);

    char filePath[512];
    snprintf(filePath, sizeof(filePath), "%s/%s", SAVE_FOLDER, cleanName);

    log_to_file("sdmc:/roms/gba/myapp.log", "----- FILE PATH -----");
    log_to_file("sdmc:/roms/gba/myapp.log", filePath);

    char saveFilesEndPoint[512];
    snprintf(saveFilesEndPoint, sizeof(saveFilesEndPoint), "%s%s", SERVER_PROD_URL, SAVEFILES_ENDPOINT);
    
    log_to_file("sdmc:/roms/gba/myapp.log", "----- FILE PATH -----");
    log_to_file("sdmc:/roms/gba/myapp.log", saveFilesEndPoint);

    Result ret = httpUploadFile(saveFilesEndPoint, filePath);
    if (ret != 0 ) {
        printf("FILE LOCATION: %s\n", filePath);
        printf("File upload failed\n");
        log_to_file("sdmc:/roms/gba/myapp.log", "UPLAOD FAILED");
    } else {
        printf("File upload Complete\n");
        log_to_file("sdmc:/roms/gba/myapp.log", "UPLAOD SUCCEEDED");
    }
    
    httpcExit();
}