#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include <vector>
#include "database.h"
#include "localizations.h"

extern vector<string> servers;

vector<string> get_all_files() {
  cout << " GET ALL FILES " << endl;
  vector<string> files;
  string mp3s = database_getvals("all_mp3s");
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
  vector<string> files = get_all_files();
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

string knn_location(string fileid) {
  //run knn
  //place on best fit server
}

string get_location(string fileid) {
  string type = "genre";
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
