#include "database.h"
#include "utils.h"

#ifdef APPLE
  int clock_gettime(int i, struct timespec* b) { 
    return 0;
  }

  char* strdup(const char* str) {
    char* newstr = (char*)malloc(strlen(str)+1);
    strcpy(newstr, str);
    return newstr;
  }
#endif

int get_file_size(string file_name){
  string tempbase = file_name;
        string base=basename(strdup(tempbase.c_str()));
  string path=append_path2(base);
  struct stat st;
  stat(path.c_str(), &st);
  return st.st_size;
}

vector<string> split(string istr, string delim) {
  int start=0, end; 
  vector<string> vec; 
  char *saveptr;
  char *token;
  char *str = strdup(istr.c_str());
  const char *delimiters = delim.c_str();
  for(token = strtok_r(str, delimiters, &saveptr);
    token != NULL;
    token = strtok_r(NULL, delimiters, &saveptr)) {
    vec.push_back(string(token));
  }
  return vec; 
}

string join(vector<string> these, string delim) {
  string ret = "";
  for(int i=0; i<these.size(); i++) {
    if(i>0) {
      ret+=delim;
    }
    ret+=these[i];
  }
  return ret; 
}


string subtract(string files1, string files2){
  string ret="";
  string tok1="";
  string tok2="";
  stringstream f1(files1);
  while(getline(f1,tok1,':')){
    stringstream f2(files2);
    bool found = false;
    while(getline(f2,tok2,':')){
      if(strcmp(tok1.c_str(),tok2.c_str())==0){
        found = true;        
      }
    }
    if(!found) {
      ret+=":"+tok1;
    }
  }
  return ret;
}
string intersect(string files1, string files2){
  string ret="";
  string tok1="";
  string tok2="";
  stringstream f1(files1);
  while(getline(f1,tok1,':')){
    stringstream f2(files2);
    while(getline(f2,tok2,':')){
      if(strcmp(tok1.c_str(),tok2.c_str())==0){
        ret+=":"+tok1;
      }
    }
  }
  return ret;
}

string trim_right(string source, string t = " \n")
{
  string str = source;
  return str.erase( str.find_last_not_of(t) + 1);
}

string trim_left( string source, string t = " \n")
{
  std::string str = source;
  return str.erase(0 , source.find_first_not_of(t) );
}

string trim(string source, string t)
{
  string str = source;
  return trim_left( trim_right( str , t) , t );
}


int count_string(string tobesplit){
  int count=0;
  if(strcmp(tobesplit.c_str(),"null")==0){
    return 0;
  } else {
    stringstream ss(tobesplit.c_str());
    string token;
    while(getline(ss, token, ':')){
      if(token.length()>0) {
        count++;
      }
    }
    return count;
  }
}


char* append_path(const char * newp) {
  char msg[100];
  sprintf(msg,"in append_path with %s and %s",servers[0].c_str(),newp);
  log_msg(msg);
  fpath=(char*)malloc(MAX_PATH_LENGTH);
      memset(fpath,0,MAX_PATH_LENGTH);
          sprintf(&fpath[0],"%s%s",servers[0].c_str(),newp);
  //log_msg("returning");
            return fpath;
}

char* append_path2(string newp) {
  //get file from database
  //cout<<"in append_path2"<<endl;
  string fid=database_getval("name",newp);
  //cout<<"got fid:"<<fid<<endl;
  //get server name
  string server=database_getval(fid,"server");
  //cout<<"got server:"<<server<<endl;
  vector<string> places = split(server, ":");
  int i=0;
  server = "";
  for(i=0; i<places.size(); i++) {
    //prefer local to cloud
    if(places[i]=="cloud" && server=="") {
      server = "/tmp";
    } else {
      server = places[i];
    }
  }
  //append and return c_str
  return strdup((server+"/"+newp).c_str());
}




