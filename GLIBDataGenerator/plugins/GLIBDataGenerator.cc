// -*- C++ -*-
//
// Package:    DPGAnalysis-SiPixelTools/GLIBDataGenerator
// Class:      GLIBDataGenerator
//
/**\class GLIBDataGenerator GLIBDataGenerator.cc DPGAnalysis-SiPixelTools/GLIBDataGenerator/plugins/GLIBDataGenerator.cc

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
//#include "CalibFormats/SiPixelObjects/src/PixelNameTranslation.cc"
//// To use root file
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

//
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
#include "PixelEncoder.h"
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
// Include the helper decoding class (how did I mange to avoid linking conflicts?)
// /////////////////////////////////////////////////////////////////////////////
#define PHASE1
namespace {
bool printErrors  = false;
bool printData    = false; 
bool printHeaders = false; 
const bool printBX = false;
const bool CHECK_PIXELS = true;
const bool PRINT_BASELINE = false;
// to store the previous pixel 
int fed0 = -1, chan0 = -1, roc0 = -1, dcol0 = -1, pix0 =-1, count0=-1;
int countDecodeErrors1=0, countDecodeErrors2=0;
#ifdef PHASE1
const int n_of_FEDs = 139; //kme 56
const int n_of_Channels = 48; //kme 96?? -- maybe n_of_Channels = 48
const int fedIdBpixMax = 1293; //kme 40
const int fedIdBpixMax_sup = 93;
const bool phase1 = true;
#define NUMBER_FED 139
#define MAX_COLUMN_BARREL 300
#define MAX_ROW_BARREL 300
#define MAX_COLUMN_CAP 300
#define MAX_ROW_CAP 300
#define FIRST_FED_ID 1200
#define LAST_FED_ID 1338
#else
const int n_of_FEDs = 41;
const int n_of_Channels = 36;
const int fedIdBpixMax = 32;
const bool phase1 = false;
#endif
}

//Decoding helper functions
class MyDecode {
public:
	MyDecode() {}
	~MyDecode() {}
	static int error(int error,int & fedChannel, int fed, int & stat1, int & stat2, bool print=false);
	int data(int error, int & fedChannel, int fed, int & stat1, int & stat2, bool print=true);
	static int header(unsigned long long word64, int fed, bool print, unsigned int & bx);
	static int trailer(unsigned long long word64, int fed, bool print);
	static int convertToCol(int dcol,int pix);
	static int convertToRow(int pix);
	static int checkLayerLink(int fed, int chan);
	int get_adc(void) {return adc_;}
	int get_roc(void) {return roc_;}
	int get_dcol(void) {return dcol_;}
	int get_pix(void) {return pix_;}
	int get_col(void) {return col_;}
	int get_row(void) {return row_;}
	int get_channel(void) {return channel_;}
private:
	int channel_, adc_, roc_, dcol_, pix_, col_, row_;
};
//Decoded data structure
class Pixel{
public:
	Pixel(Int_t eventID,Int_t fedID,Int_t layer,Int_t channel,Int_t ROC,Int_t row,Int_t col,Int_t adc){
		_eventID=eventID;
		_fedID=fedID;
		_channel=channel;
		_ROC=ROC;
		_row=row;
		_col=col;
		_adc=adc;
		_layer=layer;
	}
	Pixel(){};
	void fill(Int_t eventID, Int_t fedID,Int_t layer,MyDecode d){
		_eventID=eventID;
		_fedID=fedID;
		_channel=d.get_channel();
		_ROC=d.get_roc();
		_row=d.get_row();
		_col=d.get_col();
		_adc=d.get_adc();
		_layer=layer;
	}
	Int_t _eventID,_fedID,_layer,_channel,_ROC,_row,_col,_adc;
};

int MyDecode::checkLayerLink(int fed, int chan) {
	int layer = 0;
	if(fed<0 || fed>31) return layer;  // return 0 for invalid of fpix

	if( chan>24) {   // layer 3

		if(fed==0 || fed==8 )  {  // Type A

			if(chan==28 ||chan==34 ||chan==35 ||chan==36 ) layer=13;  // 1/2 module
			else layer = 3;

		} else if(fed==23 || fed==31 )  {  // Type A

			if(chan==27 ||chan==31 ||chan==32 ||chan==33 ) layer=13;  // 1/2 module
			else layer = 3;

		} else if(fed==7 || fed==15 || fed==16 || fed==24 )  { // Type D

			if(chan==25 ||chan==26 ||chan==29 ||chan==30 ) layer=13;
			else layer = 3;

		}  else { layer = 3;}

		return layer; // layer 3

	} else if( (chan>=13 && chan<=19) || chan==24 ) {

		return 2; //layer 2

	} else {

		if(fed==0 || fed==8 || fed==16 || fed==24 )  {  // Type A   WRONG AFTER FIBER SWAP

			if(chan==5 ||chan==6 ||chan==22 ||chan==23 ) layer=12; // 1/2 module 
			else if(chan==4 ||chan==10 ||chan==11 ||chan==12 ) layer=11; // 1/2 module 
			else layer = 1;

		} else if(fed==7 || fed==15 || fed==23 || fed==31 )  { // Type D

			if(chan==1 ||chan==2 ||chan==20 ||chan==21 ) layer=12; // 1/2
			else if(chan==3 ||chan==7 ||chan==8 ||chan==9 ) layer=11; // 1/2 
			else layer = 1;

		} else if(
				fed==1  || fed==2  || fed==3  ||   
				fed==9  || fed==10 || fed==11 ||   
				fed==17 || fed==18 || fed==19 ||   
				fed==25 || fed==26 || fed==27  )  { // Type B

			if( (chan>=4 && chan<=6) || (chan>=10 && chan<=12) || (chan>=22 && chan<=23) ) layer=2;
			else layer = 1;

		} else if(
				fed==4  || fed==5  || fed==6  ||   
				fed==12 || fed==13 || fed==14 ||   
				fed==20 || fed==21 || fed==22 ||   
				fed==28 || fed==29 || fed==30  )  { // Type C

			if( (chan>=1 && chan<=3) || (chan>=7 && chan<=9) || (chan>=20 && chan<=21) ) layer=2;
			else layer = 1;

		} else {
			cout<<"unknown fed "<<fed<<endl;
		} // if fed 

		return layer;

	}  // if chan
}

int MyDecode::convertToCol(int dcol, int pix) {
	// First find if we are in the first or 2nd col of a dcol.
	int colEvenOdd = pix%2;  // module(2), 0-1st sol, 1-2nd col.
	// Transform
	return (dcol * 2 + colEvenOdd); // col address, starts from 0

}
int MyDecode::convertToRow(int pix) {
	return abs( int(pix/2) - 80); // row addres, starts from 0
}

int MyDecode::header(unsigned long long word64, int fed, bool print, unsigned int & bx) {
	int fed_id=(word64>>8)&0xfff;
	int event_id=(word64>>32)&0xffffff;
	bx =(word64>>20)&0xfff;
	//   if(bx!=101) {
	//     cout<<" Header "<<" for FED "
	// 	<<fed_id<<" event "<<event_id<<" bx "<<bx<<endl;
	//     int dummy=0;
	//     cout<<" : ";
	//     cin>>dummy;
	//   }
	if(print) cout<<"Header "<<" for FED "
			<<fed_id<<" event "<<event_id<<" bx "<<bx<<endl;
	fed0=-1; // reset the previous hit fed id
	return event_id;
}
//
int MyDecode::trailer(unsigned long long word64, int fed, bool print) {
	int slinkLength = int( (word64>>32) & 0xffffff );
	int crc         = int( (word64&0xffff0000)>>16 );
	int tts         = int( (word64&0xf0)>>4);
	int slinkError  = int( (word64&0xf00)>>8);
	if(print) cout<<"Trailer "<<" len "<<slinkLength
			<<" tts "<<tts<<" error "<<slinkError<<" crc "<<hex<<crc<<dec<<endl;
	return slinkLength;
}
//
// Decode error FIFO
// Works for both, the error FIFO and the SLink error words. d.k. 25/04/07
int MyDecode::error(int word, int & fedChannel, int fed, int & stat1, int & stat2, bool print) {
	int status = -1;
	print = print || printErrors;

	const unsigned int  errorMask      = 0x3e00000;
	const unsigned int  dummyMask      = 0x03600000;
	const unsigned int  gapMask        = 0x03400000;
	const unsigned int  timeOut        = 0x3a00000;
	//const unsigned int  timeOut2       = 0x3b00000;
	const unsigned int  eventNumError  = 0x3e00000;
	const unsigned int  trailError     = 0x3c00000;
	const unsigned int  fifoError      = 0x3800000;

	//  const unsigned int  timeOutChannelMask = 0x1f;  // channel mask for timeouts
	//const unsigned int  eventNumMask = 0x1fe000; // event number mask
	const unsigned int  channelMask = 0xfc000000; // channel num mask
	const unsigned int  tbmEventMask = 0xff;    // tbm event num mask
	const unsigned int  overflowMask = 0x100;   // data overflow
	const unsigned int  tbmStatusMask = 0xff;   //TBM trailer info
	const unsigned int  BlkNumMask = 0x700;   //pointer to error fifo #
	const unsigned int  FsmErrMask = 0x600;   //pointer to FSM errors
	const unsigned int  RocErrMask = 0x800;   //pointer to #Roc errors
	const unsigned int  ChnFifMask = 0x1f;   //channel mask for fifo error
	const unsigned int  Fif2NFMask = 0x40;   //mask for fifo2 NF
	const unsigned int  TrigNFMask = 0x80;   //mask for trigger fifo NF

	//masks for diagnostic words from piggy pilot FED
	const unsigned int  NORMask = 0x1f0000;  //number of ROCs 
	const unsigned int  AutoResetMask = 0x80; //autoreset detected
	const unsigned int  PKAMResetMask = 0x40; //PKAM reset detected
	const unsigned int  TBMStackCntMask =0x1f; //TBM Stack Count
	const unsigned int  TimeStampMask = 0xFFFFF; //Time Stamp for first ROC header & TBM trailer


	const int offsets[8] = {0,4,9,13,18,22,27,31};
	unsigned int channel = 0;

	//cout<<"error word "<<hex<<word<<dec<<endl;

	/* ----------------------------------------------------------------------

  if( (word&errorMask) == dummyMask ) { // DUMMY WORD
    //cout<<" Dummy word";
    return 0;

  } else if( (word&errorMask) == gapMask ) { // GAP WORD
    //cout<<" Gap word";
    return 0;

    ---------------------------------------------------------------------- */

	if( (word&errorMask) == dummyMask ) { // DUMMY WORD
		//cout<<"error word "<<hex<<word<<dec<<endl;                                                                                  
		if( (word&channelMask) != 0 ) { //pilot piggy tbm trailer bits
			unsigned int chan = (word&channelMask)>>26;
			unsigned int nor = (word&NORMask)>>16;
			unsigned int autoreset = (word&AutoResetMask) >> 7;
			unsigned int pkamreset = (word&PKAMResetMask) >> 6;
			unsigned int stackCnt = (word&TBMStackCntMask);
			if(print) 
				cout<<" Diag ch="<<chan<<" #Rocs=" <<nor<<" resets? Auto/PKAM=" << autoreset << "/"<< pkamreset 
				<< " TBMStackCnt=" << stackCnt << endl;
		}
		//    else if(print) {
		//      cout<<" Dummy word"<<endl;}
		return 0;

	} else if( (word&errorMask) == gapMask ) { // GAP WORD
		//cout<<"error word "<<hex<<word<<dec<<endl;                                                                                  
		if( (word&channelMask)!=0 ) { // pilot piggy timestamps first ROC header TBM trailer
			unsigned int chan = (word&channelMask)>>26;
			unsigned int bit20 = (word & 0x100000)>>20; // on=ROC header off=TBM trailer
			unsigned int time = (word&TimeStampMask);
			if(print) {
				cout << " Chan=" << chan;
				if(bit20==1) cout << " 1st ROC H timestamp= " ;
				else cout << " TBM T timestamp= ";
				cout << time << endl;
			} //print
		} // chan !=0
		//    else if(print) cout<<" Gap word"<<endl;
		return 0;

	} else if( (word&errorMask)==timeOut ) { // TIMEOUT

		unsigned int bit20 =      (word & 0x100000)>>20; // works only for slink format

		if(phase1) {  // has two timeout words bit 20 marks
			unsigned int chan = (word&channelMask)>>26; //channel ## for timeout
			//      unsigned int l1acnt=(word& 0x1FE000) >> 13; //FED event counter only in error FIFO
			unsigned int diag = (word & 0x3f); // TBM stack or event#
			if(print) {
				cout << "Timeout Error -channel: " << chan ; // << " FED Evt# " << l1acnt ;
				if(bit20) cout <<" TBM Stack Count " << diag << endl; //maybe 0 all the time
				else cout << " Chan = " << diag <<endl;
			}
		}
		else if(bit20 == 0) { // 2nd word 

			unsigned int timeoutCnt = (word &  0x7f800)>>11; // only for slink
			// unsigned int timeoutCnt = ((word&0xfc000000)>>24) + ((word&0x1800)>>11); // only for fifo
			// More than 1 channel within a group can have a timeout error

			unsigned int index = (word & 0x1F);  // index within a group of 4/5
			unsigned int chip = (word& BlkNumMask)>>8;
			int offset = offsets[chip];
			if(print) cout<<"Timeout Error- channel: ";
			//cout<<"Timeout Error- channel: ";
			for(int i=0;i<5;i++) {
				if( (index & 0x1) != 0) {
					channel = offset + i + 1;
					if(print) cout<<channel<<" ";
					//cout<<channel<<" ";
				}
				index = index >> 1;
			}

			if(print) cout << " TimeoutCount: " << timeoutCnt;
			//cout << " TimeoutCount: " << timeoutCnt<<endl;;

			//if(print) cout<<" for Fed "<<fed<<endl;
			status = -10;
			fedChannel = channel;
			//end of timeout  chip and channel decoding

		} else {  // this is the 1st timout word with the baseline correction 

			int baselineCorr = 0;
			if(word&0x200){
				baselineCorr = -(((~word)&0x1ff) + 1);
			} else {
				baselineCorr = (word&0x1ff);
			}

			if(PRINT_BASELINE && print) cout<<"Timeout BaselineCorr: "<<baselineCorr<<endl;
			//cout<<"Timeout BaselineCorr: "<<baselineCorr<<endl;
			status = 0;
		}


	} else if( (word&errorMask) == eventNumError ) { // EVENT NUMBER ERROR
		channel =  (word & channelMask) >>26;
		unsigned int tbm_event   =  (word & tbmEventMask);

		if(print) cout<<" Event Number Error- channel: "<<channel<<" tbm event nr. "
				<<tbm_event<<" ";
		status = -11;
		fedChannel = channel;

	} else if( ((word&errorMask) == trailError)) {  // TRAILER 
		channel =  (word & channelMask) >>26;
		unsigned int tbm_status   =  (word & tbmStatusMask);


		if(tbm_status!=0) {
			if(print) cout<<" Trailer Error- "<<"channel: "<<channel<<" TBM status:0x"
					<<hex<<tbm_status<<dec<<" "; // <<endl;
			status = -15;
			// implement the resync/reset 17
		}

		if(word & RocErrMask) {
			if(print) cout<<"Number of Rocs Error- "<<"channel: "<<channel<<" "; // <<endl;
			status = -12;
		}

		if(word & overflowMask) {
			if(print) cout<<"Overflow Error- "<<"channel: "<<channel<<" "; // <<endl;
			status = -14;
		}

		if(word & FsmErrMask) {
			if(phase1) { 
				if(print) {
					cout << "word 0x" << std::hex << word << std::dec << " " ;
					if(word & 0x200)cout<<" PKAM- ";
					if(word & 0x400)cout<<" AutoReset- ";
					cout <<"channel: "<<channel;
					status = -15;
				}
			}
			else{
				if(print) {
					cout<<"Finite State Machine Error- "<<"channel: "<<channel
							<<" Error status:0x"<<hex<< ((word & FsmErrMask)>>9)<<dec<<" "; // <<endl;
					cout <<"channel: "<<channel;
				}
				status = -13;
			}
		}


		fedChannel = channel;

	} else if((word&errorMask)==fifoError) {  // FIFO
		if(print) { 
			if(word & Fif2NFMask) cout<<"A fifo 2 is Nearly full- ";
			if(word & TrigNFMask) cout<<"The trigger fifo is nearly Full - ";
			if(word & ChnFifMask) cout<<"fifo-1 is nearly full for channel"<<(word & ChnFifMask);
			//cout<<endl;
			status = -16;
		}

	} else {
		if( (word == 0) && phase1 ) { 
			// for phase1 simulations there are sometimes 0 words (fillers?)  
			status = 0; // report OK 
		} else {
			// cout<<" Unknown error?"<<" : ";
			// cout<<" for FED "<<fed<<" Word "<<hex<<word<<dec<<endl;
		}
	}

	if(print && status <0) cout<<" FED "<<fed<<" status "<<status<<endl;
	return status;
}
///////////////////////////////////////////////////////////////////////////
int MyDecode::data(int word, int & fedChannel, int fed, int & stat1, int & stat2, bool print) {

	//const int ROCMAX = 24;
	const unsigned int plsmsk = 0xff;   // pulse height
	const unsigned int pxlmsk = 0xff00; // pixel index
	const unsigned int dclmsk = 0x1f0000;
#ifdef PHASE1 // Same as phase 0.
	const unsigned int rocmsk  =  0x3e00000;  // 4 bits 
	const unsigned int chnlmsk = 0xfc000000; // 6 bits 
	//  const unsigned int rocmsk  =  0x1e00000;  // 4 bits 
	//  const unsigned int chnlmsk = 0xfe000000; // 7 bits 
	const unsigned int rocshift = 21;
	const unsigned int linkshift = 26;
	//  const unsigned int linkshift = 25;
#else
	const unsigned int rocmsk =   0x3e00000;   // 5 bits
	const unsigned int chnlmsk = 0xfc000000; // 6 bits 
	const unsigned int rocshift = 21;
	const unsigned int linkshift = 26;
#endif

	int status = 0;

	roc_ = ((word&rocmsk)>>rocshift); // rocs start from 1
	// Check for embeded special words
	if(roc_>0 && roc_<25) {  // valid ROCs go from 1-24
		//if(print) cout<<"data "<<hex<<word<<dec;
		channel_ = ((word&chnlmsk)>>linkshift);

		if(channel_>=0 && channel_ <= n_of_Channels) {  // valid channels 1-36
			//cout<<hex<<word<<dec;
			dcol_=(word&dclmsk)>>16;
			pix_=(word&pxlmsk)>>8;
			adc_=(word&plsmsk);
			fedChannel = channel_;

			col_ = convertToCol(dcol_,pix_);
			row_ = convertToRow(pix_);

			// print the roc number according to the online 0-15 scheme
			if(print) cout<<" Fed "<<fed<<" Channel- "<<channel_<<" ROC- "<<(roc_-1)<<" DCOL- "<<dcol_<<" Pixel- "
					<<pix_<<" ("<<col_<<","<<row_<<") ADC- "<<adc_<<endl;
			status++;

			if(CHECK_PIXELS) {

				// Check invalid ROC numbers
				if( ((fed>31) && (roc_>24)) || ((fed<=31) && (roc_>16))  ) {  //inv ROC
					//if(printErrors) 
					cout<<" Fed "<<fed<<" wrong roc number chan/roc/dcol/pix/adc = "<<channel_<<"/"
							<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<endl;
					status = -4;


					// protect for rerouted signals
				} else if( fed<=31 && channel_<=24 && roc_>8 ) {
					if( !( (fed==6 && channel_==1) ||(fed==9 && channel_==16) ||(fed==23 && channel_==15)
							|| (fed==31 && channel_==10) ||(fed==27 && channel_==15) )  ) {
						//if(printErrors) 
						cout<<" Fed "<<fed<<" wrong channel number, chan/roc/dcol/pix/adc = "<<channel_<<"/"
								<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<endl;
						status = -4;
					}
				}

				// will need here a loopup table which can distuinguish layer 1 links
				// to decide if we read dcol/pis or col/row
				if(print) cout<<" Fed "<<fed<<" Channel- "<<channel_<<" ROC- "<<(roc_-1)<<" DCOL- "
						<<dcol_<<" Pixel- "<<pix_<<" OR col-"<<col_<<" row-"<<row_<<") ADC- "<<adc_<<endl;

				status++;

			} else { // phase0 
				col_ = convertToCol(dcol_,pix_);
				row_ = convertToRow(pix_);

				// print the roc number according to the online 0-15 scheme
				if(print) cout<<" Fed "<<fed<<" Channel- "<<channel_<<" ROC- "<<(roc_-1)<<" DCOL- "<<dcol_<<" Pixel- "
						<<pix_<<" ("<<col_<<","<<row_<<") ADC- "<<adc_<<endl;
				status++;

				if(CHECK_PIXELS) {

					// Check invalid ROC numbers
					if( ((fed>31) && (roc_>24)) || ((fed<=31) && (roc_>16))  ) {  //inv ROC
						//if(printErrors) 
						cout<<" Fed "<<fed<<" wrong roc number chan/roc/dcol/pix/adc = "<<channel_<<"/"
								<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<endl;
						status = -4;


						// protect for rerouted signals
					} else if( fed<=31 && channel_<=24 && roc_>8 ) {
						if( !( (fed==6 && channel_==1) ||(fed==9 && channel_==16) ||(fed==23 && channel_==15)
								|| (fed==31 && channel_==10) ||(fed==27 && channel_==15) )  ) {
							//if(printErrors) 
							cout<<" Fed "<<fed<<" wrong channel number, chan/roc/dcol/pix/adc = "<<channel_<<"/"
									<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<endl;
							status = -4;
						}
					}

					// Check pixels
					if(pix_==0) {  // PIX=0
						// Detect pixel 0 events
						if(printErrors) 
							cout<<" Fed "<<fed
							<<" pix=0 chan/roc/dcol/pix/adc = "<<channel_<<"/"<<roc_-1<<"/"<<dcol_<<"/"
							<<pix_<<"/"<<adc_<<" ("<<col_<<","<<row_<<")"<<endl;
						count0++;
						stat1 = roc_-1;
						stat2 = count0;
						status = -5;

					} else if( fed==fed0 && channel_==chan0 && roc_==roc0 && dcol_==dcol0 && pix_==pix0 ) {
						// detect multiple pixels 

						count0++;
						if(printErrors) cout<<" Fed "<<fed
								//cout<<" Fed "<<fed
								<<" double pixel  chan/roc/dcol/pix/adc = "<<channel_<<"/"<<roc_-1<<"/"<<dcol_<<"/"
								<<pix_<<"/"<<adc_<<" ("<<col_<<","<<row_<<") "<<count0<<endl;
						stat1 = roc_-1;
						stat2 = count0;
						status = -6;

					} else {  // normal

						count0=0;

						fed0 = fed; chan0 =channel_; roc0 =roc_; dcol0 =dcol_; pix0=pix_;

						// Decode errors
						if(pix_<2 || pix_>161) {  // inv PIX
							if(printErrors)cout<<" Fed "<<fed<<" wrong pix number chan/roc/dcol/pix/adc = "<<channel_<<"/"
									<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<" ("<<col_<<","<<row_<<")"<<endl;
							status = -3;
						}

						if(dcol_<0 || dcol_>25) {  // inv DCOL
							if(printErrors) cout<<" Fed "<<fed<<" wrong dcol number chan/roc/dcol/pix/adc = "<<channel_<<"/"
									<<roc_-1<<"/"<<dcol_<<"/"<<pix_<<"/"<<adc_<<" ("<<col_<<","<<row_<<")"<<endl;
							status = -3;
						}

					} // check pixels

					// Summary error count (for testing only)
					if(pix_<2 || pix_>161 || dcol_<0 || dcol_>25) {
						countDecodeErrors2++;  // count pixels with errors 
						if(pix_<2 || pix_>161)  countDecodeErrors1++; // count errors
						if(dcol_<0 || dcol_>25) countDecodeErrors1++; // count errors
						//if(fed==6 && channel==35 ) cout<<" Fed "<<fed<<" wrong dcol number chan/roc/dcol/pix/adc = "<<channel<<"/"
						//			 <<roc-1<<"/"<<dcol<<"/"<<pix<<"/"<<adc<<" ("<<col<<","<<row<<")"<<endl;
					}

				}  // if CHECK_PIXELS

			} // phase 

		} else { // channel

			cout<<" Wrong channel "<<channel_<<" : "
					<<" for FED "<<fed<<" Word "<<hex<<word<<dec<<endl;
			return -2;

		}

	} else if(roc_==25) {  // ROC? 
		//unsigned int chan = ((word&chnlmsk)>>26);
		//cout<<"Wrong roc 25 "<<" in fed/chan "<<fed<<"/"<<chan<<endl;
		status=-4;

	} else {  // error word

		//cout<<"error word "<<hex<<word<<dec;
		status=error(word, fedChannel, fed, stat1, stat2, print);

	}

	return status;
}
class GLIBDataGenerator : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
public:
	explicit GLIBDataGenerator(const edm::ParameterSet&);
	~GLIBDataGenerator();
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
	TTree *tree;
	Pixel p;
	edm::Service<TFileService> fs;
	//First is number of events that the fed got data, second is number of hit
	vector<pair<int,int>> HitFedCount;
	MAP fedMap;
	Pixel_Store pix;
	int numbEvent;
	string binFileName;
};

