#include "config.h"
#include "params.h"
#include <algorithm>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <fuse.h>
#include <fuse_lowlevel.h>
#include <fuse_opt.h>
#include <glob.h>
#include <iterator>
#include <libgen.h>
#include <list>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <map>
#include <memory>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#ifdef APPLE
  #include "Python.h"
#else
  #include "Python.h"
#endif

#include "log.h"
#include "utils.h"
#include "localizations.h"
#include "database.h"


using namespace std;

#define MAX_LEN 4096 

#define FUSE_USE_VERSION 26
#define MAX_PATH_LENGTH 2048

static char command[MAX_PATH_LENGTH];
static struct khan_state *khan_data=NULL;

time_t time_now;
char * fpath=NULL;
mode_t khan_mode=S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
char * temp=NULL;
char * temp2=NULL;
char *args=NULL;
char msg[4096];
int timestamp;

#define PACKAGE_VERSION 2.6


