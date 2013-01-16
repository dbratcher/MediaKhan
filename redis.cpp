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


