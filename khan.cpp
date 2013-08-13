
/**
 **    File: khan.cpp
 **    Description: Main functionality for the khan filesystem.
**/

#include "khan.h"
#define VOLDEMORT 1
#define REDIS 2
#define DATABASE REDIS

#define log stderr

#ifdef APPLE
  char* strdup(const char* str) {
    char* newstr = (char*)malloc(strlen(str)+1);
    strcpy(newstr, str);
    return newstr;
  }
#endif

//mkdir stats prints stats to stats file and console:
string stats_file="./stats.txt";
vector<string> servers;
vector<string> server_ids;
string this_server;
string this_server_id;

void process_filetypes(string server) {
  string line;
  ifstream filetypes_file((server+"/filetypes.txt").c_str());
  getline(filetypes_file, line);
  while(filetypes_file.good()){
    cout << "=============== got type =   " << line <<endl;
    //add line to vold as file type
    database_setval("allfiles","types",line);
    database_setval(line,"attrs","all_"+line+"s");
    string asetval=database_setval("all_"+line+"sgen","command","basename");
    string ext=line;
    getline(filetypes_file,line);
    const char *firstchar=line.c_str();
    while(firstchar[0]=='-'){
      //add line to vold under filetype as vold
      stringstream ss(line.c_str());
      string attr;
      getline(ss,attr,'-');
      getline(ss,attr,':');
      string command;
      getline(ss,command,':');
      cout << "============ checking attr = "<<attr<<endl;
      cout << "============ checking command = "<<command<<endl;
      attr=trim(attr);
      database_setval(ext,"attrs",attr);
      database_setval(attr+"gen","command",command);
      getline(filetypes_file,line);
      firstchar=line.c_str();
    }
  }
}

void process_file(string server, string fileid) {
  string file = database_getval(fileid, "name");
  string ext = database_getval(fileid, "ext");
  file = server + "/" + file;
  string attrs=database_getval(ext,"attrs");
  string token="";
  stringstream ss2(attrs.c_str());
  while(getline(ss2,token,':')){
    if(strcmp(token.c_str(),"null")!=0){
      cout << "========= looking at attr =   " << token <<endl;
      string cmd=database_getval(token+"gen","command");
      string msg2=(cmd+" "+file).c_str();
      cout << "========= issuing command =   " << msg2 <<endl;
      FILE* stream=popen(msg2.c_str(),"r");
      if(fgets(msg,200,stream)!=0){
        cout << "========= attr value =   " << msg <<endl;
        database_setval(fileid,token,msg);
      }
      pclose(stream);
    }
  }
}


void unmounting(string mnt_dir) {
  log_msg("in umounting");
  #ifdef APPLE
  string command = "umount " + mnt_dir + "\n";
  #else
  string command = "fusermount -u " + mnt_dir + "\n";
  #endif
  if (system(command.c_str()) < 0) {  
    sprintf(msg,"Could not unmount mounted directory!\n");
    log_msg(msg);
    return;
  }
  log_msg("fusermount successful\n");
}