GLIBDataGenerator::GLIBDataGenerator(const edm::ParameterSet& iConfig)
{
	//Get out the name of the object need to be filtered in root files
	string label = iConfig.getUntrackedParameter<std::string>("InputLabel");
	binFileName=iConfig.getUntrackedParameter<std::string>("OutputBinFileName");	
	//Get the raw data collected from the FED throught the object with the name specified
	//in the label variable
	rawData = consumes<FEDRawDataCollection>(label);
	//Initialize the hit counter per FED
	for (unsigned int i=0;i<n_of_FEDs;i++)
		HitFedCount.push_back(pair<int,int>(0,0));
	//Creating new structure
	cout<<"GLIBDataGenerator object is created"<<endl;
	//Tree to store full decoded raw data
	tree= new TTree("DecodedData","DecodedData");
	tree->Branch("Data",&p,"_eventID/I:_fedID/I:_layer/I:_channel/I:_ROC/I:_row/I:_col/I:_adc/I");
	//Create the cabling map that map fedid and channel to the layer it is connected to
	//Make sure you have the translation.dat file in the folder you run cmsRun
	fedMap=MakeCablingMap();
	//Configure the pixel class with the given map
	//Initialize the counter
	numbEvent=0;
}


GLIBDataGenerator::~GLIBDataGenerator()
{

	// do anything here that needs to broc.e done at desctruction time
	// (e.g. close files, deallocate resources etc.)
	cout<<"GLIBDataGenerator object is destroyed"<<endl;

}


