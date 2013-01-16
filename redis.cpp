#include "redis.h"

extern int last_id;
extern redisContext *c;
extern redisReply *reply;

bool redis_init(){
         struct timeval timeout = { 3600, 0};
         c=redisConnectWithTimeout((char*)"127.0.0.2",6379, timeout);
         if(c->err){
                 printf("Connection error: %s\n",c->errstr);
                 return 0;
         }
         reply = (redisReply *)redisCommand(c,"PING");
         printf("PING: %s\n", reply->str);
         freeReplyObject(reply);
         return 1;
}

string redis_getval(string file_id, string col){
         cout << "in redis getval with file:"<<file_id<<" and col:     "<<col<<endl;
         reply = (redisReply *)redisCommand(c,("get "+file_id).c_str());
         if(reply->len!=0) {
                 log_msg("it got a result...");
         } else {
                 log_msg("it did not...");
                 return "null";
         }
         string output=reply->str;
         cout <<output <<"\n";
         size_t exact=output.find("~"+col+":");
         string another="null";
         if(exact!=string::npos){
                 another=output.substr(exact);
                 another=another.substr(2+col.length());
                 size_t exact2=another.find("~");
   		 if(exact2==string::npos){
                         exact2=another.find("}");
                 }
                 another=another.substr(0,exact2);
                 cout<<"another="<<another<<endl;
         }
         log_msg("get1 success");
         cout << "another="<<another<<endl;
         return another;
}


string redis_getkeys(string col, string val){
	printf("in1\n");
	fflush(stdout);
	log_msg("int getkeys");
	string key_query=col;
	cout <<"qeurying:"<<key_query<<"\n";
	reply = (redisReply *)redisCommand(c,("get "+key_query).c_str());
	if(reply->len!=0) {
		log_msg("it got a result2...");
		log_msg(reply->str);
		log_msg("done");
	} else {
		log_msg("it did not...");
		return "null";
	}
	log_msg("here");
	log_msg(reply->str);
	log_msg("there");
	string output=reply->str;
	string another="";
	size_t exact=output.find("~"+val+":");
	if (exact!=string::npos){
		another=output.substr(exact);
		another=another.substr(2+val.length());
		size_t exact2=another.find("~");
		if(exact2==string::npos){
			exact2=another.find("}");
		}
		another=another.substr(0,exact2);
		cout<<"another="<<another<<endl;
	}
	//log_msg("get1 success");
	cout<<"voldemort key value: "<<  output <<"\n";
	printf("out12\n");
	fflush(stdout);
	return another;//return key list of another
}


string redis_getkey_values(string col){
	cout<<"qeury for:"<<col<<endl;
	reply = (redisReply *)redisCommand(c,("get "+col).c_str());
	string output="";
	if(reply->len!=0){
		output=reply->str;
	}
	string ret_val="";
	cout<<"found col with following:"<<output<<endl;
	stringstream ss(output);
	string val;
	while(getline(ss,val,'~')){
		cout << "got val = " << val << endl;
		stringstream ss2(val);
		getline(ss2, val, ':');
		while(getline(ss2, val, ':')){
			ret_val+=val;
		}
	}
	cout << "returning " << ret_val << endl;
	return ret_val;
}





string redis_getkey_cols(string col){
	cout<<"qeury for:"<<col<<endl;
	reply = (redisReply *)redisCommand(c,("get "+col).c_str());
	string output="";
	if(reply->len!=0){
		output=reply->str;
	}
	string ret_val="";
	cout<<"found col with following:"<<output<<endl;
	stringstream ss(output);
	string val;
	while(getline(ss,val,'~')){
		cout << "got val = " << val << endl;
		stringstream ss2(val);
		getline(ss2, val, ':');
		ret_val+=val+":";
	}
	cout << "returning " << ret_val << endl;
	return ret_val;
}