int initializing_khan(char * mnt_dir) {
  log_msg("In initialize\n");
  unmounting(mnt_dir);
  clock_gettime(CLOCK_REALTIME,&start);
        //Opening root directory and creating if not present
  cout<<"khan_root[0] is "<<servers.at(0)<<endl;
  if(NULL == opendir(servers.at(0).c_str()))  {
    sprintf(msg,"Error msg on opening directory : %s\n",strerror(errno));
    log_msg(msg);
    log_msg("Root directory might not exist..Creating\n");
    string command = "mkdir " + servers.at(0);
    if (system(command.c_str()) < 0) {
      log_msg("Unable to create storage directory...Aborting\n");
      exit(1);
    }
  } else {
    fprintf(stderr, "directory opened successfully\n");
  }

  init_database();

  //check if we've loaded metadata before
  string output=database_getval("setup","value");
  if(output.compare("true")==0){
    log_msg("Database was previously initialized.");
    clock_gettime(CLOCK_REALTIME,&stop);
    tot_time+=(stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION;
    return 0; //setup has happened before
  }

  //if we have not setup, do so now
  log_msg("it hasnt happened, setvalue then setup");
  database_setval("setup","value","true");

  //load metadata associatons
  for(int i=0; i<servers.size(); i++){
    process_filetypes(servers.at(i));
  }

  //load metadata for each file on each server
  string types=database_getval("allfiles","types");
  cout << "================= types to look for ="<<types<<endl;
  for(int i=0; i<servers.size(); i++) {
    glob_t files;
    glob((servers.at(i)+"/*.*").c_str(),0,NULL,&files);
    for(int j=0; j<files.gl_pathc; j++) {//for each file
      string file = files.gl_pathv[j];
      string ext = strrchr(file.c_str(),'.')+1;
      string filename=strrchr(file.c_str(),'/')+1;
      string fileid = database_setval("null","name",filename);
      database_setval(fileid,"ext",ext);
      database_setval(fileid,"server",servers.at(i));
      for(int k=0; k<server_ids.size(); k++) {
        database_setval(fileid, server_ids.at(k), "0");
      }
      process_file(servers.at(i), fileid);
    }
  }
  clock_gettime(CLOCK_REALTIME,&stop);
  tot_time+=(stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION;
  log_msg("At the end of initialize\n");
  return 0;
}


int khan_opendir(const char *c_path, struct fuse_file_info *fi) {
  string path = this_server + c_path;
  DIR* dp = opendir(path.c_str());
  fi->fh = (intptr_t) dp;
  return 0;
}

void calc_time_start(int *calls) {
  clock_gettime(CLOCK_REALTIME,&start);
  (*calls)++;
}

void calc_time_stop(int *calls, double *avg) {
  clock_gettime(CLOCK_REALTIME,&stop);
  time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; 
  tot_time += time_spent;
  *avg=((*avg)*((*calls)-1)+time_spent)/(*calls);
}

static int khan_getattr(const char *path, struct stat *stbuf) {
  calc_time_start(&getattr_calls);
  int res=0;
  time_t current_time;
  time(&current_time);
 
  if(0 == strcmp(path,"/")) {
    log_msg("looking at root");
    stbuf->st_mode=S_IFDIR | 0555;
    string types=database_getval("allfiles","types");
    stbuf->st_nlink=count_string(types)+2;
    stbuf->st_size=4096;
    stbuf->st_atime=current_time;
    stbuf->st_mtime=current_time;
    stbuf->st_ctime=current_time;
    calc_time_stop(&getattr_calls, &getattr_avg_time);
    return 0;
  }

  //decompose path
  stringstream ss0(path+1);
  string type, attr, val, file, more;
  void *tint=getline(ss0, type, '/');
  void *fint=getline(ss0,file, '/');
  void *mint=getline(ss0,more, '/');

  //check type
  if(tint) {
    string types=database_getval("allfiles","types");
    stringstream ss(types.c_str());
    string token;
    string files="";
    int here=0;
    while(getline(ss,token,':')){
      if(strcmp(token.c_str(),type.c_str())==0){
        string attrs=database_getval(type,"attrs");
        //check attr
        int cont=1;
        int found=0;
        do {
          //get attr and val from more
          found=0;
          void *aint=fint;
          string attr=file;
          void *vint=mint;
          string val=more;
          fint=getline(ss0, file, '/');
          mint=getline(ss0, more, '/');
          //if(mint){ cout << "got more"<<endl;}
          //if(fint){ cout << "got file"<<endl;}
          cout << "looking at attr="<<attr<<" val="<<val<<" file="<<file<<" more="<<more<<endl;


          if(!aint) {
            stbuf->st_mode=S_IFDIR | 0555;
            stbuf->st_nlink=count_string(attrs);
            stbuf->st_size=4096;
            calc_time_stop(&getattr_calls, &getattr_avg_time);
            return 0;
          } else {
            stringstream ss2(attrs.c_str());
            while(getline(ss2,token,':')){
              cout << "COMPARING  =" << token << "= TO =" << attr<<"="<< endl;
              if(strcmp(token.c_str(),attr.c_str())==0){
                string  vals=database_getvals(attr);
                cout << "vals="<<vals << " current attr="<<token<<" val="<<val<<endl;
                //check val (loop this and attr later)
                if(!vint){
                  cout << "matched an attr path"<<endl;
                  stbuf->st_mode=S_IFDIR | 0755;
                  stbuf->st_nlink=count_string(vals);
                  stbuf->st_size=4096;
                  calc_time_stop(&getattr_calls, &getattr_avg_time);
                  return 0;
                } else {
                  cout << "more to look at "<<endl;
                  stringstream ss3(vals.c_str());
                  while(getline(ss3, token, ':')){
                    cout << "2comparing "<<token<<" and "<<val<<endl;
                    if(strcmp(token.c_str(),val.c_str())==0){
                      if(strcmp(files.c_str(),"")!=0) {
                        cout << "files =" << files;
                        cout << "new = " << database_getval(attr,val);
                        cout <<"intersect=" << intersect(files,database_getval(attr,val));
                        files=intersect(database_getval(attr, val),files);
                      } else {
                        files=database_getval(attr,val);
                      }

                      //check file
                      if(fint) {
                        cout<< "more to go"<<endl;
                        stringstream ss4(files);
                        while(getline(ss4,token,':')){
                          token=database_getval(token,"name");
                          cout <<"3comparing "<<token<<" and " << file<<endl;
                          if(strcmp(token.c_str(),file.c_str())==0){
                            if(mint){
                              cout << "more left but good so far" <<endl;
                              found=1;
                            } else {
                              stbuf->st_mode=S_IFREG | 0666;
                              stbuf->st_nlink=1;
                              stbuf->st_size=get_file_size(file);
                              calc_time_stop(&getattr_calls, &getattr_avg_time);
                              return 0;
                            }
                          }
                        }
                        stringstream ss5(attrs.c_str());
                        if(found==0){
                          while(getline(ss5,token,':')){
                            if(strcmp(token.c_str(),file.c_str())==0){
                              if(mint) {
                                found=1;
                              } else {
                                stbuf->st_mode=S_IFDIR | 0755;
                                stbuf->st_nlink=count_string(attrs);
                                stbuf->st_size=4096;
                                calc_time_stop(&getattr_calls, &getattr_avg_time);
                                return 0;
                              }
                            }
                          }
                        }
                      } else {
                        cout << "at a  val dir="<<attr<<" or "<<type <<endl;
                        if(strcmp(attr.c_str(),("all_"+type+"s").c_str())==0){
                          cout<<"IN ALL_TYPE folder with val:"<<val<<endl<<endl;
                          stbuf->st_mode=S_IFREG | 0666;
                          stbuf->st_nlink=1;
                          stbuf->st_size=get_file_size(val);
                          calc_time_stop(&getattr_calls, &getattr_avg_time);
                          return 0;
                        } else {
                          stbuf->st_mode=S_IFDIR | 0755;
                          stbuf->st_nlink=count_string(files);
                          stbuf->st_size=4096;
                          calc_time_stop(&getattr_calls, &getattr_avg_time);
                          return 0;
                        }
                      }
                    }
                  }
                }
              }
            }
            log_msg("exiting big block");
            //not valid attr
            if(!found){
              log_msg("not found - return -2");
              calc_time_stop(&getattr_calls, &getattr_avg_time);
              return -2;
            }
          }
        }while(found);
      }
    }
  }
  calc_time_stop(&getattr_calls, &getattr_avg_time);
  return -2;
}

vector<string> split(string str, string delim) {
  int start=0, end;
  vector<string> vec;
  while((end = str.find(delim, start)) != string::npos) {
    if(end-start>delim.length()) {
      vec.push_back(str.substr(start, end-start));
    }
    start = end+delim.length();
  }
  if(end-start>delim.length()) {
    vec.push_back(str.substr(start));
  }
  return vec;
}

static int xmp_readdir(const char *c_path, void *buf, fuse_fill_dir_t filler,off_t offset, struct fuse_file_info *fi) {
  calc_time_start(&readdir_calls);
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  
  stringstream path(c_path+1);
  string attr, val, file, more;
  void* aint=getline(path, attr, '/');
  void* vint=getline(path, val, '/');
  void* mint=getline(path, more, '/');

  if(aint) {
    cout << "some path, not implemented" << endl;
  } else {
    // at root, return all attrs
    vector<string> attrs = split(database_getvals("attrs"),":");
    for(int i=0; i<attrs.size(); i++) {
      filler(buf, attrs[i].c_str(), NULL, 0);
    }
  }

  calc_time_stop(&readdir_calls, &readdir_avg_time);
  return 0;
}

int khan_open(const char *path, struct fuse_file_info *fi)
{
  log_msg("in khan_open");
  //update usage count
  string filename = basename(strdup(path));
  string fileid = database_getval("name",filename);
  string val = database_getval(fileid, this_server_id);
  int value = atoi(val.c_str())+1;
  ostringstream new_val;
  new_val << value;
  cout << "new val:" << new_val.str() <<endl;
  database_setval(fileid, this_server_id, new_val.str()); 
  database_remove_val(fileid, this_server_id, val);
  int retstat = 0;
  int fd;
  path=append_path2(basename(strdup(path)));

  sprintf(msg,"In khan_open path : %s\n",path);
  log_msg(msg);
  fd = open(path, fi->flags);
  fi->fh = fd;
  return 0;
}


int xmp_access(const char *path, int mask)
{
    calc_time_start(&access_calls);
    fprintf(log, "in xmp_access with path: %s\n", path);

    char *path_copy=strdup(path);
  if(strcmp(path,"/")==0) {
    log_msg("at root");
    calc_time_stop(&access_calls, &access_avg_time);
    return 0;
  }
//  if(strcmp(path,"/")==0) {
        log_msg("at root");
        calc_time_stop(&access_calls, &access_avg_time);
        return 0;
//  }

  string dirs=database_getval("alldirs","paths");
  string temptok="";
  stringstream dd(dirs);
  while(getline(dd,temptok,':')){
    if(strcmp(temptok.c_str(),path)==0){
      calc_time_stop(&access_calls, &access_avg_time);
      return 0;
    }
  }

  int c=0;
  for(int i=0; path[i]!='\0'; i++){
    if(path[i]=='/') c++;
  }

  //decompose path
  stringstream ss0(path+1);
  string type, attr, val, file, more;
  void* tint=getline(ss0, type, '/');
  void* fint=getline(ss0, file, '/');
  void* mint=getline(ss0, more, '/');
  int reta=0;

  //check for filetype
  if(tint){
    string types = database_getval("allfiles","types");
    stringstream ss(types.c_str());
    string token;
    while(getline(ss,token,':')){
      if(strcmp(type.c_str(),token.c_str())==0){
        reta=1;
      }
    }
    int found=0;

    do{
      //get attr and val
      found=0;
      void *aint=fint;
      string attr=file;
      void *vint=mint;
      string val=more;
      fint=getline(ss0, file, '/');
      mint=getline(ss0, more, '/');

      //check for attr
      if(reta && aint) {
        cout << attr << endl;
        string attrs= database_getval(type,"attrs");
        stringstream ss3(attrs.c_str());
        reta=0;
        while(getline(ss3,token,':')){
          if(strcmp(attr.c_str(), token.c_str())==0){
            reta=1;
          }
        }

        //check for val
        if(reta && vint) {
          cout << val << endl;
          if(strcmp(attr.c_str(),("all_"+type+"s").c_str())==0) {
            clock_gettime(CLOCK_REALTIME,&stop);
            time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
            access_avg_time=(access_avg_time*(access_calls-1)+time_spent)/access_calls;
            return 0;
          }
          string vals=database_getvals(attr);
          stringstream ss4(vals.c_str());
          reta=0;
          while(getline(ss4,token,':')){
            cout << val << token << endl;
            if(strcmp(val.c_str(), token.c_str())==0){
              reta=1;
            }
          }

          //check for file
          if(reta && fint) {
            cout << file << endl;
            string files=database_getval(attr, val);
            stringstream ss4(files.c_str());
            if(!mint) {
              reta=0;
              while(getline(ss4,token,':')){
                token=database_getval(token,"name");
                if(strcmp(file.c_str(), token.c_str())==0){
                  reta=1;
                }
              }
              stringstream ss5(attrs.c_str());
              while(getline(ss5,token,':')){
                if(strcmp(file.c_str(),token.c_str())==0){
                  reta=1;
                }
              }
            } else {
              found=1;
            }
          }
        }
      }
    }while(found);
  }

  if(reta && !getline(ss0, val, '/')) {
    calc_time_stop(&access_calls, &access_avg_time);
    return 0;
  }
    path=append_path(path);
    int ret = access(path, mask);
  calc_time_stop(&access_calls, &access_avg_time);
    return ret;
}


static int xmp_mknod(const char *path, mode_t mode, dev_t rdev) {
      log_msg("in xmp_mknod");

  path=append_path2(basename(strdup(path)));
    sprintf(msg,"khan_mknod, path=%s\n",path);
    log_msg(msg);
    int res;
    if (S_ISFIFO(mode))
    res = mkfifo(path, mode);
    else
    res = mknod(path, mode, rdev);
    if (res == -1) {
    fprintf(stderr, "\nmknod error \n");
    return -errno;
    }
    return 0;
}


static int xmp_mkdir(const char *path, mode_t mode) {
  struct timespec mkdir_start, mkdir_stop;
  string strpath=path;
  if(strpath.find("localize")!=string::npos) {
    clock_gettime(CLOCK_REALTIME,&mkdir_start);
    if(strpath.find("usage")!=string::npos) {
      usage_localize();
    } else {
      cout << "LOCALIZING" << endl;
      cout << strpath << endl;
      //check location
      string filename = "winter.mp3";
      string fileid = database_getval("name", filename);
      string location = get_location(fileid);
      string server = database_getval(fileid, "server");
      cout << "======== LOCATION: " << location << endl << endl;
      //if not current
      if(location.compare(server)!=0) {
        //  move to new location
        cout << " MUST MOVE "<<server<<" TO "<<location<<endl;
        database_setval(fileid,"server",location);
        string from = server + "/" + filename;
        string to = location + "/" + filename;
        string command = "mv " + from + " " + to;
        FILE* stream=popen(command.c_str(),"r");
        pclose(stream);
        //rename(from.c_str(), to.c_str());
      }
    }
    clock_gettime(CLOCK_REALTIME,&mkdir_stop);
    localize_time = (mkdir_stop.tv_sec-mkdir_start.tv_sec)+(mkdir_stop.tv_nsec-mkdir_start.tv_nsec)/BILLION; 
    cout << "LOCALIZATION TIME:" << localize_time << endl <<endl;
    return -1;
  }
  if(strpath.find("stats")!=string::npos){
    //print stats and reset
    ofstream stfile;
    stfile.open(stats_file.c_str(), ofstream::out);
    stfile << "TOT TIME    :" << tot_time << endl;
    stfile << "Vold Calls   :" << vold_calls << endl;
    stfile << "     Avg Time:" << vold_avg_time << endl;
    stfile << "Readdir Calls:" << readdir_calls << endl;
    stfile << "     Avg Time:" << readdir_avg_time << endl;
    stfile << "Access Calls :" << access_calls << endl;
    stfile << "     Avg Time:" << access_avg_time << endl;
    stfile << "Read Calls   :" << read_calls << endl;
    stfile << "     Avg Time:" << read_avg_time << endl;
    stfile << "Getattr Calls:" << getattr_calls << endl;
    stfile << "     Avg Time:" << getattr_avg_time << endl;
    stfile << "Write Calls  :" << write_calls << endl;
    stfile << "     Avg Time:" << write_avg_time << endl;
    stfile << "Create Calls :" << create_calls << endl;
    stfile << "     Avg Time:" << create_avg_time << endl;
    stfile << "Rename Calls :" << rename_calls << endl;
    stfile << "     Avg Time:" << rename_avg_time << endl;
    stfile.close();
    cout << "TOT TIME    :" << tot_time << endl;
    cout << "Vold Calls   :" << vold_calls << endl;
    cout << "     Avg Time:" << vold_avg_time << endl;
    cout << "Readdir Calls:" << readdir_calls << endl;
    cout << "     Avg Time:" << readdir_avg_time << endl;
    cout << "Access Calls :" << access_calls << endl;
    cout << "     Avg Time:" << access_avg_time << endl;
    cout << "Read Calls   :" << read_calls << endl;
    cout << "     Avg Time:" << read_avg_time << endl;
    cout << "Getattr Calls:" << getattr_calls << endl;
    cout << "     Avg Time:" << getattr_avg_time << endl;
    cout << "Write Calls  :" << write_calls << endl;
    cout << "     Avg Time:" << write_avg_time << endl;
    cout << "Create Calls :" << create_calls << endl;
    cout << "     Avg Time:" << create_avg_time << endl;
    cout << "Rename Calls :" << rename_calls << endl;
    cout << "     Avg Time:" << rename_avg_time << endl;
    vold_calls=0;
    readdir_calls=0;
    access_calls=0;
    getattr_calls=0;
    read_calls=0;
    write_calls=0;
    create_calls=0;
    rename_calls=0;
    tot_time=0;
    vold_avg_time=0;
    readdir_avg_time=0;
    access_avg_time=0;
    getattr_avg_time=0;
    read_avg_time=0;
    write_avg_time=0;
    create_avg_time=0;
    rename_avg_time=0;
    return -1;
  }

  log_msg("xmp_mkdir");
  sprintf(msg,"khan_mkdir for path=%s\n",path);
  log_msg(msg);
  struct stat *st;
  if(khan_getattr(path, st)<0) {
    //add path
    database_setval("alldirs","paths",path);
    //and break into attr/val pair and add to vold
  } else {
    log_msg("Directory exists\n");
  }
  return 0;
}


static int xmp_readlink(const char *path, char *buf, size_t size) {
        log_msg("xmp_readlink");
  //TODO: handle in vold somehow
  log_msg("In readlink\n");
        int res = -1;
        path=append_path2(basename(strdup(path)));
        //res = readlink(path, buf, size - 1);
        if (res == -1)
                return -errno;
        buf[res] = '\0';
        return 0;
}


static int xmp_unlink(const char *path) {
  log_msg("in xmp_unlink");
  //TODO: handle in vold somehow
  int res;
  string fileid=database_getval("name",basename(strdup(path)));

    string fromext=database_getval(fileid,"ext");
    string file=append_path2(basename(strdup(path)));
    string attrs=database_getval(fromext,"attrs");
    cout << fromext <<  fileid << endl;
    cout<<"HERE!"<<endl;
    database_remove_val(fileid,"attrs","all_"+fromext+"s");
    cout<<"THERE!"<<endl;
    //database_remove_val("all_"+fromext+"s",strdup(basename(strdup(from))),fileid);
    cout<<"WHERE!"<<endl;
    string token="";
    stringstream ss2(attrs.c_str());
    while(getline(ss2,token,':')){
      if(strcmp(token.c_str(),"null")!=0){
        string cmd=database_getval(token+"gen","command");
        string msg2=(cmd+" "+file).c_str();
        FILE* stream=popen(msg2.c_str(),"r");
        if(fgets(msg,200,stream)!=0){
          database_remove_val(fileid,token,msg);
        }
                                pclose(stream);
      }
    }

        path=append_path2(basename(strdup(path)));
        res = unlink(path);
  if (res == -1)
    return -errno;
  return 0;
}


static int xmp_rmdir(const char *path) {
  //if hardcoded, just remove
  database_remove_val("alldirs","paths",path);

  //if exists
    //get contained files
    //get attrs+vals from path
    //unset files attrs
    //if entire attr, remove attr

  return 0;
}

static int xmp_symlink(const char *from, const char *to) {
  log_msg("in xmp_symlink");
  //TODO: handle in vold somehow
  int res=-1;
  from=append_path2(basename(strdup(from)));
        to=append_path2(basename(strdup(to)));
        sprintf(msg,"In symlink creating a symbolic link from %s to %s\n",from, to);
  log_msg(msg);
  //res = symlink(from, to);
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_rename(const char *from, const char *to) {
  calc_time_start(&rename_calls);
  log_msg("in xmp_rename");
  int res;
        sprintf(msg, "0000000000000000000000000000000000000000000000000000000000000000000000000\n-----------------------------In rename from %s to %s\n",from, to);
        log_msg(msg);

  //get from fileid
  cout <<basename(strdup(from)) << " is the filename "<<endl;
  string fileid=database_getval("name",basename(strdup(from)));
  cout << "moving internal file(manipulating attrs):"<<fileid<<endl;


  if(strcmp(basename(strdup(from)),basename(strdup(to)))!=0){
    rename(append_path2(basename(strdup(from))), append_path2(basename(strdup(to))));
    string fromext=database_getval(fileid,"ext");
    string file=append_path2(basename(strdup(from)));
    string attrs=database_getval(fromext,"attrs");
    cout << fromext <<  fileid << endl;
    cout<<"HERE!"<<endl;
    database_remove_val(fileid,"attrs","all_"+fromext+"s");
    cout<<"THERE!"<<endl;
    //database_remove_val("all_"+fromext+"s",strdup(basename(strdup(from))),fileid);
    cout<<"WHERE!"<<endl;
    string token="";
    stringstream ss2(attrs.c_str());
    while(getline(ss2,token,':')){
      if(strcmp(token.c_str(),"null")!=0){
        string cmd=database_getval(token+"gen","command");
        string msg2=(cmd+" "+file).c_str();
        FILE* stream=popen(msg2.c_str(),"r");
        if(fgets(msg,200,stream)!=0){
          database_remove_val(fileid,token,msg);
        }
                                pclose(stream);
      }
    }
    //remove from from database
  }




  //set fileid name to basename(to)
  database_remove_val(fileid,"name",basename(strdup(from)));
  database_setval(fileid,"name",basename(strdup(to)));

  //decompose from path
  string type, attr, val, file;
  stringstream ss(from);
  void* tint=getline(ss,type,'/');
  tint=getline(ss,type,'/');


  //decompose to path
  stringstream ss2(to);
  tint=getline(ss2,type,'/');
  tint=getline(ss2,type,'/');
  if(tint){
    void* aint=getline(ss2,attr,'/');
    void* vint=getline(ss2,val,'/');

    //for each attr/val pair
    while(aint && vint) {
      //if attr is not there
      if(database_getval(type,"attrs").find(attr)==string::npos){
        //add attr to type
        database_setval(type,"attrs",attr);
      }

      //database-set fileid attr val
      database_setval(fileid,attr,val);
      aint=getline(ss2,attr,'/');
      vint=getline(ss2,val,'/');

    }
  }

  //if to dir is in all dirs, remove it
  string dirs=database_getval("alldirs","paths");
  stringstream dd(dirs);
  string sto=to;
  string tok;
  while(getline(dd,tok,':')){
    cout<<"4comparing sto:"<<sto<<" to tok:"<<tok<<endl;
    if(sto.find(tok)!=string::npos){
      database_remove_val("alldirs","paths",tok);
    }
  }
  calc_time_stop(&rename_calls, &rename_avg_time);
  return 0;
}

static int xmp_link(const char *from, const char *to) {
  log_msg("in xmp_link");
  //TODO:handle in vold somehow...
        int retstat = 0;
        from=append_path2(basename(strdup(from)));
        to=append_path2(basename(strdup(to)));
        sprintf(msg,"khan_link initial path=\"%s\", initial to=\"%s\")\n",from, to);
        log_msg(msg);
        retstat = link(from,to);
      return retstat;
}

static int xmp_chmod(const char *path, mode_t mode) {
  log_msg("in xmp_chmod");

  int res;
        path=append_path2(basename(strdup(path)));
        sprintf(msg, "In chmod for: %s\n",path);
        log_msg(msg);
  res = chmod(path, mode);
#ifdef APPLE
        res = chmod(path, mode);
#else
        res = chmod(path, mode);
#endif
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_chown(const char *path, uid_t uid, gid_t gid) {
  log_msg("in xmp_chown");
  int res;
        path=append_path2(basename(strdup(path)));
        sprintf(msg,"In chown for : %s\n",path);
        log_msg(msg);
  res = lchown(path, uid, gid);
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_truncate(const char *path, off_t size) {
  //update for vold?
  log_msg("In xmp_truncate\n");
  int res;
        path++;
  res = truncate(path, size);
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_utimens(const char *path, const struct timespec ts[2]) {
  log_msg("in utimens");
  int res;
  struct timeval tv[2];
        path=append_path2(basename(strdup(path)));
        sprintf(msg,"in utimens for path : %s\n",path);
  log_msg(msg);
        tv[0].tv_sec = ts[0].tv_sec;
  tv[0].tv_usec = ts[0].tv_nsec / 1000;
  tv[1].tv_sec = ts[1].tv_sec;
  tv[1].tv_usec = ts[1].tv_nsec / 1000;
  res = utimes(path, tv);
  if (res == -1)
    return -errno;
  return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  calc_time_start(&read_calls);
       log_msg("in xmp_read");

  //if just created
    //set created and read
  //if just created, read and wrote
    //do nothing and quit

   int fd,j=0;
      int res;
      int khan_write =0;
      char newpath[100];
      sprintf(msg,"KHAN_READ >> PATH : %s\n",path); log_msg(msg);
    path=append_path2(basename(strdup(path)));
  cout<<"PATH NAME!!!!!"<<path<<endl<<endl<<endl;
          (void) fi;
          FILE *thefile = fopen(path, "r");

      if (thefile == NULL){
    calc_time_stop(&read_calls, &read_avg_time);
    return -errno;
  }
  fseek(thefile, offset, SEEK_SET);
      res = fread(buf, 1, size, thefile);

  cout << "READ THIS MANY !!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl<<res<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;

      if (res == -1)
  res = -errno;
        close(fd);
  calc_time_stop(&read_calls, &read_avg_time);
      return res;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
  clock_gettime(CLOCK_REALTIME,&start);
  write_calls++;
  log_msg("in xmp_write");
  int fd;
  int res;
  
  path=append_path2(basename(strdup(path)));
  (void) fi;
  fd = open(path, O_WRONLY);
  if (fd == -1){
    clock_gettime(CLOCK_REALTIME,&stop);
    time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
    write_avg_time=(write_avg_time*(write_calls-1)+time_spent)/write_calls;
    return errno;
  }
  res = pwrite(fd, buf, size, offset);
  if (res == -1)
    res = errno;
  close(fd);
  clock_gettime(CLOCK_REALTIME,&stop);
  time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
  write_avg_time=(write_avg_time*(write_calls-1)+time_spent)/write_calls;
  return res;
}

static int xmp_statfs(const char *path, struct statvfs *stbuf) {
    /* Pass the call through to the underlying system which has the media. */
    fprintf(log, "in xmp_statfs with path %s\n", path);
    int res = statvfs(path, stbuf);
    if (res != 0) {
        fprintf(log, "statfs error for %s\n",path);
        return errno;
    }
    return 0;
}

static int xmp_release(const char *path, struct fuse_file_info *fi) {
    /* Just a stub. This method is optional and can safely be left unimplemented. */
    fprintf(log, "in xmp_release with path %s\n", path);
    return 0;
}

static int xmp_fsync(const char *path, int isdatasync,struct fuse_file_info *fi) {
    /* Just a stub. This method is optional and can safely be left unimplemented. */
    fprintf(log, "in xmp_fsync with path %s\n", path);
    return 0;
}


void *khan_init(struct fuse_conn_info *conn) {

    log_msg("khan_init() called!\n");
    sprintf(msg,"khan_root is : %s\n",servers.at(0).c_str());log_msg(msg);
    if(chdir(servers.at(0).c_str())<0) {
       sprintf(msg,"could not change directory ,errno %s\n",strerror(errno)); log_msg(msg);
       perror(servers.at(0).c_str());
    }
    sprintf(msg,"AT THE END OF INIT\n"); log_msg(msg);
    return KHAN_DATA;
}



int khan_flush (const char * path, struct fuse_file_info * info ) {
  cout << "=============IN KHAN FLUSH!!!!!!!!" << endl << endl;
  string filename = basename(strdup(path));
  string fileid=database_getval("name",filename);
  string server=database_getval(fileid,"server");
  process_file(server, fileid);

  return 0;
}


int khan_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{

  clock_gettime(CLOCK_REALTIME,&start);
  create_calls++;

  log_msg("in khan_create");
  int retstat = 0;
  int fd=0;

  string fileid=database_getval("name",basename(strdup(path)));

  //if file name is in system dont create, just update metadata
  if(strcmp(fileid.c_str(),"null")!=0){
    cout << "FILE ALREADY EXISTS!"<<endl;
  } else {
    //get from fileid
    fileid=database_setval("null","name",basename(strdup(path)));
    cout << "creating internal file(manipulating attrs):"<<fileid<<endl;
  }


  string spath=path;
  string mytype=spath.substr(1,spath.find("/",1)-1);
  string dtype=database_getval(mytype,"attrs");
  database_setval(fileid,"ext",mytype);
  if(strcmp(dtype.c_str(),"null")!=0){
    //get all attrs, set in database
    database_setval(fileid,"server",servers.at(0));
    string attrs=database_getval(mytype,"attrs");
    string token="";
    stringstream ss2(attrs.c_str());
    while(getline(ss2,token,':')){
      if(strcmp(token.c_str(),"null")!=0){
        cout << "=============== looking at attr =   " << token <<endl;
        string cmd=database_getval(token+"gen","command");
        string msg2=(cmd+" "+servers.at(0)+"/"+basename(strdup(path))).c_str();
        cout << "=============== issuing command =   " << msg2 <<endl;
        FILE* stream=popen(msg2.c_str(),"r");
        if(fgets(msg,200,stream)!=0){
          cout << "=============== attr value =   " << msg <<endl;
          database_setval(fileid,token,msg);
        }
        pclose(stream);
      }
    }
  } else {
    //create filetype

    //add line to vold as file type
    database_setval("allfiles","types",mytype);
    database_setval(mytype,"attrs","all_"+mytype+"s");
    string asetval=database_setval("all_"+mytype+"sgen","command","basename");
  }

  //decompose path
  string type, attr, val;
  stringstream ss(path);
  void* tint=getline(ss,type,'/');
  tint=getline(ss,type,'/');
  if(tint){
    void* aint=getline(ss,attr,'/');
    void* vint=getline(ss,val,'/');

    //for each attr/val pair
    while(aint && vint) {
      //if attr is not there
      if(database_getval(type,"attrs").find(attr)==string::npos){
        //add attr to type
        database_setval(type,"attrs",attr);
      }

      //database-set fileid attr val
      database_setval(fileid,attr,val);
      aint=getline(ss,attr,'/');
      vint=getline(ss,val,'/');

    }
  }

  //if to dir is in all dirs, remove it
  string dirs=database_getval("alldirs","paths");
  stringstream dd(dirs);
  string sto=path;
  string tok;
  while(getline(dd,tok,':')){
    cout<<"5comparing sto:"<<sto<<" to tok:"<<tok<<endl;
    if(sto.find(tok)!=string::npos){
      database_remove_val("alldirs","paths",tok);
    }
  }

  //open file handle to create on file system
  path=append_path2(basename(strdup(path)));
  fd = open(path,fi->flags, mode);
  close(fd);


  clock_gettime(CLOCK_REALTIME,&stop);
  time_spent = (stop.tv_sec-start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION; tot_time += time_spent;;
  create_avg_time=(create_avg_time*(create_calls-1)+time_spent)/create_calls;
  return 0;
}

int khan_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
  log_msg("in khan_fgetattr");

    int retstat = 0;
    sprintf(msg,"khan_fgetattr(path=%s \n",path);
    retstat = fstat(fi->fh, statbuf);
    return retstat;
}
#ifdef APPLE
static int xmp_setxattr(const char *path, const char *name, const char *value,  size_t size, int flags, uint32_t param) {
#else
static int xmp_setxattr(const char *path, const char *name, const char *value,  size_t size, int flags) {
#endif
  return 0;
}

static int xmp_getxattr(const char *path, const char *name, char *value, size_t size) {
  return 0;
}

static int xmp_listxattr(const char *path, char *list, size_t size) {
  return 0;
}

static int xmp_removexattr(const char *path, const char *name) {
  return 0;
}

#ifdef APPLE

static int xmp_setvolname(const char* param) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_exchange(const char* param1, const char* param2, unsigned long param3) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_getxtimes(const char* param1, struct timespec* param2, struct timespec* param3) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_setbkuptime(const char* param1, const struct timespec* param2) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_setchgtime(const char* param1, const struct timespec* param2) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_setcrtime(const char* param1, const struct timespec* param2) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_chflags(const char* param1, uint32_t param2) {
    log_msg("apple function called\n");
    return 0;
}
static int xmp_setattr_x(const char* param1, struct setattr_x* param2) {
    log_msg("apple function called\n");
    return 0;
}

static int xmp_fsetattr_x(const char* param1, struct setattr_x* param2, struct fuse_file_info* param3) {
    log_msg("apple function called\n");
    return 0;
}

#endif
struct khan_param {
        unsigned                major;
        unsigned                minor;
        char                    *dev_name;
        int                     is_help;
};

#define KHAN_OPT(t, p) { t, offsetof(struct khan_param, p), 1 }

static const struct fuse_opt khan_opts[] = {
        KHAN_OPT("-s %s",            dev_name),
        KHAN_OPT("--cs %s",        dev_name),
        FUSE_OPT_END
};

static int khan_process_arg(void *data, const char *arg, int key, struct fuse_args *outargs) {
        struct khan_param *param = (struct khan_param*)data;
        fprintf(stderr,"param.dev_name : %s\n",param->dev_name);
        (void)outargs;
        (void)arg;
        switch (key) {
          case 0:
                  param->is_help = 1;
                  return fuse_opt_add_arg(outargs, "-ho");
          default:
                   return 1;
        }
}

static struct fuse_operations khan_ops;


int main(int argc, char *argv[])
{
  khan_ops.getattr  = khan_getattr;
  khan_ops.init     = khan_init;
  khan_ops.access    = xmp_access;
  khan_ops.readlink  = xmp_readlink;
  khan_ops.readdir  = xmp_readdir;
  khan_ops.mknod    = xmp_mknod;
  khan_ops.mkdir    = xmp_mkdir;
  khan_ops.symlink  = xmp_symlink;
  khan_ops.unlink    = xmp_unlink;
  khan_ops.rmdir    = xmp_rmdir;
  khan_ops.rename    = xmp_rename;
  khan_ops.link    = xmp_link;
  khan_ops.chmod    = xmp_chmod;
  khan_ops.chown    = xmp_chown;
  khan_ops.truncate  = xmp_truncate;
  khan_ops.create   = khan_create;
  khan_ops.utimens  = xmp_utimens;
  khan_ops.open    = khan_open;
  khan_ops.read    = xmp_read;
  khan_ops.write    = xmp_write;
  khan_ops.statfs    = xmp_statfs;
  khan_ops.release  = xmp_release;
  khan_ops.fsync    = xmp_fsync;
  khan_ops.opendir  = khan_opendir;
  khan_ops.flush    = khan_flush;
  khan_ops.getxattr  = xmp_getxattr;
#ifdef APPLE
  khan_ops.setxattr  = xmp_setxattr;
  khan_ops.listxattr  = xmp_listxattr;
  khan_ops.removexattr  = xmp_removexattr;
  khan_ops.setvolname     = xmp_setvolname;
  khan_ops.exchange       = xmp_exchange;
  khan_ops.getxtimes      = xmp_getxtimes;
  khan_ops.setbkuptime    = xmp_setbkuptime;
  khan_ops.setchgtime     = xmp_setchgtime;
  khan_ops.setcrtime      = xmp_setcrtime;
  khan_ops.chflags        = xmp_chflags;
  khan_ops.setattr_x      = xmp_setattr_x;
  khan_ops.fsetattr_x     = xmp_fsetattr_x;
#endif


  int retval=0;
  struct khan_param param = { 0, 0, NULL, 0 };
  if((argc<2)||(argc>4)) {
    printf("Usage: ./khan <mount_dir_location> [stores.txt] [-d]\nAborting...\n");
    exit(1);
  }

  struct fuse_args args = FUSE_ARGS_INIT(0, NULL);
  int j;
  const char* store_filename="stores.txt";
  for(j = 0; j < argc; j++) {
    if((j == 2) && (argv[j][0]!='-')) {
      store_filename = argv[j];
    } else {
      fuse_opt_add_arg(&args, argv[j]);
    }
  }
     
  fprintf(stderr, "store filename: %s\n", store_filename);
  FILE* stores = fopen(store_filename, "r");
  char buffer[100];
  char buffer2[100];
  fscanf(stores, "%s\n", buffer);
  this_server_id = buffer;
  while(fscanf(stores, "%s %s\n", buffer, buffer2)!=EOF) {
    servers.push_back(buffer);
    server_ids.push_back(buffer2);
    if(this_server_id == buffer2) {
      this_server = buffer;
    }
  }
  fclose(stores);
  umask(0);
  if(-1==log_open()) {
    printf("Unable to open the log file..NO log would be recorded..!\n");
  }
  log_msg("\n\n--------------------------------------------------------\n");
  khan_data = (khan_state*)calloc(sizeof(struct khan_state), 1);
  if (khan_data == NULL)  {
    log_msg("Could not allocate memory to khan_data!..Aborting..!\n");
    abort();
  }
  if(initializing_khan(argv[1])<0)  {
    log_msg("Could not initialize khan..Aborting..!\n");
    return -1;
  }
  log_msg("initialized....");
  retval=fuse_main(args.argc,args.argv, &khan_ops, khan_data);
  log_msg("Done with fuse_main...\n");
  return retval;
}
