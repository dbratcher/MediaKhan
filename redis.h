#ifndef REDIS_H
#define REDIS_H

#include <string>
#include <vector>
#include <iostream>
#include <hiredis/hiredis.h>

#include "database.h"


class Redis: public Database {
  redisContext* context;
  redisReply* reply;
  int last_id;
  public:
    void init();
    string get(string key);
    void set(string key, string value);
    int get_id();
    void remove(string key, string value);
    void hset(string hash, string key, string value);
    string hget(string hash, string key);
    string type(string key);
};

#endif
