#include "bdb.h"
#include "utils.h"

DB *dbp;
u_int32_t flags;

DBT dbt_from_str(string str) {
  DBT data;
  memset(&data, 0, sizeof(DBT)); 
  data.data = (void*)str.c_str();
  data.size = strlen(str.c_str())+1;
  return data;
}

void put(string key, string val) {
  DBT dbt_key = dbt_from_str(key);
  DBT dbt_val = dbt_from_str(val);
  dbp->put(dbp, NULL, &dbt_key, &dbt_val, 0); 
}

string get(string key) {
  DBT dbt_key = dbt_from_str(key);
  DBT dbt_data = dbt_from_str("");
  int ret_val = dbp->get(dbp, NULL, &dbt_key, &dbt_data, 0); 
  string ret = "";
  if (ret_val == DB_NOTFOUND) {
    //dbp->err(dbp, ret_val, "Data not found in the database");
  } else {
    ret = (char*) dbt_data.data;
  }
  return ret;
}

void print_db() {
  //cout << "Printing database..." << endl << flush;
  DBC *cursorp; 
  int ret;
  /* Initialize cursor */ 
  dbp->cursor(dbp, NULL, &cursorp, 0); 
  /* Initialize our DBTs. */ 
  DBT key = dbt_from_str("");
  DBT data = dbt_from_str("");
  /* Iterate over the database, retrieving each record in turn. */
  while (!(ret=cursorp->c_get(cursorp, &key, &data, DB_NEXT))) {
    printf("%s - %s\n", key.data, data.data);
  }; 
  if (ret != DB_NOTFOUND) { 
    fprintf(stderr,"Nothing found in the database.\n");
    exit(1);
  };
 
  /* Close cursor before exit */
  if (cursorp != NULL) 
    cursorp->c_close(cursorp); 
  //cout << "...finished printing database" << endl << endl << endl << flush;
}

bool bdb_init() {
  cout << "enter init" << endl << flush;
  int ret = db_create(&dbp, NULL, 0);
  flags = DB_CREATE;
  ret = dbp->open(dbp,		/* DB structure pointer */ 
			NULL,		/* Transaction pointer */ 
			"my_db.db",	/* On-disk file that holds the database. */ 
			NULL,		/* Optional logical database name */ 
			DB_HASH,	/* Database access method */ 
			flags,		/* Open flags */ 
			0);		
  cout << "exit init" << endl << flush;
  return true;
}

string bdb_getval(string file_id, string col) {
  ////cout << "enter getval" << endl << flush;
  string output = get(file_id);
  ////cout <<output <<"\n";
  size_t exact=output.find("~"+col+":");
  string another="null";
  if(exact!=string::npos){
    another=output.substr(exact);
    another=another.substr(2+col.length());
    size_t exact2=another.find("~");
    if(exact2==string::npos){
      exact2=another.find("}");
    }
    another=another.substr(0,exact2);
    ////cout<<"another="<<another<<endl;
  }
  ////cout << "exit getval" << endl << flush;
  return another;
}

string bdb_getkey_cols(string col) {
       //cout<<"qeury for:"<<col<<endl;
        string output = get(col);
        string ret_val="";
        //cout<<"found col with following:"<<output<<endl;
        stringstream ss(output);
        string val;
        while(getline(ss,val,'~')){
                //cout << "got val = " << val << endl;
                stringstream ss2(val);
                getline(ss2, val, ':');
                ret_val+=val+":";
        }   
        //cout << "returning " << ret_val << endl;
  return ret_val;
}

