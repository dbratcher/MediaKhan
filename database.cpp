#include "utils.h"
#include "database.h"

struct timespec start, stop;
double time_spent;

int vold_calls=0;
int readdir_calls=0;
int access_calls=0;
int getattr_calls=0;
int read_calls=0;
int write_calls=0;
int create_calls=0;
int rename_calls=0;
double tot_time=0;
double vold_avg_time=0;
double readdir_avg_time=0;
double access_avg_time=0;
double getattr_avg_time=0;
double read_avg_time=0;
double write_avg_time=0;
double create_avg_time=0;
double rename_avg_time=0;
double localize_time=0;
int redis_calls=0;
double redis_avg_time=0;

bool init_database(){
  #ifdef VOLDEMORT_FOUND
  if(DATABASE==VOLDEMORT) {
    return voldemort_init();
  }
  #endif
  #ifdef REDIS_FOUND
  if(DATABASE==REDIS){
    return redis_init();
  }
  #endif
  #ifdef BDB_FOUND
    if(DATABASE==BDB) {
      return bdb_init();
    }
  #endif
}


string database_setval(string file_id, string col, string val){
  file_id=trim(file_id);
  col=trim(col);
  val=trim(val);
  #ifdef VOLDEMORT_FOUND
  if(DATABASE==VOLDEMORT){
    vold_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstring="fail";
    retstring=voldemort_setval(file_id,col,val);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    vold_avg_time=(vold_avg_time*(vold_calls-1)+time_spent)/vold_calls;
    return retstring;
  }
  #endif
  #ifdef REDIS_FOUND
  if(DATABASE==REDIS){
    redis_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstring="fail";
    retstring=redis_setval(file_id,col,val);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    redis_avg_time=(redis_avg_time*(redis_calls-1)+time_spent)/redis_calls;
    return retstring;
  }
  #endif
  #ifdef BDB_FOUND
  if(DATABASE==BDB) {
    string retstring = bdb_setval(file_id, col, val);
    return retstring;
  }
  #endif  
}


string database_getval(string col, string val){
  //fprintf(stderr, "in getval with %s %s", col.c_str(), val.c_str());
  col=trim(col);
  val=trim(val);
  #ifdef VOLDEMORT_FOUND
  if(DATABASE==VOLDEMORT){
    //log_msg("using vold");
    vold_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstring=voldemort_getval(col,val);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
    vold_avg_time=(vold_avg_time*(vold_calls-1)+time_spent)/vold_calls;
    return retstring;
  }
  #endif
  #ifdef REDIS_FOUND
  if(DATABASE==REDIS){
    redis_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstring=redis_getval(col,val);
    //cout<<"just got a "<<retstring<<endl;
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
    redis_avg_time=(redis_avg_time*(redis_calls-1)+time_spent)/redis_calls;
    return retstring;
  }
  #endif
  #ifdef BDB_FOUND
  if(DATABASE==BDB) {
    string retstring = bdb_getval(col, val);
    return retstring;
  }
  #endif  
}

string database_getvals(string col){
  col=trim(col);
  #ifdef VOLDEMORT_FOUND
  if(DATABASE==VOLDEMORT){
    vold_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstr=voldemort_getkey_cols(col);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    vold_avg_time=(vold_avg_time*(vold_calls-1)+time_spent)/vold_calls;
    return retstr;
  }
  #endif
  #ifdef REDIS_FOUND
  if(DATABASE==REDIS){
    redis_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    string retstr=redis_getkey_cols(col);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    redis_avg_time=(redis_avg_time*(redis_calls-1)+time_spent)/redis_calls;
    return retstr;
  }
  #endif
  #ifdef BDB_FOUND
  if(DATABASE==BDB) {
    string retstring = bdb_getkey_cols(col);
    return retstring;
  }
  #endif  
}

void database_remove_val(string file, string col, string val){
  file=trim(file);
  col=trim(col);
  val=trim(val);
  #ifdef VOLDEMORT_FOUND
  if(DATABASE==VOLDEMORT){
    vold_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    voldemort_remove_val(file,col,val);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    vold_avg_time=(vold_avg_time*(vold_calls-1)+time_spent)/vold_calls;
    return;
  }
  #endif
  #ifdef REDIS_FOUND
  if(DATABASE==REDIS){
    redis_calls++;
    clock_gettime(CLOCK_REALTIME,&start);
    redis_remove_val(file,col,val);
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;
    redis_avg_time=(redis_avg_time*(redis_calls-1)+time_spent)/redis_calls;
    return;
  }
  return;
  #endif
  #ifdef BDB_FOUND
    if(DATABASE==BDB) {
      bdb_remove_val(file, col, val);
      return;
    }
  #endif  
}



