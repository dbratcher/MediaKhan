#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>

//#define FILENAME /net/hu21/tgupta9/hello/logFile

int log_open(void);
void log_fi (struct fuse_file_info *fi);
void log_stat(struct stat *si);
void log_statvfs(struct statvfs *sv);
void log_utime(struct utimbuf *buf);

void log_msg(const char *format);
#endif
