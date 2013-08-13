
/*
  khan.cpp
  supports filesystem calls
*/

#include "khan.h"
#include "database.h"
#include "redis.h"

using namespace std;


// global variables
static struct fuse_operations khan_ops;
vector<string> servers;
vector<string> server_ids;
string this_server_id;
string this_server;
Database* database;

static int khan_get_attr(const char* c_path, struct stat* stbuf) {
  string path = c_path;

  // check if root
  if(path == "/") {
    stbuf->st_mode = S_IFDIR | 0555;
    stbuf->st_nlink = 2;
    stbuf->st_size = 68;
  }

  return 0;
}

void* khan_init(struct fuse_conn_info* conn) {
  return NULL;
}

int khan_access(const char* c_path, int mask) {
  string path = c_path; 
 
  // check if root
  if(path == "/") {
    return 0;
  }

  // else doesn't exist
  return -1;
}

static int khan_readlink(const char* path, char* buf, size_t size) {
  return readlink(path, buf, size);
}

static int khan_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);
  return 0;
} 

static int khan_mknod(const char* path, mode_t mode, dev_t rdev) {
  return mknod(path, mode, rdev);
}

static int khan_mkdir(const char* path, mode_t mode) {
  return 0;
}

static int khan_symlink(const char* from, const char* to) {
  return 0;
}

static int khan_unlink(const char* path) {
  return 0;
}

static int khan_rmdir(const char* path) {
  return 0;
}

static int khan_rename(const char* from, const char* to) {
  return 0;
}

static int khan_link(const char* from, const char* to) { return 0; }

static int khan_chmod(const char* path, mode_t mod) { return 0; }

static int khan_chown(const char* path, uid_t uid, gid_t gid) { return 0; }

static int khan_truncate(const char* path, off_t size) { return 0; }

static int khan_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
  return 0;
}

static int khan_utimens(const char* path, const struct timespec ts[2]) { return 0; }

static int khan_open(const char* path, struct fuse_file_info* fi) { return 0; }

static int khan_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) { return 0; }

static int khan_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) { return 0; }

static int khan_statfs(const char* path, struct statvfs* stbuf) { return 0; }

static int khan_release(const char* path, struct fuse_file_info* fi) { return 0; }

static int khan_fsync(const char* path, int isdatasync, struct fuse_file_info* fi) { return 0; }

static int khan_opendir(const char* path, struct fuse_file_info* fi) { return 0; }

static int khan_flush(const char* path, struct fuse_file_info* info) { return 0; }

// Apple Functionality
#ifdef APPLE
  static int khan_setxattr(const char* path, const char* name, const char* value,  size_t size, int flags, uint32_t param) {
    return 0;
  }

  static int khan_getxattr(const char* path, const char* name, char* value, size_t size, uint32_t param) {
    return 0;
  }

  static int khan_listxattr(const char* path, char* list, size_t size) { return 0; }
  static int khan_removexattr(const char* path, const char* name) { return 0; }
  static int khan_setvolname(const char* param) { return 0; }
  static int khan_exchange(const char* param1, const char* param2, unsigned long param3) {
    return 0;
  }

  static int khan_getxtimes(const char* param1, struct timespec* param2, struct timespec* param3) {
    return 0;
  }

  static int khan_setbkuptime(const char* param1, const struct timespec* param2) {
    return 0;
  }

  static int khan_setchgtime(const char* param1, const struct timespec* param2) {
    return 0;
  }

  static int khan_setcrtime(const char* param1, const struct timespec* param2) {
    return 0;
  }

  static int khan_chflags(const char* param1, uint32_t param2) {
    return 0;
  }

  static int khan_setattr_x(const char* param1, struct setattr_x* param2) {
    return 0;
  }

  static int khan_fsetattr_x(const char* param1, struct setattr_x* param2, struct fuse_file_info* param) {
    return 0;
  }

#endif

void process_stores(const char* store_filename) {
  cout << "Loading stores from " << store_filename << "." << endl;
  FILE* stores = fopen(store_filename, "r");
  if(!stores) {
    cout << "Could not open stores file." << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }
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
}

void move_to_root() {
  cout << "Attempting to open " << this_server << endl;
  if(opendir(this_server.c_str()) == NULL) {
    cout << "Could not open this server root." << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }
}

void clear_mount(const char* mount_dir) {
  string command = "fusermount -u ";
  #ifdef APPLE
    command = "umount ";
  #endif
  command.append(mount_dir);
  if(system(command.c_str()) < 0) {
    cout << "Could not unmount desired mount directory." << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }
}

