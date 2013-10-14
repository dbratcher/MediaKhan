#include "voldemort.h"

int vold_last_id=1;
list<string> bootstrapUrls;
string storeName("test");
ClientConfig *gconfig;
SocketStoreClientFactory *gfactory;
auto_ptr<StoreClient> *gclient;
StoreClient *myclient;

bool voldemort_init() {
	//setup voldemort connection
	list<string> bootstrapUrls;
	bootstrapUrls.push_back(string("tcp://143.215.204.146:6666"));
	string storeName("test");
	gconfig=new ClientConfig();
	gconfig->setBootstrapUrls(&bootstrapUrls);
	gfactory=new SocketStoreClientFactory(*gconfig);
	gclient=new auto_ptr<StoreClient>(gfactory->getStoreClient(storeName));
	myclient=gclient->get();


/* Test Cases
	string key = voldemort_setval("null","col","val");
	cout<<"insert key returned "<<key<<endl;
	voldemort_setval(key,"col2","val2");
	voldemort_setval(key,"col3","val3");
	voldemort_setval(key,"col4","val4");

	string key2 = voldemort_setval("null","col5","val5");
	cout<<"insert key returned "<<key2<<endl;
	voldemort_setval(key2,"col3","val6");
	voldemort_setval(key2,"col4","val4");

	cout<<"keyvals for col3:"<<voldemort_getkey_cols("col3")<<endl;

	voldemort_getval("0","col2");
	voldemort_getval("0","col3");
	voldemort_getval("1","col3");
*/
	return 1;
}





string voldemort_getval(string file_id, string col){
	//log_msg("in getval");
	const VersionedValue* result2 = myclient->get(&file_id);
	if(result2) {
		//log_msg("it got a result...");
	} else {
		//log_msg("it did not...");
		return "null";
	}
	string output=*(result2->getValue());
	//cout <<output <<"\n";
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
		//cout<<"another="<<another<<endl;
	}
	//log_msg("get1 success");
	return another;
}




string voldemort_setval(string file_id, string col, string val){
	//cout<<"in vold_setval with file_id:"<<file_id<<" col:"<<col<<" val:"<<val<<endl;
	if(file_id.compare("null")==0){
		string out=voldemort_getval("vold_last_id","val");
		//cout<< "OUT="<<out<<endl;
		if(out.compare("null")==0){
			out="0";
		}
		//cout<< "OUT="<<out<<endl;
		string file_id=out;
		vold_last_id=0;
		vold_last_id=atoi(out.c_str());
		//cout << "OLD LAST ID="<<vold_last_id<<endl;
		vold_last_id++;//find non-local solution (other table?)
		ostringstream result;
		//cout << "NEW LAST ID="<<vold_last_id<<endl;
		result<<vold_last_id;
		//cout << "RESULT="<<result.str()<<endl;
		voldemort_remove_val("vold_last_id","val",out);
		voldemort_setval("vold_last_id","val",result.str());
		//cout << "running with fileid:" << file_id << endl; 
                voldemort_setval(file_id,col,val);
		//cout << "RETURNING FILE ID = " <<file_id << endl << endl;
                return file_id;
	}

	//cout<<"setting value for file_id:"<<file_id<<endl;

	//handle file_id key
	const VersionedValue* result3 = myclient->get(&file_id);
	string output;
	if(result3){
		output=*(result3->getValue());
	} else {
		//create this specific file id
		output="";
	}
	string store=output;
	//cout<<"got "<<output<<endl;
	string rest;
	if(store.find("~"+col+":")!=string::npos){//col already set
		string setval=voldemort_getval(file_id,col);
		int len=setval.length();
		//cout<<"col already set to "<<setval<<endl;
		if(setval.find(val)==string::npos){
			setval+=":"+val;
		}
		store.replace(store.find("~"+col+":")+2+col.length(),len,setval);
	} else {
		//cout<<"adding col - not already set"<<endl;
		store+="~"+col+":"+val;
	}
	myclient->put(&file_id,&store);
	//cout<<"put the string "<<store<<" at the key "<<file_id<<endl;


	//handle col key
	//cout<<"qeury for:"<<col<<endl;
	const VersionedValue* result4 = myclient->get(&col);
	output="";
	if(result4){
		output=*(result4->getValue());
	}
	store=output;
	//cout<<"returns:"<<store<<endl;
	rest="";
	if(store.find("~"+val+":")!=string::npos){//col already set
		//log_msg("handling col that already has val!");
		//cout<<"old_key:"<<store<<endl;
		rest=store.substr(store.find("~"+val+":"));
		rest=rest.substr(val.length()+2);
		size_t exact2=rest.find("~");
		if(exact2!=string::npos){
			rest=rest.substr(0,exact2);
		}
		//cout<<"original values of val:"<<rest<<endl;
		size_t orig_length=rest.length();
		if(rest.find(file_id)==string::npos){
			rest+=":"+file_id;
		}
		//cout<<"updated version:"<<rest<<endl;
		store=store.replace(store.find("~"+val+":")+2+val.length(),orig_length,rest);
		//cout<<"new key:"<<store<<endl;
	} else {
		store+="~"+val+":"+file_id;
	}
	myclient->put(&col,&store);
	//cout<<"put the string "<<store<<" at the key "<<col<<endl;
	return file_id;
}



string voldemort_getkey_values(string col){
	//cout<<"qeury for:"<<col<<endl;
	const VersionedValue* result4 = myclient->get(&col);
	string output="";
	if(result4){
		output=*(result4->getValue());
	}
	string ret_val="";
	//cout<<"found col with following:"<<output<<endl;
	stringstream ss(output);
	string val;
	while(getline(ss,val,'~')){
		//cout << "got val = " << val << endl;
		stringstream ss2(val);
		getline(ss2, val, ':');
		while(getline(ss2, val, ':')){
			ret_val+=val;
		}
	}
	//cout << "returning " << ret_val << endl;
	return ret_val;
}

string voldemort_getkey_cols(string col){
	//cout<<"qeury for:"<<col<<endl;
	const VersionedValue* result4 = myclient->get(&col);
	string output="";
	if(result4){
		output=*(result4->getValue());
	}
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


void voldemort_remove_val(string fileid, string col, string val){
	string replaced=voldemort_getval(fileid,col);
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
		myclient->put(&fileid,&replaced);

		//remove from col entry
		const VersionedValue* result4 = myclient->get(&col);
		string sout=*(result4->getValue());
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
		myclient->put(&col,&sout);
		result4=myclient->get(&col);
		string s2=*(result4->getValue());
		//cout << "did it take?:"<<s2<<endl;
	}
}




