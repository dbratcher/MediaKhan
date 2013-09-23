#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <db_cxx.h>
#include "log.h"

using namespace std;

bool bdb_init();
string bdb_getval(string file_id, string col);
string bdb_getkey_cols(string col);
string bdb_setval(string file_id, string col, string val);
void bdb_remove_val(string fileid, string col, string val);