void start_database() {
  #ifdef REDIS_FOUND
    database = new Redis();
    database->init();
  #endif
  cout << "database set" << endl;
}

void process_filetypes(string server) {
}

void load_system_metadata() {
  cout << "Loading system metadata." << endl;
  for(int i=0; i<servers.size(); i++){
    process_filetypes(servers.at(i));
  }
}

void load_file_metadata() {
  cout << "Loading metadata for every file." << endl;
  int file_count = 0;
  for(int i=0; i<servers.size(); i++) {
    glob_t files;
    string server = servers.at(i);
    string server_id = server_ids.at(i);
    glob((server+"/*.*").c_str(),0,NULL,&files);
    for(int j=0; j<files.gl_pathc; j++) {
      file_count += 1;
      string file = files.gl_pathv[j];
      string ext = strrchr(file.c_str(),'.')+1;
      string filename=strrchr(file.c_str(),'/')+1;
      cout << "file: " << file << endl;
      cout << "server: " << server << endl;
      cout << "server_id: " << server_id << endl;
      cout << "extension: " << ext << endl;
      cout << "filename: " << filename << endl << endl;
    }
  }
  cout << file_count << " files discoverd..." << endl;
}

void load_all_metadata(bool force) {
  if(!force) {
    string state = "";
    string loaded = database->get("state");
    if(loaded == "loaded") {
      cout << "Database already has metadata." << endl;
      cout << "Continuing..." << endl;
      return;
    } 
  }

  load_system_metadata();
  load_file_metadata(); 
  database->set("state", "loaded");
}

int main(int argc, char* argv[]) {
  // register operations
  khan_ops.getattr    = khan_get_attr;
  khan_ops.init       = khan_init;
  khan_ops.access     = khan_access;
  khan_ops.readlink   = khan_readlink;
  khan_ops.readdir    = khan_readdir;
  khan_ops.mknod      = khan_mknod;
  khan_ops.mkdir      = khan_mkdir;
  khan_ops.symlink    = khan_symlink;
  khan_ops.unlink     = khan_unlink;
  khan_ops.rmdir      = khan_rmdir;
  khan_ops.rename     = khan_rename;
  khan_ops.link       = khan_link;
  khan_ops.chmod      = khan_chmod;
  khan_ops.chown      = khan_chown;
  khan_ops.truncate   = khan_truncate;
  khan_ops.create     = khan_create;
  khan_ops.utimens    = khan_utimens;
  khan_ops.open       = khan_open;
  khan_ops.read       = khan_read;
  khan_ops.write      = khan_write;
  khan_ops.statfs     = khan_statfs;
  khan_ops.release    = khan_release;
  khan_ops.fsync      = khan_fsync;
  khan_ops.opendir    = khan_opendir;
  khan_ops.flush      = khan_flush;
  #ifdef APPLE
    khan_ops.setxattr    = khan_setxattr;
    khan_ops.getxattr    = khan_getxattr;
    khan_ops.listxattr   = khan_listxattr;
    khan_ops.removexattr = khan_removexattr;
    khan_ops.setvolname  = khan_setvolname;
    khan_ops.exchange    = khan_exchange;
    khan_ops.getxtimes   = khan_getxtimes;
    khan_ops.setbkuptime = khan_setbkuptime;
    khan_ops.setchgtime  = khan_setchgtime;
    khan_ops.setcrtime   = khan_setcrtime;
    khan_ops.chflags     = khan_chflags;
    khan_ops.setattr_x   = khan_setattr_x;
    khan_ops.fsetattr_x  = khan_fsetattr_x;
  #endif

  // process arguements
  if((argc < 2) || (argc > 4)) {
    cout << "Usage: ./khan <mount_dir> [stores.txt] [-d]" << endl;
    cout << "Aborting..." << endl;
    exit(1);
  }

  struct fuse_args khan_args = FUSE_ARGS_INIT(0, NULL);
  int j;
  const char* store_filename="stores.txt";
  for(j = 0; j < argc; j++) {
    if ((j == 2) && (argv[j][0] != '-')) {
      store_filename = argv[j];
    } else {
      fuse_opt_add_arg(&khan_args, argv[j]);
    }
  }

  // setup system
  process_stores(store_filename);
  move_to_root();
  clear_mount(argv[1]);
  start_database();
  load_all_metadata(false);

  return fuse_main(khan_args.argc, khan_args.argv, &khan_ops, NULL);
}

