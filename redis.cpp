#include "redis.h"

/*
utils may be handy one day

vector<string> split(string str, string delim) { 
  unsigned start = 0;
  unsigned end; 
  vector<string> v; 

  while( (end = str.find(delim, start)) != string::npos ) { 
    v.push_back(str.substr(start, end-start)); 
    start = end + delim.length(); 
  } 
  v.push_back(str.substr(start)); 
  return v; 
}

string join(vector<string> vals, string delim) {
  string ret = "";
  for(int i=0; i<vals.size(); i++) {
    ret = ret + vals[i];
  }
  return ret;
}
*/


void Redis::init() {
  struct timeval timeout = {3600, 0};
  context = redisConnectWithTimeout((char*)"127.0.0.1",6379, timeout);
  if(context->err) {
    cout << "Could not setup redis connection: " << context->errstr << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }
}

string Redis::get(string key) {
  string ret = "";
  reply = (redisReply*) redisCommand(context, ("get "+key).c_str());
  if(reply->str) {
    ret = reply->str;
  }
  return ret;
}

void Redis::set(string key, string value) {
  redisCommand(context, ("set " + key + " " + value).c_str());
}

int Redis::get_id() {
  last_id+=1;
  return last_id;
}

void Redis::remove(string key, string value) {
  string vals = Redis::get(key);
  size_t it = vals.find(value);
  if (it != string::npos) {
    vals.erase(it, value.length()); 
    cout << "new string " << vals << endl;
    redisCommand(context, ("set " + key + " " + vals).c_str()); 
  }
}

string Redis::hget(string hash, string key) {
  string ret = "";
  reply = (redisReply*) redisCommand(context, ("hget "+hash+" "+key).c_str());
  if(reply->len > 0) {
    ret = reply->str;
  }
  return ret;
}

void Redis::hset(string hash, string key, string value) {
  redisCommand(context, ("hset " + hash + " " + key + " " + value).c_str());
}

string Redis::type(string key) {
  reply = (redisReply*) redisCommand(context, ("type " + key).c_str());
  if(reply->len == 0) {
    return "null";
  }
  return reply->str;
}
