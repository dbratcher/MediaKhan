#include "redis.h"

redisContext *c;
redisReply *reply=NULL;


bool redis_init() {
  struct timeval timeout = { 3600, 0};
  c=redisConnectWithTimeout((char*)"127.0.0.1",6379, timeout);
    
  if(c->err) {
    fprintf(stderr, "Connection error: %s\n", c->errstr);
    return 0;
  }
    
  return 1;
}


string redis_getval(string file_id, string col) {
  reply = (redisReply *)redisCommand(c,"hget %s %s",file_id.c_str(),col.c_str());

  string output = "null";
    
  if(reply->len!=0) {
    output = reply->str;
  }

  return output;
}

string redis_getkey_cols(string col) {
  reply = (redisReply *)redisCommand(c,"hkeys %s",col.c_str());
  cout << "fetching col " << col << endl;
  string output = "null";

  if(reply->elements!=0) {
    output = "";
    for(int i=0; i<reply->elements; i++) {
      output = output + reply->element[i]->str + ":";
    }
  } else if(reply->len!=0) {
    output = reply->str;
  }
  cout << "returned " << output << endl;
  return output;

}


string redis_setval(string file_id, string col, string val) {  
  // generate file_id if needed
  if(file_id.compare("null")==0) {
    string file_id=redis_getval("redis_last_id","val");
    cout << "got file id " << file_id << endl;	
    if(file_id.compare("null")==0) {
      file_id="1";
    }

    int redis_last_id=0;
    redis_last_id=atoi(file_id.c_str());
    redis_last_id++;//find non-local solution (other table?)
    ostringstream result;
    result<<redis_last_id;
    cout << "removing " << file_id << endl;
    redis_remove_val("redis_last_id","val",file_id);
    reply = (redisReply*)redisCommand(c,"hget redis_last_id val");
    if(reply->len!=0) {
      string rep_str = reply->str;
      cout << "did it take?" << rep_str << endl;
    }
    cout << "setting " << result.str() << endl;
    redis_setval("redis_last_id","val",result.str());
    file_id = result.str();
    redis_setval(file_id,col,val);
    cout << "returning " << file_id;
    return file_id;
  }

  // handle file_id key
  reply = (redisReply*)redisCommand(c,"hget %s %s",file_id.c_str(),col.c_str());
  string output = val;
    
  if(reply->len != 0) {
    string rep_str = reply->str;
    output = rep_str + ":" + output;
  }
    
  reply = (redisReply*)redisCommand(c,"hset %s %s %s",file_id.c_str(),col.c_str(),output.c_str());

  //handle col key
  reply = (redisReply*)redisCommand(c,"hget %s %s",col.c_str(),val.c_str());
  output = file_id;
    
  if(reply->len != 0) {
    string rep_str = reply->str;
    output = rep_str + ":" + output;
  }
    
  reply = (redisReply *)redisCommand(c,"hset %s %s %s",col.c_str(),val.c_str(),output.c_str());
  return file_id;
}



void redis_remove_val(string fileid, string col, string val){
  cout << "in remove val" << endl;
  reply = (redisReply*)redisCommand(c,"hget %s %s",fileid.c_str(),col.c_str());
  if(reply->len != 0 ) {
    string source = reply->str;
    cout << "got " << source << endl;
    size_t found = source.find(val);
    if(found != string::npos) {
      source.erase(found, val.length());
      cout << "after erase " << source << endl;
    }
    if(source.length()>0) {
      redisCommand(c,"hset %s %s %s",fileid.c_str(),col.c_str(),source.c_str());
    } else { 
      redisCommand(c,"hdel %s %s",fileid.c_str(),col.c_str());
    }
  }
  
  //remove from col entry
  reply = (redisReply *)redisCommand(c,"hdel %s %s",col.c_str(),val.c_str());
}


