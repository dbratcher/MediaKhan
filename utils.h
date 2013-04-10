#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <libgen.h>
#include "config.h"
#define MAX_PATH_LENGTH 2048

using namespace std;

extern char* fpath;
extern string servers[5];

string trim(string source, string t = " \n");
char* append_path2(string newp);
char* append_path(const char * newp);
int count_string(string tobesplit);
string intersect(string files1, string files2);
int get_file_size(string file_name);

#ifdef APPLE
int clock_gettime(int i, struct timespec* b);
char* strdup(const char* str);
#endif
