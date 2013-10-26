#include <stdio.h>
#include <glob.h>
#include <string>
#include <iostream>
#include <dirent.h>

using namespace std;

int main ()
{
  //cout << "here1" << endl;
  int result;
  int j=0;
  DIR *Dir;
  struct dirent *DirEntry;
  //cout << "here2" << endl;
  Dir = opendir("./test/location/laptop/");
  //cout << "here3" << endl;
  if(!Dir) {
    cout << "Fail\n";
  }
  //cout << "here4" << endl;
  while((DirEntry = readdir(Dir)) != NULL) {
    //cout << "here5" << endl;
    string file = DirEntry->d_name;
    //cout << "here6" << endl;
    //cout << file;
    //cout << "here7" << endl;
    string filename = "./test/tags/favs/" + file;
    //cout << "here8" << endl;
    file = "./test/location/laptop/" + file;
    //cout << file << " -> " << filename << endl;
    if(j%10==0) {
      cout << "file " << j << endl << endl;
    }
    j+=1;
    result= rename( file.c_str() ,  filename.c_str());
  }
  return 0;
}
