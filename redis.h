#include <iostream>
#include <string>
#include <hiredis/hiredis.h>

using namespace std;

bool redis_init();
string redis_getval(string file_id, string col);
void redis_remove_val(string fileid, string col, string val);

