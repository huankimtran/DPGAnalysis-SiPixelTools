// -*- C++ -*-
//
// Package:    DPGAnalysis-SiPixelTools/MCHitAnalyzer
// Class:      MCHitAnalyzer
//
/**\class MCHitAnalyzer MCHitAnalyzer.cc DPGAnalysis-SiPixelTools/MCHitAnalyzer/plugins/MCHitAnalyzer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
 */
//
// Original Author:  HUAN K TRAN
//         Created:  Mon, 04 Jun 2018 19:06:56 GMT
//
//

//==========THIS FILE IS BASED ON =======================
//https://cmssdt.cern.ch/lxr/source/EventFilter/SiPixelRawToDigi/plugins/SiPixelRawToDigi.cc?v=CMSSW_10_2_X_2018-06-03-2300#0232
// system include files
#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESTransientHandle.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"

#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

#include "EventFilter/SiPixelRawToDigi/interface/PixelDataFormatter.h"
#include "DataFormats/Common/interface/DetSetVector.h"
#include "CalibFormats/SiPixelObjects/interface/PixelModuleName.h"
#include "CondFormats/DataRecord/interface/SiPixelFedCablingMapRcd.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelFedCablingMap.h"

//// To use root file
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"


//// For ROOT
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TF1.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TStyle.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <map>

//Configure inclusion from MyDecode.h 
#define PHASE1
#include "MyDecode.h"
#include "MyDecodeGlobalVariables.h"
#include "Pixel.h"
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
		//Read the corresponding channel
		f>>channel;
		//create the link
		p=new FEDINP(FED,channel);
		cableMap.insert(MAP_E(*p,layer));
		//Move the cursor to the newline
		getline(f,s);
	}
	cout<<"Fed cabling map is done"<<endl;
	return cableMap;
}
using reco::TrackCollection;


class MCHitAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
public:
	explicit MCHitAnalyzer(const edm::ParameterSet&);
	~MCHitAnalyzer();
	Int_t getLayer(Int_t fedID,Int_t channel);
	static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
	virtual void beginJob() override;
	virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
	virtual void endJob() override;
	
	edm::EDGetTokenT<FEDRawDataCollection> rawData;
	std::vector<unsigned int> fedIds;
	std::unique_ptr<SiPixelFedCablingTree> cabling_;
	MyDecode decode;
	TTree *tree, *hFedTree,*zeroTree;
	Pixel p;
	edm::Service<TFileService> fs;
	//First is number of events that the fed got data, second is number of hit
	vector<pair<int,int>> HitFedCount;
	MAP fedMap;
	int numbEvent;
};

MCHitAnalyzer::MCHitAnalyzer(const edm::ParameterSet& iConfig)
{
	//Get out the name of the object need to be filtered in root files
	string label = iConfig.getUntrackedParameter<std::string>("InputLabel");
	//Get the raw data collected from the FED throught the object with the name specified
	//in the label variable
	rawData = consumes<FEDRawDataCollection>(label);
	//Initialize the hit counter per FED
	for (unsigned int i=0;i<n_of_FEDs;i++)
		HitFedCount.push_back(pair<int,int>(0,0));
	//Creating new structure
	cout<<"MCHitAnalyzer object is created"<<endl;
	//Tree to store full decoded raw data
	tree= new TTree("DecodedData","DecodedData");
	tree->Branch("Data",&p,"_eventID/I:_fedID/I:_layer/I:_channel/I:_ROC/I:_row/I:_col/I:_adc/I");
	//Tree to store decoded raw data of the FED with the highest number of hit
	hFedTree= new TTree("HighFedData","HighFedData");
	hFedTree->Branch("Data",&p,"_eventID/I:_fedID/I:_layer/I:_channel/I:_ROC/I:_row/I:_col/I:_adc/I");
	//Tree to store empty event fed
	zeroTree= new TTree("ZeroData","ZeroData");
	zeroTree->Branch("Data",&p,"_eventID/I:_fedID/I:_layer/I:_channel/I:_ROC/I:_row/I:_col/I:_adc/I");
	//Create the cabling map that map fedid and channel to the layer it is connected to
	//Make sure you have the translation.dat file in the folder you run cmsRun
	fedMap=MakeCablingMap();
	//Configure the pixel class with the given map
	numbEvent=0;
}


