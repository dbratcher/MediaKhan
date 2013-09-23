#include "bdb.h"
#include "utils.h"

DB *dbp;
u_int32_t flags;

bool bdb_init() {
  int ret = db_create(&dbp, NULL, 0);
  flags = DB_CREATE;
  ret = dbp->open(dbp,		/* DB structure pointer */ 
			NULL,		/* Transaction pointer */ 
			"my_db.db",	/* On-disk file that holds the database. */ 
			NULL,		/* Optional logical database name */ 
			DB_HASH,	/* Database access method */ 
			flags,		/* Open flags */ 
			0);		
  return true;
}

string bdb_getval(string file_id, string col) {
    
  return "";
}

string bdb_getkey_cols(string col) {
  return "";
}

string bdb_setval(string file_id, string col, string val) {  
  return "";
}

void bdb_remove_val(string fileid, string col, string val){
}