string redis_setval(string file_id, string col, string val){
	cout<<"in redis_setval with file_id:"<<file_id<<" col:"<<col<<" val:"<<val<<endl;
	if(file_id.compare("null")==0){
		string out=redis_getval("last_id","val");
		cout<< "OUT="<<out<<endl;
		if(out.compare("null")==0){
			out="1";
		}
		cout<< "OUT="<<out<<endl;
		string file_id=out;
		last_id=0;
		last_id=atoi(out.c_str());
		cout << "OLD LAST ID="<<last_id<<endl;
		last_id++;//find non-local solution (other table?)
		ostringstream result;
		cout << "NEW LAST ID="<<last_id<<endl;
		result<<last_id;
		cout << "RESULT="<<result.str()<<endl;
		redis_remove_val("last_id","val",out);
		redis_setval("last_id","val",result.str());
		redis_setval(file_id,col,val);
		return file_id;
	}

	cout<<"setting value for file_id:"<<file_id<<endl;

	//handle file_id key
	reply = (redisReply *)redisCommand(c,("get "+file_id).c_str());
	string output;
	if(reply->len!=0){
		output=reply->str;
	} else {
		//create this specific file id
		output="";
	}
	string store=output;
	cout<<"got "<<output<<endl;
	string rest;
	if(store.find("~"+col+":")!=string::npos){//col already set
		string setval=redis_getval(file_id,col);
		int len=setval.length();
		cout<<"col already set to "<<setval<<endl;
		if(setval.find(val)==string::npos){
			setval+=":"+val;
		}
		store.replace(store.find("~"+col+":")+2+col.length(),len,setval);
	} else {
		cout<<"adding col - not already set"<<endl;
		store+="~"+col+":"+val;
	}
	reply = (redisReply *)redisCommand(c,"set %s %s",file_id.c_str(),store.c_str());
	if(reply->len!=0){
		cout<<"\n\nset returned:"<< reply->str <<endl;
	}
	cout<<"put the string "<<store<<" at the key "<<file_id<<endl;


	//handle col key
	cout<<"qeury for:"<<col<<endl;
	reply = (redisReply *)redisCommand(c,("get "+col).c_str());
	output="";
	if(reply->len!=0){
		output=reply->str;
	}
	store=output;
	cout<<"returns:"<<store<<endl;
	rest="";
	if(store.find("~"+val+":")!=string::npos){//col already set
		//log_msg("handling col that already has val!");
		cout<<"old_key:"<<store<<endl;
		rest=store.substr(store.find("~"+val+":"));
		rest=rest.substr(val.length()+2);
		size_t exact2=rest.find("~");
		if(exact2!=string::npos){
			rest=rest.substr(0,exact2);
		}
		cout<<"original values of val:"<<rest<<endl;
		size_t orig_length=rest.length();
		if(rest.find(file_id)==string::npos){
			rest+=":"+file_id;
		}
		cout<<"updated version:"<<rest<<endl;
		store=store.replace(store.find("~"+val+":")+2+val.length(),orig_length,rest);
		cout<<"new key:"<<store<<endl;
	} else {
		store+="~"+val+":"+file_id;
	}
	reply = (redisReply *)redisCommand(c,"set %s %s",col.c_str(),store.c_str());
	cout<<"put the string "<<store<<" at the key "<<col<<endl;
	return file_id;
}



void redis_remove_val(string fileid, string col, string val){
         string replaced=redis_getval(fileid,col);
         cout << "file:"<<fileid<<" col:"<<col<<" val:"<<val<<endl;
         cout << "replaced :"<<replaced<<endl;
         if(replaced.find(val)!=string::npos){
                 cout<<"its here"<<endl;
 
                 //remove from file entry
                 replaced.replace(replaced.find(val),val.length()+1,"");
                 if(replaced.length()>0 && replaced.at(0)==':'){
                         replaced="~"+col+replaced;
                 } else {
                         replaced="~"+col+":"+replaced;
                 }
                 if((replaced.length()-1)>0){
                         cout<<replaced.length()<<endl;
                         if(replaced.at(replaced.length()-1)==':')     {
                                 replaced.erase(replaced.length()-     1);
                         }
                 }
                 cout<<"new replaced:"<<replaced<<endl;
                 reply = (redisReply *)redisCommand(c,"set %s %s",     fileid.c_str(),replaced.c_str());
 
                 //remove from col entry
                 reply = (redisReply *)redisCommand(c,("get "+col).c_str());
                 string sout=reply->str;
                 cout <<"col side:"<<sout<<endl;
                 int len=sout.find("~"+val+":");
                 int len2=sout.find("~",len+1);
                 cout <<"len1:"<<len<<" len2:"<<len2<<" len2-len1:     "<<(len2-len)<<endl;
                 if(len2>0){
                         sout.replace(len,len2-len,"");
                 } else if(len>0) {
                         sout.replace(len,sout.length(),"");
                 } else {
                         sout="";
                 }
                 cout <<"new col:"<<sout<<endl;
                 reply = (redisReply *)redisCommand(c,"set %s %s",     col.c_str(),sout.c_str());
         }
 }