MCHitAnalyzer::~MCHitAnalyzer()
{

	// do anything here that needs to be done at desctruction time
	// (e.g. close files, deallocate resources etc.)
	cout<<"MCHitAnalyzer object is destroyed"<<endl;

}


//
// member functions
//

void
MCHitAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	//Counting number of events
	numbEvent++;
	cout<<"MCHitAnalyzer analyze is being run"<<endl;
	edm::Handle<FEDRawDataCollection> buffers;	
	//Get raw data
	iEvent.getByToken(rawData , buffers);
	std::pair<int,int> fedIds(1200,1338);
	//Getting configuration details
	int run       = iEvent.id().run();
	int event     = iEvent.id().event();
	int lumiBlock = iEvent.luminosityBlock();
	int bx        = iEvent.bunchCrossing(); // CMSSW bx
	//For plotting row and col
	typedef uint32_t Word32;
	typedef uint64_t Word64;
	int status=0;
	int stat1=-1, stat2=-1;
	int eventId = -1;
	//bool wrongBX=false;
	const unsigned int plsmsk = 0xff;   // pulse height                                                                      
	const unsigned int pxlmsk = 0xff00; // pixel index                                                                       
	const unsigned int dclmsk = 0x1f0000;
	for (int fedId = fedIds.first; fedId < (fedIds.second + 1); fedId++) {
		//Get the raw data associating with the given fedId
		const FEDRawData& rawData = buffers->FEDData( fedId ); 
		//FED with no data should not be checked
		//Add zeror event
		if(rawData.size()==0){
			p.fillEmpty(event,fedId);
			zeroTree->Fill();
			continue;
		}
		//Increase the number of events this FED has hit
		HitFedCount[fedId-FIRST_FED_ID].first++;
		//Getting the header of the raw data
		const Word64* header = reinterpret_cast<const Word64* >(rawData.data()); 
		
		int nWords = rawData.size()/sizeof(Word64);
		unsigned int bxid = 0;
		//Extracting the eventId and the header 
		eventId =MyDecode::header(*header, fedId, printHeaders, bxid);
		//Nik's code
		if(bx != int(bxid) ) { 
			if(printErrors && printBX && !phase1) 
				cout<<" Inconsistent BX: for event "<<event<<" (fed-header event "<<eventId<<") for LS "<<lumiBlock
				<<" for run "<<run<<" for bx "<<bx<<" fed bx "<<bxid<<endl;
		}
		if(bx<0) bx=bxid;
		const Word64* trailer = reinterpret_cast<const Word64* >(rawData.data())+(nWords-1);
		//Getting the trailer (the end of the raw data)
		status = MyDecode::trailer(*trailer,fedId, printHeaders);
		int fedChannel = 0;
		//Looping through each byte of the raw data
		for (const Word64* word = header+1; word != trailer; word++) {
			static const Word64 WORD32_mask  = 0xffffffff;
			for(int ipart=0;ipart<2;++ipart) {
				Word32 w = 0;
				if(ipart==0) {
					w = *word&WORD32_mask;  // 1st word
				} else if(ipart==1) {
					w = *word>>32&WORD32_mask;  // 2nd word
				}
				//Decode the 32-bit memory to get the information of the pixel
				status =decode.data(w,fedChannel, fedId, stat1, stat2, printData);
				//Catch error from decoding
				if(status<0) {  
					// error word
					if(printErrors) cout<<"    Bad stats for FED "<<fedId<<" Event "<<eventId<<"/"<<(eventId%256)
							    								  <<" chan "<<fedChannel<<" status "<<status<<endl;
					status=abs(status);
					// 2 - wrong channel
					// 3 - wrong pix or dcol 
					// 4 - wrong roc
					// 5 - pix=0
					// 6 - double pixel
					// 10 - timeout ()
					// 11 - ene ()
					// 12 - mum pf rocs error ()
					// 13 - fsm ()
					// 14 - overflow ()
					// 15 - trailer ()
					// 16 - fifo  (30)
					// 17 - reset/resync NOT INCLUDED YET

					switch(status) {

					case(10) : { // Timeout
						cout<<"Timeout"<<endl;
						break; } 

					case(14) : {  
						cout<<"Overflow"<<endl;
						break; }

					case(11) : {  
						cout<<"ENE"<<endl;
						break; }

					case(16) : { //FIFO
						cout<<"FIFO"<<endl;
						break; }

					case(12) : {  // NOR
						cout<<"NOR"<<endl;
						break; }

					case(15) : {  // TBM Trailer
						cout<<"TBM Trailer"<<endl;
						break; }

					case(13) : {  // FSM
						cout<<"FSM"<<endl;
						break; }

					case(3) : {  //  inv. pix-dcol
						cout<<"Invalid pix-dcol"<<endl;
						break; }

					case(4) : {  // inv roc
						cout<<"Invalid ROC"<<endl;
						break; }

					case(5) : {  // pix=0
						cout<<"pix=0"<<endl;
						break; }

					case(6) : {  // double pix
						cout<<"Double pix"<<endl;
						break; }

					case(1) : {  // unknown
						cout<<"Unknown"<<endl;
						break; }

					} //end switch
				}//end if(status<0)
				else{
					p.fill(eventId,fedId,this->getLayer(fedId,decode.get_channel()),decode);
					tree->Fill();
					HitFedCount[fedId-FIRST_FED_ID].second++;
				}
			}//end one loop
		}//end two word loop
	}
}	


