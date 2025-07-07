#include "cleanupFileName.h"

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