/**
 **   File: log.cpp
 **   Description: Logging functionality, always flushes.
 **/
#include <stdio.h>
#include "params.h"
#include "log.h"

static int openFile = 0;
FILE *fileDescriptor = NULL;
const char *logLocation = "./logFile";

int log_open() {
    fileDescriptor = fopen(logLocation, "a+");
    
    if(fileDescriptor == NULL) {
        printf("Cannot open log file.\n");
        return -1;
    } else {
        openFile = 1;
        return 0;
    }
}

void log_msg(const char *msg) {
    fprintf(fileDescriptor, "%s\n",msg);
    fflush(fileDescriptor);
}