string bdb_setval(string file_id, string col, string val) { 
  if(file_id=="null") {
    string out=bdb_getval("bdb_last_id","val");
    ////cout<< "OUT="<<out<<endl;
    if(out.compare("null")==0){
      out="0";
    }
    ////cout<< "OUT="<<out<<endl;
    string file_id=out;
    int bdb_last_id=0;
    bdb_last_id=atoi(out.c_str());
    ////cout << "OLD LAST ID="<<bdb_last_id<<endl;
    bdb_last_id++;//find non-local solution (other table?)
    ostringstream result;
    ////cout << "NEW LAST ID="<<bdb_last_id<<endl;
    result<<bdb_last_id;
    ////cout << "RESULT="<<result.str()<<endl;
    bdb_remove_val("bdb_last_id","val",out);
    bdb_setval("bdb_last_id","val",result.str());
    bdb_setval(file_id,col,val);
    return file_id;
  }

        ////cout<<"setting value for file_id:"<<file_id<<endl;

        //handle file_id key
        string output = get(file_id);
        string store=output;
        ////cout<<"got "<<output<<endl;
        string rest;
        if(store.find("~"+col+":")!=string::npos){//col already set
                string setval=bdb_getval(file_id,col);
                int len=setval.length();
                ////cout<<"col already set to "<<setval<<endl;
                if(setval.find(val)==string::npos){
                        setval+=":"+val;
                }   
                store.replace(store.find("~"+col+":")+2+col.length(),len,setval);
        } else {
                ////cout<<"adding col - not already set"<<endl;
                store+="~"+col+":"+val;
        }   
        put(file_id,store);
        ////cout<<"put the string "<<store<<" at the key "<<file_id<<endl;


        //handle col key
        ////cout<<"qeury for:"<<col<<endl;
        output = get(col);
        store=output;
        ////cout<<"returns:"<<store<<endl;
        rest="";
        if(store.find("~"+val+":")!=string::npos){//col already set
                //log_msg("handling col that already has val!");
                ////cout<<"old_key:"<<store<<endl;
                rest=store.substr(store.find("~"+val+":"));
                rest=rest.substr(val.length()+2);
                size_t exact2=rest.find("~");
                if(exact2!=string::npos){
                        rest=rest.substr(0,exact2);
                }   
                ////cout<<"original values of val:"<<rest<<endl;
                size_t orig_length=rest.length();
                if(rest.find(file_id)==string::npos){
                        rest+=":"+file_id;
                }   
                ////cout<<"updated version:"<<rest<<endl;
                store=store.replace(store.find("~"+val+":")+2+val.length(),orig_length,rest);
                ////cout<<"new key:"<<store<<endl;
        } else {
                store+="~"+val+":"+file_id;
       }
  put(col, store);
  //print_db();
  ////cout<<"put the string "<<store<<" at the key "<<col<<endl;
  return file_id;
}

void bdb_remove_val(string fileid, string col, string val){
        string replaced=bdb_getval(fileid,col);
        //cout << "file:"<<fileid<<" col:"<<col<<" val:"<<val<<endl;
        //cout << "replaced :"<<replaced<<endl;
        if(replaced.find(val)!=string::npos){
                //cout<<"its here"<<endl;

                //remove from file entry
                replaced.replace(replaced.find(val),val.length()+1,"");
                if(replaced.length()>0 && replaced.at(0)==':'){
                        replaced="~"+col+replaced;
                } else {
                        replaced="~"+col+":"+replaced;
                }   
                if((replaced.length()-1)>0){
                        //cout<<replaced.length()<<endl;
                        if(replaced.at(replaced.length()-1)==':'){
                                replaced.erase(replaced.length()-1);
                        }   
                }   
                //cout<<"new replaced:"<<replaced<<endl;
                put(fileid,replaced);

                //remove from col entry
                string sout = get(col);
                //cout <<"col side:"<<sout<<endl;
                int len=sout.find("~"+val+":");
                int len2=sout.find("~",len+1);
                //cout <<"len1:"<<len<<" len2:"<<len2<<" len2-len1:"<<(len2-len)<<endl;
                if(len2>0){
                        sout.replace(len,len2-len,"");
                } else if(len>0) {
                        sout.replace(len,sout.length(),"");
                } else {
                        sout="";
                }
                //cout <<"new col:"<<sout<<endl;
                put(col,sout);
                string s2 = get(col);
                //cout << "did it take?:" << s2 << endl;
  }
}

