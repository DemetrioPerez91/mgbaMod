#include "logToFile.h"

void log_to_file(const char *filename, const char *message) {
    if (!LOGS_ENABLED) { 
        return; 
    }
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
