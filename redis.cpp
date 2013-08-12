#include "redis.h"

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

void Redis::init() {
  struct timeval timeout = {3600, 0};
  context = redisConnectWithTimeout((char*)"127.0.0.1",6379, timeout);
  if(context->err) {
    cout << "Could not setup redis connection: " << context->errstr << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }
}

vector<string> Redis::get(string key) {
  cout << "in get" << endl << flush;
  vector<string> ret;
  cout << "in get2" << endl << flush;
  reply = (redisReply*) redisCommand(context, ("get "+key).c_str());
  cout << "in get3" << endl << flush;
  if(reply->len > 0) {
    ret = split(reply->str, ",");
  }
  cout << "in get4" << endl << flush;
  return ret;
}

void Redis::set(string key, string value) {
  cout << "in set" << endl << flush;
  vector<string> vals = Redis::get(key);
  vals.push_back(value);
  string val = join(vals, ",");
  redisCommand(context, ("set " + key + " " + val).c_str());
}

int Redis::get_id() {
  last_id+=1;
  return last_id;
}

void Redis::remove(string key, string value) {
  vector<string> vals = Redis::get(key);
  vector<string>::iterator it = find(vals.begin(), vals.end(), value);
  if (it != vals.end()) {
    vals.erase(it); 
    string val = join(vals, ",");
    redisCommand(context, ("set " + key + " " + val).c_str()); 
  }
}

vector<string> Redis::hget(string hash, string key) {
  vector<string> ret;
  reply = (redisReply*) redisCommand(context, ("hget "+hash+" "+key).c_str());
  if(reply->len > 0) {
    ret = split(reply->str, ",");
  }
  return ret;
}

void Redis::hset(string hash, string key, string value) {
  vector<string> vals = Redis::hget(hash, key);
  vals.push_back(value);
  string val = join(vals, ",");
  redisCommand(context, ("hset " + hash + " " + key + " " + val).c_str());
}

string Redis::type(string key) {
  reply = (redisReply*) redisCommand(context, ("type " + key).c_str());
  if(reply->len == 0) {
    return "null";
  }
  return reply->str;
}
