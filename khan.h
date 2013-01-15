#include "params.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fuse.h>
#include <ulockmgr.h> /**/
#include <string.h>
#include <vector>
#include <functional>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h> 
#include <string>
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
#include <voldemort/voldemort.h>
#include <hiredis/hiredis.h>
#include <string>
#include <list>
#include <memory>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdlib.h>
#include <libgen.h>
#include "log.h"
#include "math.h"
#include <glob.h>

using namespace std;
using namespace Voldemort;

#define CREATION 0
#define SEARCH 1
#define UPDATE 2
#define MAX 50
#define MAX_LEN 4096 

#define FUSE_USE_VERSION 26
#define MAX_PATH_LENGTH 2048
#define NO_RECORDS 1024
#define DATE 100 
#define MAX_NODES 10
static char command[MAX_PATH_LENGTH];
static struct khan_state *khan_data=NULL;
string khan_root="/home/drew/projects/mediakhan/khan_root";

     time_t time_now;
       static char * fpath=NULL;
mode_t khan_mode=S_ISUID | S_ISGID | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
char * temp=NULL;
char * temp2=NULL;
char *args=NULL;
static int fetch_for_node=-1;
char msg[4096];
int timestamp;

#define add_path "storage"

#define PACKAGE_VERSION 2.6


