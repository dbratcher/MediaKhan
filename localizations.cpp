#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector>
#include "database.h"
#include "localizations.h"

extern vector<string> servers;
extern vector<string> server_ids;

vector<string> get_all_files(string ext) {
  cout << " GET ALL FILES " << endl;
  vector<string> files;
  string mp3s = database_getvals("all_"+ext+"s");
  stringstream ss(mp3s);
  string filename;
  while(getline(ss, filename, ':')) {
    if(filename.compare("")!=0) {
      string fileid = database_getval("name",filename);
      files.push_back(fileid);
      cout << "added id:"<<fileid<<endl;
    }
  }
  cout << " DONE " << endl;
  return files;
}

string random_location(string fileid) {
  return servers.at(rand()%servers.size());	
}

string genre_location(string fileid) {
  //get genre of file
  string genre = database_getval(fileid, "genre");
  cout << "looking for "<<genre<<endl;
  vector<int> server_counts(servers.size());
  //for each file
  vector<string> files = get_all_files("mp3");
  for(int i=0; i<files.size(); i++) {
    //if genre = file genre
    if(fileid.compare(files.at(i))) {
      string file_genre = database_getval(files.at(i), "genre");
      cout << "file genre "<<file_genre<<endl;
      if(file_genre.compare(genre)==0) {
        cout << "match "<<endl;
        string server = database_getval(files.at(i), "server");
        for(int j=0; j<servers.size(); j++) {
          if(servers.at(j).compare(server)==0) {
            server_counts.at(j) = server_counts.at(j) + 1;
            cout << "new count "<<server_counts.at(j) << endl;
          }
        }
      }
    }
  }
  //place on highest count server
  int max_num = 0;
  string max ="";
  for(int j=0; j<servers.size(); j++) {
    cout << "looking at server " << j << " with count " << server_counts.at(j) << endl;
    if(server_counts.at(j)>max_num) {
      max_num = server_counts.at(j);
      max = servers.at(j);
    }
  }
  return max;
}

vector<string> get_all_attr_vals(string attr) {
  vector<string> vals;
  string mp3s = database_getvals(attr);
  stringstream ss(mp3s);
  string val;
  while(getline(ss, val, ':')) {
    if(val.compare("")!=0) {
      vals.push_back(val);
    }
  }
  return vals;
}


int get_attr_numeric_val(string attr, string val) {
  //get all attr vals
  vector<string> vals = get_all_attr_vals(attr);
  //sort
  sort(vals.begin(), vals.end());
  //return index of val
  int index = 0;
  for(int i =0; i<vals.size(); i++) {
    if(vals.at(i).compare(val)==0) {
      index = i;
    }
  }
  return index;
}

string knn_location(string fileid) {
  //keep track of min distance file
  float min_dist = 1000000;
  string min_file = fileid;
  //for each file
  string ext = database_getval(fileid, "ext");
  vector<string> files = get_all_files(ext);
  for(int i=0; i<files.size(); i++) {
    if(files.at(i).compare(fileid)!=0) {
      vector<string> attrs = get_all_attr_vals(fileid);
      int sum = 0;
      //for each file attr
      for(int j =0; j<attrs.size(); j++) {
        // get files attr numeric vals
        string val = database_getval(files.at(i), attrs.at(j));
        int fv = get_attr_numeric_val(attrs.at(j), val);
        val = database_getval(fileid, attrs.at(j));
        int iv = get_attr_numeric_val(attrs.at(j), val);
        //subtract
        int diff = fv - iv;
        //sqr then add to sum
        sum += pow(diff, 2);
      }
      //sqrt sum = distance
      float dist = sqrt(sum);
      //keep track of min dist file
      if(dist < min_dist) {
        min_dist = dist;
        min_file = files.at(i);
      }
    }
  }
  
  cout << "KNN SELECTED A LOCATION "<<endl;
  cout << min_file << endl;
  //get closest file server
  string min_file_server = database_getval(min_file, "server");
  return min_file_server;
}

string get_location(string fileid) {
  string type = "knn";
  if(type.compare("random")==0) {
    return random_location(fileid);
  } else if(type.compare("genre")==0) {
    return genre_location(fileid);
  } else if(type.compare("knn")==0) {
    return knn_location(fileid);
  } else {
    fprintf(stderr, "invalid localizing algorithm\n");
    exit(1);
  }
  return NULL;
}

void usage_localize() {
  //for all files
  vector<string> files = get_all_files("mp3");
  for(int i=0; i<files.size(); i++) {
    //for each server
    int max_usage=0;
    int max_server=0;
    cout << "looking at file " << i << endl;
    for(int j=0; j<server_ids.size(); j++) {
      cout << "looking at server " << j << endl; 
      //track max usage count server
      string res=database_getval(files.at(i), server_ids.at(j));
      int usage = atoi(res.c_str());
      cout << "this server usage " << usage << endl;
      if(usage>max_usage) {
        cout << "new max " << usage << endl;
        max_usage = usage;
        max_server = j;
      }
    }
    //move to server
    string filename = database_getval(files.at(i), "name");
    string current = database_getval(files.at(i), "server");
    string cur_path = current + "/" + filename;
    string dst_path = servers.at(max_server) + "/" + filename;
    if(cur_path.compare(dst_path)) {
      cout << "moving to server " << max_server << endl;
      string command="mv " + cur_path + " " + dst_path;
      FILE* stream=popen(command.c_str(),"r");
      pclose(stream);
      //rename(cur_path.c_str(), dst_path.c_str());
      database_setval(files.at(i), "server", dst_path);
    }
  }
  cout << "done usage localize " << endl;
}
