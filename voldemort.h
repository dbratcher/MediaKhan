#include <sstream>
#include <voldemort/voldemort.h>

using namespace std;
using namespace Voldemort;

bool voldemort_init();
string voldemort_getkeys(string col, string val);
string voldemort_getval(string file_id, string col);
string voldemort_setval(string file_id, string col, string val);
string voldemort_getkey_values(string col);
string voldemort_getkey_cols(string col);
void voldemort_remove_val(string fileid, string col, string val);

