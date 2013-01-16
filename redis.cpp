#include "redis.h"

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


