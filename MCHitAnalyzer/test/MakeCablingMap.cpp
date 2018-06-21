#include "fstream"
#include "iostream"
#include "utility"
#include "map"
using namespace std;
typedef int FEDID;
typedef int LAYER;
typedef int CHANNEL;
typedef pair<FEDID,CHANNEL> FEDINP;
typedef map<FEDINP,LAYER> MAP;
typedef pair<FEDINP,LAYER> MAP_E;

MAP MakeCablingMap(){
	string s;
	int FED,channel,layer;
	FEDINP *p;
	MAP cableMap;
	
	ifstream f("translation.dat");
	if(!f.is_open()){
		cout<<"Cabling Map making error"<<endl;
		return cableMap;
	}
	cout<<"Making FED cabling map"<<endl;
	//Get rid of the column name comment line
	getline(f,s);
	while(not f.eof()){
		//Get the name column
		f>>s;
		if(s.find("BPix")!=string::npos){
			layer=char(s[17])-48;
		}else{
			layer=5; //5 is fpix
		}
		//Move cursor to the FED column
		for(int i=0;i<7;i++)
			f>>s;
		//Read the FED ID
		f>>FED;
		f>>channel;
		p=new FEDINP(FED,channel);
		cableMap.insert(MAP_E(*p,layer));
		getline(f,s);
	}
	cout<<"Fed cabling map is done"<<endl;
	return cableMap;
}