#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>

#include "config.h"
#include "log.h"

#ifdef REDIS_FOUND
#include "redis.h"
#endif

#ifdef VOLDEMORT_FOUND
#include "voldemort.h"
#endif

#ifdef BDB_FOUND
#include "bdb.h"
#endif

using namespace std;

#define VOLDEMORT 1
#define REDIS 2
#define BDB 3
#define DATABASE REDIS
#define BILLION 1E9

#ifdef APPLE
    #include <sys/statvfs.h>
    #include <sys/dir.h>
    #define CLOCK_REALTIME 1
#endif
extern struct timespec start, stop;
extern double time_spent;

extern int vold_calls;
extern int readdir_calls;
extern int access_calls;
extern int getattr_calls;
extern int read_calls;
extern int write_calls;
extern int create_calls;
extern int rename_calls;
extern double tot_time;
extern double vold_avg_time;
extern double readdir_avg_time;
extern double access_avg_time;
extern double getattr_avg_time;
extern double read_avg_time;
extern double write_avg_time;
extern double create_avg_time;
extern double rename_avg_time;
extern double localize_time;
extern int redis_calls;
extern double redis_avg_time;
extern int bdb_calls;
extern double bdb_avg_time;

bool init_database();
string database_setval(string file_id, string col, string val);
string database_getval(string col, string val);
string database_getvals(string col);
void database_remove_val(string file, string col, string val);