//
// member functions
//

void
GLIBDataGenerator::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	cout<<"GLIBDataGenerator analyze is being run"<<endl;
	//increase event counter
	numbEvent++;
	//Object to get out fed raw data
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
		//FED with no data should not be checked but data should be saved
		if(rawData.size()==0){
			//adding empty event
			pix.add(event,fedId,0,0,0,0,0,0);
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
GLIBDataGenerator::beginJob()
{
	cout<<"GLIBDataGenerator beginJob is being called"<<endl;
}

// ------------ method called once each job just after ending the event loop  ------------
void
GLIBDataGenerator::endJob()
{
	ofstream f("HighFEDdata.txt");
	cout<<"GLIBDataGenerator endJob is being called"<<endl;
	float max=0;
	int fedMax=-1;
	for(unsigned int i=0;i<HitFedCount.size();i++){
		if(!HitFedCount[i].first) continue;
		if((HitFedCount[i].second/numbEvent)>max){
			fedMax=i;
			max=HitFedCount[i].second/numbEvent;
		}
	}
	//Convert index to FED ID
	fedMax+=FIRST_FED_ID;
	//Annoucing
	cout<<"FED ID with the highest average hit:"<<fedMax<<endl;
	cout<<"The total number of event is: "<<numbEvent<<endl;
	cout<<"The total number of hit the FED got: "<<HitFedCount[fedMax-FIRST_FED_ID].second<<endl;
	cout<<"The average hit of that FED is:"<<max<<endl;
	cout<<"The number of zero event this FED has:"<<numbEvent-HitFedCount[fedMax-FIRST_FED_ID].first<<endl;
	cout<<"Channel-to-layer map of this FED"<<endl;
	for(int i=1;i<=48;i++){
		MAP::iterator layer=fedMap.find(FEDINP(fedMax,i));
		if(layer->second==5)
			cout<<"Channel "<<i<<" is linked to layer FPIX"<<endl;
		else
			cout<<"Channel "<<i<<" is linked to layer "<<layer->second<<endl;
	}
	f<<"FED ID with the highest average hit:"<<fedMax<<endl;
	f<<"The total number of event is: "<<numbEvent<<endl;
	f<<"The total number of hit the FED got: "<<HitFedCount[fedMax-FIRST_FED_ID].second<<endl;
	f<<"The average hit of that FED is:"<<max<<endl;
	f<<"The number of zero event this FED has:"<<numbEvent-HitFedCount[fedMax-FIRST_FED_ID].first<<endl;
	f<<"Channel-to-layer map of this FED"<<endl;
	for(int i=1;i<=48;i++){
		MAP::iterator layer=fedMap.find(FEDINP(fedMax,i));
		if(layer->second==5)
			f<<"Channel "<<i<<" is linked to layer FPIX"<<endl;
		else
			f<<"Channel "<<i<<" is linked to layer "<<layer->second<<endl;
	}
	f.close();
	cout<<"Data about the FED is saved in the file named HighFedData.txt"<<endl; 
	//Filtering and put data of the FED witih highest average hit into the encoder to generate GLIB files
	cout<<"Filtering out the data of the FED with the highest average hit"<<endl;
	for(unsigned int i=0;i<tree->GetEntries();i++){
		tree->GetEntry(i);
		pix.add(p._eventID,p._fedID,p._layer,p._channel,p._ROC,p._row,p._col,p._adc);
	}
	//Save only the latest version of the trees
	tree->Write("",TObject::kOverwrite);
	//Close the file so that when the deconstructor of this class is called, the data in the memory won't be wr
	fs->file().Close();
	//Generating GLIB bin files
	cout<<"Generating GLIB bin files"<<endl;
	pix.encode(fedMax,binFileName);
	cout<<"Done generating GLIB bin files"<<endl;
}
//Get layer from fedID and channel
Int_t GLIBDataGenerator::getLayer(Int_t fedID,Int_t channel){
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
GLIBDataGenerator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(GLIBDataGenerator);
