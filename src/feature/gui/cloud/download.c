#include "download.h"

void log_to_file(const char *filename, const char *message) {
    FILE *logFile = fopen(filename, "a"); // "a" appends to file or creates if doesn't exist
    if (!logFile) {
        printf("Failed to open log file: %s\n", filename);
        return;
    }

    // Get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // Format timestamp
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S]", t);

    // Write to file
    fprintf(logFile, "%s %s\n", timeStr, message);
    fclose(logFile);
}

void cleanupFileName(const char* filepath, char* outName) {
    // Get the base filename from path
    const char* filename = strrchr(filepath, '/');
    filename = filename ? filename + 1 : filepath;

    // Copy the full filename to outName
    size_t i = 0;
    while (filename[i] != '\0' && i < 255) {
        outName[i] = filename[i];
        ++i;
    }
    outName[i] = '\0';  // Null-terminate

    // Find the last '.' in outName
    char* dot = strrchr(outName, '.');
    if (dot) {
        // Truncate at the dot
        *dot = '\0';
    }

    // Append ".sav"
    strcat(outName, ".sav");
}


void downloadFileByName(const char *saveFilename) 
{
	log_to_file("sdmc:/roms/gba/myapp.log", saveFilename);
	// char* fileName;
	// cleanupFileName(saveFilename,fileName);
	char fileName[256]; // Allocate space
	cleanupFileName(saveFilename, fileName);
	log_to_file("sdmc:/roms/gba/myapp.log", "----- corrected name-----");
	log_to_file("sdmc:/roms/gba/myapp.log", fileName);
	// Initialize HTTP
    httpcInit(0);

	// Make file download endpoint
	char encoded[256];
	urlEncode(fileName, encoded, sizeof(encoded));

	char saveFilesEndPoint[512];
	snprintf(saveFilesEndPoint, sizeof(saveFilesEndPoint), "%s%s/%s", SERVER_PROD_URL, SAVEFILES_ENDPOINT, encoded);

	log_to_file("sdmc:/roms/gba/myapp.log", "----- ENCODED-----");
	log_to_file("sdmc:/roms/gba/myapp.log", encoded);
	Result ret = http_download(saveFilesEndPoint, fileName);
	if (ret != 0 ){
		printf(" Downloading: \n\%s\n", fileName);
		printf("Download File failed\n");
		printf("FILE URL: %s\n", saveFilesEndPoint);
		log_to_file("sdmc:/roms/gba/myapp.log", "Download failed");

	}

	log_to_file("sdmc:/roms/gba/myapp.log", fileName);
	httpcExit();
}


/// Shared HTTP download logic
static Result http_download_to_buffer(const char *url, u8 **outBuf, u32 *outSize) {
	Result ret = 0;
	httpcContext context;
	char *newurl = NULL;
	u32 statuscode = 0, contentsize = 0, readsize = 0, size = 0;
	u8 *buf = NULL, *lastbuf = NULL;

retry:
	ret = httpcOpenContext(&context, HTTPC_METHOD_GET, url, 1);
	if (ret) return ret;

	httpcSetSSLOpt(&context, SSLCOPT_DisableVerify);
	httpcSetKeepAlive(&context, HTTPC_KEEPALIVE_ENABLED);
	httpcAddRequestHeaderField(&context, "User-Agent", "3ds-client/1.0");
	httpcAddRequestHeaderField(&context, "Connection", "Keep-Alive");

	ret = httpcBeginRequest(&context);
	if (ret) goto cleanup;

	ret = httpcGetResponseStatusCode(&context, &statuscode);
	if (ret) goto cleanup;

	// Handle redirect
	if ((statuscode >= 301 && statuscode <= 303) || (statuscode >= 307 && statuscode <= 308)) {
		if (!newurl) newurl = (char*)malloc(0x1000);
		if (!newurl) { ret = -1; goto cleanup; }

		ret = httpcGetResponseHeader(&context, "Location", newurl, 0x1000);
		if (ret) goto cleanup;

		url = newurl;
		httpcCloseContext(&context);
		goto retry;
	}

	if (statuscode != 200) {
		printf("HTTP error: %lu\n", statuscode);
		ret = -2;
		goto cleanup;
	}

	ret = httpcGetDownloadSizeState(&context, NULL, &contentsize);
	if (ret) goto cleanup;

	buf = (u8*)malloc(0x1000);
	if (!buf) { ret = -1; goto cleanup; }

	do {
		ret = httpcDownloadData(&context, buf + size, 0x1000, &readsize);
		size += readsize;

		if (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING) {
			lastbuf = buf;
			buf = (u8*)realloc(buf, size + 0x1000);
			if (!buf) {
				free(lastbuf);
				ret = -1;
				goto cleanup;
			}
		}
	} while (ret == (s32)HTTPC_RESULTCODE_DOWNLOADPENDING);

	if (ret) goto cleanup;

	// Final resize + null-terminate
	lastbuf = buf;
	buf = (u8*)realloc(buf, size + 1);
	if (!buf) {
		free(lastbuf);
		ret = -1;
		goto cleanup;
	}
	buf[size] = '\0';

	*outBuf = buf;
	*outSize = size;

	httpcCloseContext(&context);
	if (newurl) free(newurl);
	return 0;

cleanup:
	httpcCloseContext(&context);
	if (buf) free(buf);
	if (newurl) free(newurl);
	return ret;
}




/// File download
Result http_download(const char *url, const char *saveFilename) {

	log_to_file("sdmc:/roms/gba/myapp.log", "----- DOWNLOADING -----");
	log_to_file("sdmc:/roms/gba/myapp.log",saveFilename );

	u8 *buf = NULL;
	u32 size = 0;
	Result ret = http_download_to_buffer(url, &buf, &size);
	if (ret != 0) return ret;


	log_to_file("sdmc:/roms/gba/myapp.log", "----- WRITING FILE -----");


	char path[256];
	snprintf(path, sizeof(path), "sdmc:/roms/gba/%s", saveFilename);
	log_to_file("sdmc:/roms/gba/myapp.log", path);
	FILE *f = fopen(path, "wb");
	if (!f) {
		printf("Failed to open %s for writing\n", path);
		log_to_file("sdmc:/roms/gba/myapp.log", "Write Failed. \n");
		free(buf);
		return -3;
	}
	fwrite(buf, 1, size, f);
	fclose(f);
	log_to_file("sdmc:/roms/gba/myapp.log", "FILE FINISHED WRITTING SUCCESSFULLY");
	
	// Uncomment to see where the file is being saved
	// printf("Saved to %s (%lu bytes)\n", path, size);
	
	free(buf);
	return 0;
}
