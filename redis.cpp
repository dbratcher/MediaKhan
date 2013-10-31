#include "redis.h"
#include "utils.h"

redisContext* c;
redisReply* reply;

bool redis_init() {
  struct timeval timeout = { 3600, 0};
  c=redisConnectWithTimeout((char*)"localhost",6379, timeout);
    
  if(c->err) {
    fprintf(stderr, "Connection error: %s\n", c->errstr);
    return 0;
  }
    
  return 1;
}

string de_dup(string val) {
  // cout << "dedup " << val << " into ";
  vector<string> vals = split(val, ":");
  vector<string> uniques;
  for(int i=0; i<vals.size(); i++) {
    // cout << "looking at " << vals[i] << endl;
    string ret = join(uniques,":");
    size_t found = ret.find(vals[i]);
    if(found==string::npos) {
      // cout << "unique" << endl;
      if(vals[i]!="") {
        uniques.push_back(vals[i]);
      }
    }
  } 
  string new_ret = join(uniques, ":");
  // cout << new_ret << endl;
  return new_ret;  
}

string cleanup_str(string str) {
  if(str.at(0)==':') {
    str = str.substr(1);
  }
  return str;
}

string redis_getval(string file_id, string col) {
  reply = (redisReply*)redisCommand(c,"hget %s %s",file_id.c_str(),col.c_str());

  string output = "null";
    
  if(reply->len!=0) {
    output = reply->str;
  }

  //freeReplyObject(reply);
  return cleanup_str(output);
}

string redis_getkey_cols(string col) {
  //cout << "fetching col " << col << endl;
  reply = (redisReply*)redisCommand(c,"hkeys %s",col.c_str());
  string output = "null";

  if(reply->elements!=0) {
    //cout << "nonzero" << endl << flush;
    output = "";
    for(int i=0; i<reply->elements; i++) {
      if(reply->element[i]) {
        //cout << i << endl << flush;
        output = output + reply->element[i]->str + ":";
      }
    }
  }
  //freeReplyObject(reply);
  //cout << "returned " << output << endl;
  return cleanup_str(output);
}


string redis_setval(string file_id, string col, string val) {  
  // generate file_id if needed
  if(file_id.compare("null")==0) {
    string file_id=redis_getval("redis_last_id","val");
    //cout << "got file id " << file_id << endl;	
    if(file_id.compare("null")==0) {
      file_id="1";
    }

    int redis_last_id=0;
    redis_last_id=atoi(file_id.c_str());
    redis_last_id++;//find non-local solution (other table?)
    ostringstream result;
    result<<redis_last_id;
    //cout << "removing " << file_id << endl;
    redis_remove_val("redis_last_id","val",file_id);
    reply = (redisReply*)redisCommand(c,"hget redis_last_id val");
    if(reply->len!=0) {
      string rep_str = reply->str;
      //cout << "did it take?" << rep_str << endl;
    }
    //freeReplyObject(reply);
    //cout << "setting " << result.str() << endl;
    redis_setval("redis_last_id","val",result.str());
    file_id = result.str();
    redis_setval(file_id,col,val);
    //cout << "returning " << file_id;
    return file_id;
  }

  // handle file_id key
  reply = (redisReply*)(redisReply*)redisCommand(c,"hget %s %s",file_id.c_str(),col.c_str());
  string output = val;
    
  if(reply->len != 0) {
    string rep_str = reply->str;
    output = rep_str + ":" + output;
  }
  //freeReplyObject(reply);
  //output = de_dup(output); 
  reply = (redisReply*)redisCommand(c,"hset %s %s %s",file_id.c_str(),col.c_str(),output.c_str());
  //freeReplyObject(reply);

  //handle col key
  reply = (redisReply*)redisCommand(c,"hget %s %s",col.c_str(),val.c_str());
  output = file_id;
    
  if(reply->len != 0) {
    string rep_str = reply->str;
    output = rep_str + ":" + output;
  }
  //freeReplyObject(reply);
    
  //output = de_dup(output); 
  reply = (redisReply*)redisCommand(c,"hset %s %s %s",col.c_str(),val.c_str(),output.c_str());
  //freeReplyObject(reply);
  return file_id;
}



void redis_remove_val(string fileid, string col, string val){
  //cout << "in remove val" << endl;
  //cout << "updating fileid" << endl;
  reply = (redisReply*)redisCommand(c,"hget %s %s",fileid.c_str(),col.c_str());
  if(reply->len != 0 ) {
    string source = reply->str;
    //freeReplyObject(reply);

    //cout << "got " << source << endl;
    size_t found = source.find(val);
    if(found != string::npos) {
      source.erase(found, val.length());
      //cout << "after erase " << source << endl;
    }
    if(source.length()>0) {
      reply = (redisReply*)redisCommand(c,"hset %s %s %s",fileid.c_str(),col.c_str(),source.c_str());
      //freeReplyObject(reply);
    } else { 
      reply = (redisReply*)redisCommand(c,"hdel %s %s",fileid.c_str(),col.c_str());
      //freeReplyObject(reply);
    }
  } else {
    //freeReplyObject(reply);
  }
  
  //remove from col entry
  //cout << "updating col entry" << endl;
  string col_entry = redis_getval(col, val);
  //cout << "got " << col_entry << endl;
  size_t found = col_entry.find(fileid);
  if(found != string::npos) {
    col_entry.erase(found, fileid.length());
    //cout << "after erase " << col_entry << endl;
  }
  reply = (redisReply*)redisCommand(c,"hset %s %s %s",col.c_str(),val.c_str(), col_entry.c_str());
  //freeReplyObject(reply);
}


