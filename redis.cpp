#include "redis.h"


int redis_last_id=1;
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
    reply = (redisReply *)redisCommand(c,("get "+file_id).c_str());
    
    if(reply->len==0) {
        return "null";
    }

    string output=reply->str;
    size_t exact=output.find("~"+col+":");
    string another="null";

    if(exact!=string::npos) {
        another=output.substr(exact);
        another=another.substr(2+col.length());
        size_t exact2=another.find("~");
        
        if(exact2==string::npos) {
            exact2=another.find("}");
        }

        another=another.substr(0,exact2);
    }

    // hget file_id, col

    return another;
}

string redis_getkey_cols(string col) {
    reply = (redisReply *)redisCommand(c,("get "+col).c_str());
    string output="";

    if(reply->len!=0) {
    	output=reply->str;
    }

    string ret_val="";
    stringstream ss(output);
    string val;

    while(getline(ss,val,'~')) {
    	stringstream ss2(val);
    	getline(ss2, val, ':');
    	ret_val+=val+":";
    }

    // hkeys col

    return ret_val;
}


string redis_setval(string file_id, string col, string val) {
    
    if(file_id.compare("null")==0) {
	string out=redis_getval("redis_last_id","val");
	
        if(out.compare("null")==0) {
	    out="1";
	}

        string file_id=out;
	redis_last_id=0;
	redis_last_id=atoi(out.c_str());
	redis_last_id++;//find non-local solution (other table?)
	ostringstream result;
	result<<redis_last_id;
	redis_remove_val("redis_last_id","val",out);
	redis_setval("redis_last_id","val",result.str());
	redis_setval(file_id,col,val);
	return file_id;
    }


    //handle file_id key
    reply = (redisReply *)redisCommand(c,("get "+file_id).c_str());
    string output;
    
    if(reply->len!=0) {
        output=reply->str;
    } else {
	//create this specific file id
	output="";
    }
    
    string store=output;
    string rest;
    
    if(store.find("~"+col+":")!=string::npos) {//col already set
        string setval=redis_getval(file_id,col);
	int len=setval.length();
	
        if(setval.find(val)==string::npos) {
	    setval+=":"+val;
	}
	
        store.replace(store.find("~"+col+":")+2+col.length(),len,setval);
    } else {
	store+="~"+col+":"+val;
    }
    
    reply = (redisReply *)redisCommand(c,"set %s %s",file_id.c_str(),store.c_str());

    //handle col key
    reply = (redisReply *)redisCommand(c,("get "+col).c_str());
    output="";
    
    if(reply->len!=0) {
	output=reply->str;
    }
    
    store=output;
    rest="";
    
    if(store.find("~"+val+":")!=string::npos) {//col already set
        rest=store.substr(store.find("~"+val+":"));
	rest=rest.substr(val.length()+2);
	size_t exact2=rest.find("~");
	
        if(exact2!=string::npos) {
	    rest=rest.substr(0,exact2);
	}
	
        size_t orig_length=rest.length();
	
        if(rest.find(file_id)==string::npos) {
	    rest+=":"+file_id;
	}
	
        store=store.replace(store.find("~"+val+":")+2+val.length(),orig_length,rest);
    } else {
	store+="~"+val+":"+file_id;
    }
    
    reply = (redisReply *)redisCommand(c,"set %s %s",col.c_str(),store.c_str());
    return file_id;
}



void redis_remove_val(string fileid, string col, string val){
  string replaced=redis_getval(fileid,col);
    
  if(replaced.find(val)!=string::npos) {
      //remove from file entry
      reply = (redisReply *)redisCommand(c,("get "+fileid).c_str());
      string srep = reply->str;
      int len=srep.find("~"+col+":");
      int len2 = srep.find(val, len+1);
      srep.replace(len2, val.length()+1, "");
      int len1 = srep.find("~",len+1);
      len2 = srep.find(":",len+1);
      if(len2>len1) {
        srep.replace(len, len1-len, "");
      }
      reply = (redisReply *)redisCommand(c,"set %s %s", fileid.c_str(), srep.c_str());

      //remove from col entry
      reply = (redisReply *)redisCommand(c,("get "+col).c_str());
      string sout=reply->str;
      len=sout.find("~"+val+":");
      len2=sout.find("~",len+1);
        
      if(len2>0) {
          sout.replace(len,len2-len,"");
      } else if(len>0) {
          sout.replace(len,sout.length(),"");
      } else {
          sout="";
      }

      reply = (redisReply *)redisCommand(c,"set %s %s", col.c_str(),sout.c_str());
    }
}