// ------------ method called once each job just before starting event loop  ------------
void
MCHitAnalyzer::beginJob()
{
	cout<<"MCHitAnalyzer beginJob is being called"<<endl;
}

// ------------ method called once each job just after ending the event loop  ------------
void
MCHitAnalyzer::endJob()
{	
	cout<<"MCHitAnalyzer endJob is being called"<<endl;
	float max=0;
	int fedMax=-1;
	for(unsigned int i=0;i<HitFedCount.size();i++){
		if(!HitFedCount[i].first) continue;
		if((HitFedCount[i].second/numbEvent)>max){
			fedMax=i;
			max=HitFedCount[i].second/numbEvent;
		}
	}
	fedMax+=1200;
	cout<<"Total number of event: "<<numbEvent<<endl;
	cout<<"Total number of zero event: "<<numbEvent-HitFedCount[fedMax-1200].first<<endl;
	cout<<"FED ID with the highest average hit:"<<fedMax<<endl;
	cout<<"The average hit of that FED is:"<<max<<endl;
	cout<<"Filtering out the data of the FED with the highest number of hit"<<endl;
	//Filtering out the data of the fed with the highest hit
	hFedTree->SetBranchAddress("Data",&p);
	for(unsigned int i=0;i<tree->GetEntries();i++){
		tree->GetEntry(i);
		if(p._fedID==fedMax)
			hFedTree->Fill();
	}
	//Save only the latest version of the trees
	tree->Write("",TObject::kOverwrite);
	hFedTree->Write("",TObject::kOverwrite);
	zeroTree->Write("",TObject::kOverwrite);
	//Close the file so that when the deconstructor of this class is called, the data in the memory won't be wr
	fs->file().Close();
}
//Get layer from fedID and channel
Int_t MCHitAnalyzer::getLayer(Int_t fedID,Int_t channel){
	MAP::iterator i=fedMap.find(FEDINP(fedID,channel));
	if(i==fedMap.end()){
		cout<<"FED "<<fedID<<" channel "<<channel<<". No layer found!"<<endl;
		//Layer not found
		return 0;
	}
		//return the layer if found
	return i->second;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
MCHitAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
	//The following says we do not know what parameters are allowed so do no validation
	// Please change this to state exactly what you do use, even if it is no parameters
	edm::ParameterSetDescription desc;
	desc.setUnknown();
	descriptions.addDefault(desc);

	//Specify that only 'tracks' is allowed
	//To use, remove the default given above and uncomment below
	//ParameterSetDescription desc;
	//desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
	//descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(MCHitAnalyzer);
