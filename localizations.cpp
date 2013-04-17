#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdio.h>
#include "localizations.h"

extern string servers[5];
extern int num_servers;

string random_location(char* filename) {
  return servers[rand()%num_servers];	
}

string genre_location(char* filename) {
  //get genre of file
  //for each server
    //for each file
      //if genre = file genre
        //genre_count for server ++
  //place on highest count server
}

string knn_location(char* filename) {
  //run knn
  //place on best fit server
}

string get_location(char* filename) {
  char* type = strdup("random");
  if(strcmp(type,"random")==0) {
    return random_location(filename);
  } else if(strcmp(type, "genre")==0) {
    return genre_location(filename);
  } else if(strcmp(type, "knn")==0) {
    return knn_location(filename);
  } else {
    fprintf(stderr, "invalid localizing algorithm\n");
    exit(1);
  }
  return NULL;
}
