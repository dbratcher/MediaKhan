#ifndef DATABASE_H
#define DATABASE_H

using namespace std;

class Database {
  public:
    virtual void init() = 0;
    virtual string get(string key) = 0;
    virtual void set(string key, string value) = 0;
    virtual int get_id() = 0;
    virtual void remove(string key, string value) = 0;
    virtual void hset(string hash, string key, string value) = 0;
    virtual string hget(string hash, string key) = 0;
    virtual string type(string key) = 0;
};

#endif
