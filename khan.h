#include "config.h"
#include "params.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fuse.h>
#include <vector>
#include <functional>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h> 
#include <fuse_opt.h>
#include <fuse_lowlevel.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <list>
#include <memory>
#include <fstream>
#include <map>
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdlib.h>
#include <libgen.h>
#include <glob.h>
#include <string.h>

#include "log.h"

#ifdef REDIS_FOUND
  #include "redis.h"
#endif

#ifdef VOLDEMORT_FOUND
  #include "voldemort.h"
#endif

#ifdef APPLE
    #include <sys/statvfs.h>
    #include <sys/dir.h>
    #define CLOCK_REALTIME 1
#endif

using namespace std;

#define MAX_LEN 4096 

#define FUSE_USE_VERSION 26
#define MAX_PATH_LENGTH 2048

static char command[MAX_PATH_LENGTH];
static struct khan_state *khan_data=NULL;

     time_t time_now;
       static char * fpath=NULL;
mode_t khan_mode=S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
char * temp=NULL;
char * temp2=NULL;
char *args=NULL;
char msg[4096];
int timestamp;

#define PACKAGE_VERSION 2.6


