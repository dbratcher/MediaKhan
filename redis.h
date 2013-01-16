#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <hiredis/hiredis.h>
#include "log.h"

using namespace std;

bool redis_init();
string redis_getval(string file_id, string col);
string redis_getkeys(string col, string val);
string redis_getkey_values(string col);
string redis_getkey_cols(string col);
string redis_setval(string file_id, string col, string val);
void redis_remove_val(string fileid, string col, string val);

