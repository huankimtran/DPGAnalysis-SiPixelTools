/** \class SiPixelRawDumper_H
 *  Plug-in module that dump raw data file 
 *  for pixel subdetector
 *  Added class to interpret the data d.k. 30/10/08
 *  Add histograms. Add pix 0 detection.
 * Works with v7x, comment out the digis access.
 * Adopted for Phase1.
 * Add simple error vs fed num histos.
 */

#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"


#include "DataFormats/Common/interface/Handle.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDRawData.h"

#include "DataFormats/FEDRawData/interface/FEDNumbering.h"

#include "EventFilter/SiPixelRawToDigi/interface/PixelDataFormatter.h"

//#include "CalibFormats/SiPixelObjects/src/PixelNameTranslation.cc"

// To use root histos
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// For ROOT
#include <TROOT.h>
//#include <TChain.h>
#include <TFile.h>
#include <TF1.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TStyle.h>

#include <iostream>
#include <fstream>

using namespace std;

//#define L1  // L1 information not in RAW
//#define OUTFILE 
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
#else
  const int n_of_FEDs = 41;
  const int n_of_Channels = 36;
  const int fedIdBpixMax = 32;
  const bool phase1 = false;
#endif
}

// Include the helper decoding class (how did I mange to avoid linking conflicts?)
/////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////
//Returns 1,2,3 for layer 1,2,3 full modules, 11,12,13 for 1/2 modules
// 0 for fpix
// needs fedid 0-31, and channel 1-36.
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
////////////////////////////////////////////////////////////////////////////

class SiPixelRawDump : public edm::EDAnalyzer {
public:

  /// ctor
  explicit SiPixelRawDump( const edm::ParameterSet& cfg);

  //explicit SiPixelRawDump( const edm::ParameterSet& cfg) : theConfig(cfg) {
  //consumes<FEDRawDataCollection>(theConfig.getUntrackedParameter<std::string>("InputLabel","source"));} 

  /// dtor
  virtual ~SiPixelRawDump() {}

  void beginJob();

  //void beginRun( const edm::EventSetup& ) {}

  // end of job 
  void endJob();

  /// get data, convert to digis attach againe to Event
  virtual void analyze(const edm::Event&, const edm::EventSetup&);

private:
  edm::ParameterSet theConfig;
  edm::EDGetTokenT<FEDRawDataCollection> rawData;

  int printLocal;
  double printThreshold;
  int countEvents, countAllEvents;
  int countTotErrors;
  float sumPixels, sumFedSize, sumFedPixels[n_of_FEDs];
  int fedErrors[n_of_FEDs][n_of_Channels];
  int fedErrorsENE[n_of_FEDs][n_of_Channels];
  int fedErrorsTime[n_of_FEDs][n_of_Channels];
  int fedErrorsOver[n_of_FEDs][n_of_Channels];
  int decodeErrors[n_of_FEDs][n_of_Channels];
  int decodeErrors000[n_of_FEDs][n_of_Channels];  // pix 0 problem
  int decodeErrorsDouble[n_of_FEDs][n_of_Channels];  // double pix  problem
  int errorType[20];
  int lastEvent = 0;
  int totalEvents = 0;
  vector< vector<int> > map;
  //vector< vector<int> > lyr1;
 
#ifdef OUTFILE
  ofstream outfile;
#endif

//  TH1D *hsize,*hsize0, *hsize1, *hsize2, *hsize3;
//#ifdef IND_FEDS
//  TH1D *hsizeFeds[n_of_FEDs];
//#endif
//  TH1D *hpixels, *hpixels0, *hpixels1, *hpixels2, *hpixels3, *hpixels4;
//  TH1D *htotPixels,*htotPixels0, *htotPixels1, *htotPixels2;
//  TH1D *herrors, *htotErrors;
//  TH1D *herrorType1, *herrorType1Fed, *herrorType1Chan,*herrorType2, *herrorType2Fed, *herrorType2Chan;
//  TH1D *hcountDouble, *hcount000, *hrocDouble, *hroc000;
//
//  TH2F *hfed2DErrorsType1,*hfed2DErrorsType2;
//  TH2F *hfed2DErrors1,*hfed2DErrors2,*hfed2DErrors3,*hfed2DErrors4,*hfed2DErrors5,
//    *hfed2DErrors6,*hfed2DErrors7,*hfed2DErrors8,*hfed2DErrors9,*hfed2DErrors10,*hfed2DErrors11,*hfed2DErrors12,
//    *hfed2DErrors13,*hfed2DErrors14,*hfed2DErrors15,*hfed2DErrors16;
//  TH2F *hfed2d, *hsize2d,*hfedErrorType1ls,*hfedErrorType2ls, *hcountDouble2, *hcount0002;
//#ifdef IND_FEDS
//  TH2F *hfed2DErrors1ls,*hfed2DErrors2ls,*hfed2DErrors3ls,*hfed2DErrors4ls,*hfed2DErrors5ls,
//    *hfed2DErrors6ls,*hfed2DErrors7ls,*hfed2DErrors8ls,*hfed2DErrors9ls,*hfed2DErrors10ls,*hfed2DErrors11ls,*hfed2DErrors12ls,
//    *hfed2DErrors13ls,*hfed2DErrors14ls,*hfed2DErrors15ls,*hfed2DErrors16ls;
//#endif
//
//  TH1D *hlumi, *horbit, *hbx, *hlumi0, *hbx0, *hbx1, *hbx2;
//  TProfile *htotPixelsls, *hsizels, *herrorType1ls, *herrorType2ls, *htotPixelsbx, 
//    *herrorType1bx,*herrorType2bx,*havsizebx,*hsizep;
//  TProfile *herror1ls,*herror2ls,*herror3ls,*herror4ls,*herror5ls,*herror6ls,*herror7ls,*herror8ls,
//    *herror9ls,*herror10ls,*herror11ls,*herror12ls,*herror13ls,*herror14ls,*herror15ls,*herror16ls; 
//  TProfile2D *hfedchannelsize;
//  TH1D *herrorTimels, *herrorOverls, *herrorTimels1, *herrorOverls1, *herrorTimels2, *herrorOverls2, 
//       *herrorTimels3, *herrorOverls3, *herrorTimels0, *herrorOverls0;
//  TH1D *hfedchannelsizeb,*hfedchannelsizeb1,*hfedchannelsizeb2,*hfedchannelsizeb3,
//    *hfedchannelsizef;
//  TH1D *hadc1,*hadc2,*hadc3,*hadc0; 
//  TProfile *hadc1ls,*hadc2ls,*hadc3ls,*hadc0ls; 
//  TProfile *hadc1bx,*hadc2bx,*hadc3bx,*hadc0bx; 
//  TH1D *htimeoutFed, *hoverflowFed, *hnorFed, *heneFed, *hpixFed;

  //TH1D *hFED[139], *eFED[139];
  TH2D *hFEDChannel[139], *eFEDChannel[139], *sumhFED, *sumeFED, *eFEDType[139], *sumType, *hModule, *hNoise;
  TH1F *PKAMCount, *NoiseCount, *EventCount;

  MyDecode decode;
};
//----------------------------------------------------------------------------------
SiPixelRawDump::SiPixelRawDump( const edm::ParameterSet& cfg) : theConfig(cfg) {
  string label = theConfig.getUntrackedParameter<std::string>("InputLabel","source");
  // For the ByToken method
  rawData = consumes<FEDRawDataCollection>(label);
} 
//----------------------------------------------------------------------------------------
void SiPixelRawDump::endJob() {
  string errorName[18] = {" "," ","wrong channel","wrong pix or dcol","wrong roc","pix=0",
			  " double-pix"," "," "," ","timeout","ENE","NOR","FSM","overflow",
			  "trailer","fifo","reset/resync"};

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

  cout<<"2 - wrong channel"<<endl
      <<"3 - wrong pix or dcol"<<endl 
      <<"4 - wrong roc"<<endl
      <<"5 - pix=0"<<endl
      <<"6 - double pixel"<<endl 
      <<"10 - timeout"<<endl
      <<"11 - ene"<<endl
      <<"12 - mum pf rocs error"<<endl
      <<"13 - fsm"<<endl
      <<"14 - overflow"<<endl
      <<"15 - trailer"<<endl
      <<"16 - fifo"<<endl
      <<"17 - reset/resync NOT INCLUDED YET"<<endl;

  double tmp = sumPixels;
  if(countEvents>0) {
    sumPixels /= float(countEvents);
    sumFedSize /= float(countAllEvents);
    for(int i = 0; i < n_of_FEDs; ++i) {
      sumFedPixels[i] /= float(countEvents);
      //hpixels4->Fill(float(i),sumFedPixels[i]); //pixels only 
    }
  }
    
  cout<<" Total/non-empty events " <<countAllEvents<<" / "<<countEvents<<" average number of pixels "<<sumPixels<<" ("<<tmp<<")"<<endl;

  cout<<" Average Fed size per event for all events (in 4-words) "<< (sumFedSize*2./static_cast<float>(n_of_FEDs)) 
      <<" total for all feds "<<(sumFedSize*2.) <<endl;

  cout<<" Size for ech FED per event in units of hit pixels:" <<endl;

  for(int i = 0; i < n_of_FEDs; ++i) cout<< sumFedPixels[i]<<" ";
  cout<<endl;

  cout<<" Total number of errors "<<countTotErrors<<" print threshold "<< int(countEvents*printThreshold) << " total errors per fed channel"<<endl;
  cout<<" FED errors "<<endl<<"Fed Channel Tot-Errors ENE-Errors Time-Errors Over-Errors"<<endl;

  for(int i = 0; i < n_of_FEDs; ++i) {
    for(int j=0;j<n_of_Channels;++j) if( (fedErrors[i][j]) > int(countEvents*printThreshold) || (fedErrorsENE[i][j] > 0) ) {
      cout<<" "<<i<<"  -  "<<(j+1)<<" -  "<<fedErrors[i][j]<<" - "<<fedErrorsENE[i][j]<<" -  "<<fedErrorsTime[i][j]<<" - "<<fedErrorsOver[i][j]<<endl;
    }
  }
  cout<<" Decode errors "<<endl<<"Fed Channel Errors Pix_000 Double_Pix"<<endl;
  for(int i = 0; i < n_of_FEDs; ++i) {
    for(int j=0;j<n_of_Channels;++j) {
      int tmp = decodeErrors[i][j] + decodeErrors000[i][j] + decodeErrorsDouble[i][j];
      if(tmp>10) 
	cout<<" "<<i<<" -  "<<(j+1)<<"   -  "
	    <<decodeErrors[i][j]<<"  -    "
	    <<decodeErrors000[i][j]<<"  -   "
	    <<decodeErrorsDouble[i][j]<<endl;
    }
  }

  cout<<" Total errors for all feds "<<endl<<" Type Name Num-Of-Errors"<<endl;
  for(int i=0;i<20;++i) {
    if( errorType[i]>0 ) cout<<"   "<<i<<" - "<<errorName[i]<<" - "<<errorType[i]<<endl;
  }

  cout<<" Test decode errors "<<countDecodeErrors1<<" "<<countDecodeErrors2<<endl;

  cout<<"Total number of Events = "<< totalEvents << endl;


#ifdef OUTFILE
  outfile.close();
#endif



}
//----------------------------------------------------------------------
void SiPixelRawDump::beginJob() {

  printLocal = theConfig.getUntrackedParameter<int>("Verbosity",1);
  printThreshold = theConfig.getUntrackedParameter<double>("PrintThreshold",0.001); // threshold per event for printing errors
  cout<<" beginjob "<<printLocal<<" "<<printThreshold<<endl;  

  if(printLocal>0) printErrors  = true;
  else printErrors = false;
  if(printLocal>1) printData  = true;
  else printData = false;
  if(printLocal>2) printHeaders  = true;
  else printHeaders = false;

  countEvents=0;
  countAllEvents=0;
  countTotErrors=0;
  sumPixels=0.;
  sumFedSize=0;
  for(int i = 0; i < n_of_FEDs; ++i) {
    sumFedPixels[i]=0;
    for(int j=0;j<n_of_Channels;++j) {fedErrors[i][j]=0; fedErrorsENE[i][j]=0; fedErrorsTime[i][j]=0; fedErrorsOver[i][j]=0;}
    for(int j=0;j<n_of_Channels;++j) {decodeErrors[i][j]=0; decodeErrors000[i][j]=0; decodeErrorsDouble[i][j]=0;}
  }
  for(int i=0;i<20;++i) errorType[i]=0;


  //************************************* Make the FED map **************************************
  cout << "Making FED map." << endl;

  string filename = ("translation.dat");
  ifstream in(filename.c_str());

  string dummy;
  string line;
  //getline(in, dummy);

  string layer_string;
  unsigned int layer_read;
  unsigned int fed_read;
  unsigned int channel_read;
  int linenum = -1;
  //int counts = 0; 

  while (getline(in,line)) {
    linenum++;
	cout<<line<<endl;
    getline(in, dummy, '_');
    if (dummy.compare("FPix") == 0) break;
    getline(in, dummy, '_');
    getline(in, dummy, '_');
    getline(in, layer_string, '_');
    getline(in, dummy, '_');
    getline(in, dummy, '_');
    getline(in, dummy, '_');
    for (int i=0;i<8;i++){
      in >> dummy;
    }
    in >> fed_read;
    in >> channel_read;
    in >> dummy;

    layer_string = layer_string.substr(3);
    layer_read = atoi(layer_string.c_str());

    vector<int> row;
    row.push_back(layer_read);
    row.push_back(fed_read);
    row.push_back(channel_read);
   
    map.push_back(row);

    //if (layer_read == 1) {
    //  vector<int> row2;
    //  row2.push_back(counts);
    //  row2.push_back(fed_read);
    //  row2.push_back(channel_read);
    //  
    //  lyr1.push_back(row2);
    //  counts++;
    //}  

    //cout << "Layer = " << map[linenum][0] << ", FED = " << map[linenum][1] << ", Channel = " << map[linenum][2] << endl; 
  }

  cout << "Completed FED Map" << endl;
  cout <<"Number of line "<<linenum<<endl;
  //************************************* Done making map  **************************************

  edm::Service<TFileService> fs;

  TFileDirectory hits = fs->mkdir("hits");
  TFileDirectory pkam = fs->mkdir("pkam");
  TFileDirectory noise = fs->mkdir("noise");
 
  //const float pixMax = 5999.5;   // pp value 
  //const float totMax = 99999.5;  // pp value 
  //const float maxLink = 200.;    // pp value

  //const float pixMax = 19999.5;  // hi value 
  //const float totMax = 399999.5; // hi value 
  //const float maxLink = 1000.;   // hi value


  //hsize = fs->make<TH1D>( "hsize", "FED event size in words-4", 6000, -0.5, pixMax);
  //hsize0 = fs->make<TH1D>( "hsize0", "FED event size in words-4 (zoomed)", 2000, -0.5, 1999.5);
  //hsize1 = fs->make<TH1D>( "hsize1", "bpix FED event size in words-4", 6000, -0.5, pixMax);
  //hsize2 = fs->make<TH1D>( "hsize2", "fpix FED event size in words-4", 6000, -0.5, pixMax);
  //hsize3 = fs->make<TH1D>( "hsize3", "ave bpix FED event size in words-4", 6000, -0.5, pixMax);

  //hpixels = fs->make<TH1D>( "hpixels", "pixels per FED (zoomed)", 2000, -0.5, 1999.5);
  //hpixels0 = fs->make<TH1D>( "hpixels0", "pixels per FED", 6000, -0.5, pixMax);
  //hpixels1 = fs->make<TH1D>( "hpixels1", "pixels >0 per FED", 6000, -0.5, pixMax);
  //hpixels2 = fs->make<TH1D>( "hpixels2", "pixels >0 per BPix FED", 6000, -0.5, pixMax);
  //hpixels3 = fs->make<TH1D>( "hpixels3", "pixels >0 per Fpix FED", 6000, -0.5, pixMax);
  //hpixels4 = fs->make<TH1D>( "hpixels4", "pixels per each FED", n_of_FEDs, -0.5, static_cast<float>(n_of_FEDs) - 0.5);

  //htotPixels = fs->make<TH1D>( "htotPixels", "pixels per event", 10000, -0.5, totMax);
  //htotPixels0 = fs->make<TH1D>( "htotPixels0", "pixels per event, zoom low region", 20000, -0.5, 19999.5);
  //htotPixels1 = fs->make<TH1D>( "htotPixels1", "pixels >0 per event", 10000, -0.5, totMax);
  //htotPixels2 = fs->make<TH1D>( "htotPixels2", "pixels per event for wrong BX", 10000, -0.5, totMax);

  //herrors = fs->make<TH1D>( "herrors", "errors per FED", 100, -0.5, 99.5);
  //htotErrors = fs->make<TH1D>( "htotErrors", "errors per event", 1000, -0.5, 999.5);

  //const float maxChan = static_cast<float>(n_of_Channels) - 0.5;

  // herrorType1     = fs->make<TH1D>( "herrorType1", "errors-1(fed) per type", 
  //       			    20, -0.5, 19.5);
  // herrorType1Fed  = fs->make<TH1D>( "herrorType1Fed", "errors-1(fed) per FED", 
  //       			    n_of_FEDs, -0.5, static_cast<float>(n_of_FEDs) - 0.5);
  // herrorType1Chan = fs->make<TH1D>( "herrorType1Chan", "errors-1(fed) per chan", 
  //       			    n_of_Channels, -0.5,maxChan);
  // hfed2DErrorsType1 = fs->make<TH2F>("hfed2DErrorsType1", "errors type-1(fed) per FED", 
  //         n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfedErrorType1ls = fs->make<TH2F>( "hfedErrorType1ls", "errors type-1 (fed) vs lumi",
  //         300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); // 
  // herrorType1ls = fs->make<TProfile>("herrorType1ls","error type-1 (fed) vs ls",300,0,3000,0,1000.);
  // herrorType1bx = fs->make<TProfile>("herrorType1bx"," error type-1 (fed) vs bx",4000,-0.5,3999.5,0,300000.);


  // herrorType2     = fs->make<TH1D>( "herrorType2", "readout errors type-2 (decode)per type", 
  //       			    20, -0.5, 19.5);
  // herrorType2Fed  = fs->make<TH1D>( "herrorType2Fed", "readout errors type-2 (decode) per FED", 
  //       			    n_of_FEDs, -0.5, static_cast<float>(n_of_FEDs) - 0.5);
  // herrorType2Chan = fs->make<TH1D>( "herrorType2Chan", "readout errors type-2 (decode) per chan", 
  //       			    n_of_Channels, -0.5,maxChan);
  // hfed2DErrorsType2 = fs->make<TH2F>("hfed2DErrorsType2", "errors type-2 (decode) per FED", 
  //         n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfedErrorType2ls = fs->make<TH2F>( "hfedErrorType2ls", "errors type-2 (decode) vs lumi",
  //         300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // herrorType2ls = fs->make<TProfile>("herrorType2ls","error type-2(decode) vs ls",300,0,3000,0,1000.);
  // herrorType2bx = fs->make<TProfile>("herrorType2bx"," error type-2(decode) vs bx",4000,-0.5,3999.5,0,300000.);

  // hcountDouble = fs->make<TH1D>( "hcountDouble", "count double pixels", 100, -0.5, 99.5);
  // hcountDouble2 = fs->make<TH2F>("hcountDouble2","count double pixels versus fed#",
  //       			 n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, 10,0.,10.); 
  // hcount000 = fs->make<TH1D>( "hcount000", "count 000 pixels", 100, -0.5, 99.5);
  // hcount0002 = fs->make<TH2F>("hcount0002","count 000 pixels versus fed",
  //       		      n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, 10,0.,10.); 
  // hrocDouble = fs->make<TH1D>( "hrocDouble", "rocs with double pixels", 25,-0.5, 24.5);
  // hroc000    = fs->make<TH1D>( "hroc000", "rocs with 000 pixels", 25,-0.5, 24.5);

  // hfed2d = fs->make<TH2F>( "hfed2d", "error type versus fed#", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5,
  //       		   21, -0.5, 20.5); // ALL

  // hsize2d = fs->make<TH2F>( "hsize2d", "size vs fed",n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, 50,0,500); // ALL
  // hsizep  = fs->make<TProfile>( "hsizep", "size vs fed",n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5,0,100000); // ALL
  //hsize2dls = fs->make<TH2F>( "hsize2dls", "size vs lumi",100,0,1000, 50,0.,500.); // ALL

#ifdef IND_FEDS
  //hsizeFeds[0] = fs->make<TH1D>( "hsizeFed0", "FED 0 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[1] = fs->make<TH1D>( "hsizeFed1", "FED 1 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[2] = fs->make<TH1D>( "hsizeFed2", "FED 2 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[3] = fs->make<TH1D>( "hsizeFed3", "FED 3 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[4] = fs->make<TH1D>( "hsizeFed4", "FED 4 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[5] = fs->make<TH1D>( "hsizeFed5", "FED 5 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[6] = fs->make<TH1D>( "hsizeFed6", "FED 6 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[7] = fs->make<TH1D>( "hsizeFed7", "FED 7 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[8] = fs->make<TH1D>( "hsizeFed8", "FED 8 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[9] = fs->make<TH1D>( "hsizeFed9", "FED 9 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[10] = fs->make<TH1D>( "hsizeFed10", "FED 10 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[11] = fs->make<TH1D>( "hsizeFed11", "FED 11 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[12] = fs->make<TH1D>( "hsizeFed12", "FED 12 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[13] = fs->make<TH1D>( "hsizeFed13", "FED 13 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[14] = fs->make<TH1D>( "hsizeFed14", "FED 14 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[15] = fs->make<TH1D>( "hsizeFed15", "FED 15 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[16] = fs->make<TH1D>( "hsizeFed16", "FED 16 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[17] = fs->make<TH1D>( "hsizeFed17", "FED 17 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[18] = fs->make<TH1D>( "hsizeFed18", "FED 18 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[19] = fs->make<TH1D>( "hsizeFed19", "FED 19 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[20] = fs->make<TH1D>( "hsizeFed20", "FED 20 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[21] = fs->make<TH1D>( "hsizeFed21", "FED 21 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[22] = fs->make<TH1D>( "hsizeFed22", "FED 22 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[23] = fs->make<TH1D>( "hsizeFed23", "FED 23 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[24] = fs->make<TH1D>( "hsizeFed24", "FED 24 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[25] = fs->make<TH1D>( "hsizeFed25", "FED 25 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[26] = fs->make<TH1D>( "hsizeFed26", "FED 26 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[27] = fs->make<TH1D>( "hsizeFed27", "FED 27 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[28] = fs->make<TH1D>( "hsizeFed28", "FED 28 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[29] = fs->make<TH1D>( "hsizeFed29", "FED 29 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[30] = fs->make<TH1D>( "hsizeFed30", "FED 30 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[31] = fs->make<TH1D>( "hsizeFed31", "FED 31 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[32] = fs->make<TH1D>( "hsizeFed32", "FED 32 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[33] = fs->make<TH1D>( "hsizeFed33", "FED 33 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[34] = fs->make<TH1D>( "hsizeFed34", "FED 34 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[35] = fs->make<TH1D>( "hsizeFed35", "FED 35 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[36] = fs->make<TH1D>( "hsizeFed36", "FED 36 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[37] = fs->make<TH1D>( "hsizeFed37", "FED 37 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[38] = fs->make<TH1D>( "hsizeFed38", "FED 38 event size ", 1000, -0.5, pixMax);
  //hsizeFeds[39] = fs->make<TH1D>( "hsizeFed39", "FED 39 event size ", 1000, -0.5, pixMax);
#endif

  //hevent = fs->make<TH1D>("hevent","event",1000,0,10000000.);
  //horbit = fs->make<TH1D>("horbit","orbit",100, 0,100000000.);
  //hlumi   = fs->make<TH1D>("hlumi", "lumi-section all", 3000,0,3000.);
  //hlumi0  = fs->make<TH1D>("hlumi0","lumi-section pix>0", 3000,0,3000.);
 
  //hbx    = fs->make<TH1D>("hbx",   "bx (cmssw) with hits",   4000,0,4000.);  
  //hbx0    = fs->make<TH1D>("hbx0",   "bx (cmssw) all",   4000,0,4000.);  
  //hbx1    = fs->make<TH1D>("hbx1",   "bx fed payload",   4000,0,4000.);  
  //hbx2    = fs->make<TH1D>("hbx2",   "bad (cmssw) bx",   4000,0,4000.);  

  //herrorTimels = fs->make<TH1D>( "herrorTimels", "timeouts vs ls", 1000,0,3000);
  //herrorOverls = fs->make<TH1D>( "herrorOverls", "overflows vs ls",1000,0,3000);

  //herrorTimels1 = fs->make<TH1D>("herrorTimels1","timeouts vs ls lay1", 1000,0,3000);
  //herrorOverls1 = fs->make<TH1D>("herrorOverls1","overflows vs ls lay1",1000,0,3000);
  //herrorTimels2 = fs->make<TH1D>("herrorTimels2","timeouts vs ls lay2", 1000,0,3000);
  //herrorOverls2 = fs->make<TH1D>("herrorOverls2","overflows vs ls lay2",1000,0,3000);
  //herrorTimels3 = fs->make<TH1D>("herrorTimels3","timeouts vs ls lay3", 1000,0,3000);
  //herrorOverls3 = fs->make<TH1D>("herrorOverls3","overflows vs ls lay3",1000,0,3000);
  //herrorTimels0 = fs->make<TH1D>("herrorTimels0","timeouts vs ls disk", 1000,0,3000);
  //herrorOverls0 = fs->make<TH1D>("herrorOverls0","overflows vs ls disk",1000,0,3000);

  //herrorTimels = fs->make<TH1D>( "herrorTimels", "timeouts vs ls", 1000,0,3000);
  //herrorOverls = fs->make<TH1D>( "herrorOverls", "overflows vs ls",1000,0,3000);
  //herrorTimels1 = fs->make<TH1D>("herrorTimels1","timeouts vs ls", 1000,0,3000);
  //herrorOverls1 = fs->make<TH1D>("herrorOverls1","overflows vs ls",1000,0,3000);
  //herrorTimels2 = fs->make<TH1D>("herrorTimels2","timeouts vs ls", 1000,0,3000);
  //herrorOverls2 = fs->make<TH1D>("herrorOverls2","overflows vs ls",1000,0,3000);
  //herrorTimels3 = fs->make<TH1D>("herrorTimels3","timeouts vs ls", 1000,0,3000);
  //herrorOverls3 = fs->make<TH1D>("herrorOverls3","overflows vs ls",1000,0,3000);
  //herrorTimels0 = fs->make<TH1D>("herrorTimels0","timeouts vs ls", 1000,0,3000);
  //herrorOverls0 = fs->make<TH1D>("herrorOverls0","overflows vs ls",1000,0,3000);

  // Error Type ploted versus fed and channel #
  //hfed2DErrors1 = fs->make<TH2F>("hfed2DErrors1", "errors type 1 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors2 = fs->make<TH2F>("hfed2DErrors2", "errors type 2per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors3 = fs->make<TH2F>("hfed2DErrors3", "errors type 3 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors4 = fs->make<TH2F>("hfed2DErrors4", "errors type 4 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors5 = fs->make<TH2F>("hfed2DErrors5", "errors type 5 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors6 = fs->make<TH2F>("hfed2DErrors6", "errors type 6 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors7 = fs->make<TH2F>("hfed2DErrors7", "errors type 7 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors8 = fs->make<TH2F>("hfed2DErrors8", "errors type 8 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors9 = fs->make<TH2F>("hfed2DErrors9", "errors type 9 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors10 = fs->make<TH2F>("hfed2DErrors10", "errors type 10 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors11 = fs->make<TH2F>("hfed2DErrors11", "errors type 11 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors12 = fs->make<TH2F>("hfed2DErrors12", "errors type 12 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors13 = fs->make<TH2F>("hfed2DErrors13", "errors type 13 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors14 = fs->make<TH2F>("hfed2DErrors14", "errors type 14 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors15 = fs->make<TH2F>("hfed2DErrors15", "errors type 15 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  //hfed2DErrors16 = fs->make<TH2F>("hfed2DErrors16", "errors type 16 per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);

#ifdef IND_FEDS
  //hfed2DErrors1ls  = fs->make<TH2F>("hfed2DErrors1ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors2ls  = fs->make<TH2F>("hfed2DErrors2ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors3ls  = fs->make<TH2F>("hfed2DErrors3ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors4ls  = fs->make<TH2F>("hfed2DErrors4ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors5ls  = fs->make<TH2F>("hfed2DErrors5ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors6ls  = fs->make<TH2F>("hfed2DErrors6ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors7ls  = fs->make<TH2F>("hfed2DErrors7ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors8ls  = fs->make<TH2F>("hfed2DErrors8ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors9ls  = fs->make<TH2F>("hfed2DErrors9ls", "errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors10ls = fs->make<TH2F>("hfed2DErrors10ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors11ls = fs->make<TH2F>("hfed2DErrors11ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors12ls = fs->make<TH2F>("hfed2DErrors12ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors13ls = fs->make<TH2F>("hfed2DErrors13ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors14ls = fs->make<TH2F>("hfed2DErrors14ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors15ls = fs->make<TH2F>("hfed2DErrors15ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  //hfed2DErrors16ls = fs->make<TH2F>("hfed2DErrors16ls","errors type 6 vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
#endif

  // hfed2DErrors1 = fs->make<TH2F>("hfed2DErrors1", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // //hfed2DErrors2 = fs->make<TH2F>("hfed2DErrors2", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors3 = fs->make<TH2F>("hfed2DErrors3", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors4 = fs->make<TH2F>("hfed2DErrors4", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors5 = fs->make<TH2F>("hfed2DErrors5", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors6 = fs->make<TH2F>("hfed2DErrors6", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // //hfed2DErrors7 = fs->make<TH2F>("hfed2DErrors7", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // //hfed2DErrors8 = fs->make<TH2F>("hfed2DErrors8", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // //hfed2DErrors9 = fs->make<TH2F>("hfed2DErrors9", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors10 = fs->make<TH2F>("hfed2DErrors10", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors11 = fs->make<TH2F>("hfed2DErrors11", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors12 = fs->make<TH2F>("hfed2DErrors12", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors13 = fs->make<TH2F>("hfed2DErrors13", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors14 = fs->make<TH2F>("hfed2DErrors14", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors15 = fs->make<TH2F>("hfed2DErrors15", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);
  // hfed2DErrors16 = fs->make<TH2F>("hfed2DErrors16", "errors per FED", n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5, n_of_Channels, -0.5,maxChan);

  // hfed2DErrors1ls  = fs->make<TH2F>("hfed2DErrors1ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // //hfed2DErrors2ls  = fs->make<TH2F>("hfed2DErrors2ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors3ls  = fs->make<TH2F>("hfed2DErrors3ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors4ls  = fs->make<TH2F>("hfed2DErrors4ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors5ls  = fs->make<TH2F>("hfed2DErrors5ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors6ls  = fs->make<TH2F>("hfed2DErrors6ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // //hfed2DErrors7ls  = fs->make<TH2F>("hfed2DErrors7ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // //hfed2DErrors8ls  = fs->make<TH2F>("hfed2DErrors8ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // //hfed2DErrors9ls  = fs->make<TH2F>("hfed2DErrors9ls", "errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors10ls = fs->make<TH2F>("hfed2DErrors10ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors11ls = fs->make<TH2F>("hfed2DErrors11ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors12ls = fs->make<TH2F>("hfed2DErrors12ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors13ls = fs->make<TH2F>("hfed2DErrors13ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors14ls = fs->make<TH2F>("hfed2DErrors14ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors15ls = fs->make<TH2F>("hfed2DErrors15ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //
  // hfed2DErrors16ls = fs->make<TH2F>("hfed2DErrors16ls","errors vs lumi",300,0,3000, n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5); //

  //hsizels = fs->make<TProfile>("hsizels"," bpix fed size vs ls",300,0,3000,0,200000.);
  //htotPixelsls = fs->make<TProfile>("htotPixelsls"," tot pixels vs ls",300,0,3000,0,300000.);
  //havsizels = fs->make<TProfile>("havsizels","av. bpix fed size vs ls",300,0,3000,0,300000.);

  //herror1ls = fs->make<TProfile>("herror1ls","error 1 vs ls",300,0,3000,0,1000.);
  //herror2ls = fs->make<TProfile>("herror2ls","error 2 vs ls",300,0,3000,0,1000.);
  //herror3ls = fs->make<TProfile>("herror3ls","error 3 vs ls",300,0,3000,0,1000.);
  //herror4ls = fs->make<TProfile>("herror4ls","error 4 vs ls",300,0,3000,0,1000.);
  //herror5ls = fs->make<TProfile>("herror5ls","error 5 vs ls",300,0,3000,0,1000.);
  //herror6ls = fs->make<TProfile>("herror6ls","error 6 vs ls",300,0,3000,0,1000.);
  //herror7ls = fs->make<TProfile>("herror7ls","error 7 vs ls",300,0,3000,0,1000.);
  //herror8ls = fs->make<TProfile>("herror8ls","error 8 vs ls",300,0,3000,0,1000.);
  //herror9ls = fs->make<TProfile>("herror9ls","error 9 vs ls",300,0,3000,0,1000.);
  //herror10ls = fs->make<TProfile>("herror10ls","error 10 vs ls",300,0,3000,0,1000.);
  //herror11ls = fs->make<TProfile>("herror11ls","error 11 vs ls",300,0,3000,0,1000.);
  //herror12ls = fs->make<TProfile>("herror12ls","error 12 vs ls",300,0,3000,0,1000.);
  //herror13ls = fs->make<TProfile>("herror13ls","error 13 vs ls",300,0,3000,0,1000.);
  //herror14ls = fs->make<TProfile>("herror14ls","error 14 vs ls",300,0,3000,0,1000.);
  //herror15ls = fs->make<TProfile>("herror15ls","error 15 vs ls",300,0,3000,0,1000.);
  //herror16ls = fs->make<TProfile>("herror16ls","error 16 vs ls",300,0,3000,0,1000.);
  //
  //htotPixelsbx = fs->make<TProfile>("htotPixelsbx"," tot pixels vs bx",4000,-0.5,3999.5,0,300000.);
  //havsizebx = fs->make<TProfile>("havsizebx"," ave bpix fed size vs bx",4000,-0.5,3999.5,0,300000.);
  //hintgl  = fs->make<TProfile>("hintgl", "inst lumi vs ls ",1000,0.,3000.,0.0,1000.);
  //hinstl  = fs->make<TProfile>("hinstl", "intg lumi vs ls ",1000,0.,3000.,0.0,10.);

  //hfedchannelsize  = fs->make<TProfile2D>("hfedchannelsize", "pixels per fed&channel",
  //      				  n_of_FEDs,-0.5,static_cast<float>(n_of_FEDs) - 0.5,
  //      				  n_of_Channels, -0.5,maxChan,0.0,10000.);  
  //hfedchannelsizeb   = fs->make<TH1D>("hfedchannelsizeb", "pixels per bpix channel",200,0.0,maxLink);
  //hfedchannelsizeb1  = fs->make<TH1D>("hfedchannelsizeb1", "pixels per bpix1 channel",200,0.0,maxLink);
  //hfedchannelsizeb2  = fs->make<TH1D>("hfedchannelsizeb2", "pixels per bpix2 channel",200,0.0,maxLink);
  //hfedchannelsizeb3  = fs->make<TH1D>("hfedchannelsizeb3", "pixels per bpix3 channel",200,0.0,maxLink);
  //hfedchannelsizef   = fs->make<TH1D>("hfedchannelsizef", "pixels per fpix channel",200,0.0,maxLink);

  //hadc1 = fs->make<TH1D>("hadc1","adc lay 1",255,0.,255.);
  //hadc2 = fs->make<TH1D>("hadc2","adc lay 2",255,0.,255.);
  //hadc3 = fs->make<TH1D>("hadc3","adc lay 3",255,0.,255.);
  //hadc0 = fs->make<TH1D>("hadc0","adc fpix",255,0.,255.);

  //hadc1ls = fs->make<TProfile>("hadc1ls","adc1 vs ls",1000,0,1000,     0.,255.);
  //hadc1bx = fs->make<TProfile>("hadc1bx","adc1 vs bx",4000,-0.5,3999.5,0.,255.);
  //hadc2ls = fs->make<TProfile>("hadc2ls","adc2 vs ls",1000,0,1000,     0.,255.);
  //hadc2bx = fs->make<TProfile>("hadc2bx","adc2 vs bx",4000,-0.5,3999.5,0.,255.);
  //hadc3ls = fs->make<TProfile>("hadc3ls","adc3 vs ls",1000,0,1000,     0.,255.);
  //hadc3bx = fs->make<TProfile>("hadc3bx","adc3 vs bx",4000,-0.5,3999.5,0.,255.);
  //hadc0ls = fs->make<TProfile>("hadc0ls","adc0 vs ls",1000,0,1000,     0.,255.);
  //hadc0bx = fs->make<TProfile>("hadc0bx","adc0 vs bx",4000,-0.5,3999.5,0.,255.);


  //htimeoutFed  = fs->make<TH1D>("htimeoutFed","timeouts per fed",41,0.,41.);
  //hoverflowFed = fs->make<TH1D>("hoverflowFed","overflow per fed",41,0.,41.);
  //hnorFed      = fs->make<TH1D>("hnorFed","nors per fed",41,0.,41.);
  //heneFed      = fs->make<TH1D>("heneFed","enes per fed",41,0.,41.);
  //hpixFed      = fs->make<TH1D>("hpixFed","pix&dcol per fed",41,0.,41.);

  EventCount = hits.make<TH1F>("EventCount", "Number of Events", 1, 0, 1);

  string titles;
  int fed_num_int = 0;
  string fed_num_string;
  string fed_label;
  string labels;
  //const char *label_char;
  //const char *title_char;
  string labels2;
  string titles2;
  const char *label_char2;
  const char *title_char2;
  string binLabel;

  for (int i = 0; i < 139; i++){
    fed_num_int = i + 1200;
    fed_num_string = to_string(fed_num_int);
    fed_label = to_string(i);
    //labels = "hFED" + fed_label; 
    //titles = "Hits in FED #" + fed_num_string;
    //label_char = labels.c_str();
    //title_char = titles.c_str();
    //hFED[i] = fs->make<TH1D>(label_char, title_char, 100, 0., 2000.0);
    //hFED[i]->GetXaxis()->SetTitle("Number of Hits");
    //hFED[i]->GetYaxis()->SetTitle("Number of Occurances");

    labels2 = "hFEDChannel" + fed_label;
    titles2 = "Hits in FED #" + fed_num_string + " in Each Channel";
    label_char2 = labels2.c_str();
    title_char2 = titles2.c_str();
    hFEDChannel[i] = hits.make<TH2D>(label_char2, title_char2, 48, 1., 49., 600, -0.5, 599.5);
    hFEDChannel[i]->GetXaxis()->SetTitle("Channel Number");
    hFEDChannel[i]->GetYaxis()->SetTitle("Number of Hits");
    //hFEDChannel[i]->SetPalette(1);
    //hFEDChannel[i]->SetContour(28);
    hFEDChannel[i]->SetOption("COLZ");

    //labels = "eFED" + fed_label;
    //titles = "Errors in FED #" + fed_num_string;
    //label_char = labels.c_str();
    //title_char = titles.c_str();
    //eFED[i] = fs->make<TH1D>(label_char, title_char, 15, 0., 15.0);
    //eFED[i]->GetXaxis()->SetTitle("Number of Errors");
    //eFED[i]->GetYaxis()->SetTitle("Number of Occurances");

    labels2 = "eFEDChannel" + fed_label;
    titles2 = "Errors in FED #" + fed_num_string + " in Each Channel";
    label_char2 = labels2.c_str();
    title_char2 = titles2.c_str();
    eFEDChannel[i] = hits.make<TH2D>(label_char2, title_char2, 48, 1., 49., 5, 0., 5.);
    eFEDChannel[i]->GetXaxis()->SetTitle("Channel Number");
    eFEDChannel[i]->GetYaxis()->SetTitle("Number of Errors");
    eFEDChannel[i]->SetOption("COLZ");

    labels2 = "eFEDType" + fed_label;
    titles2 = "Errors Types in FED #" + fed_num_string + " in Each Channel";
    label_char2 = labels2.c_str();
    title_char2 = titles2.c_str();
    eFEDType[i] = hits.make<TH2D>(label_char2, title_char2, 48, 1., 49., 17, 1., 18.);
    eFEDType[i]->GetXaxis()->SetTitle("Channel Number");
    eFEDType[i]->GetYaxis()->SetTitle("Type of Error");
    eFEDType[i]->SetOption("COLZ");
    eFEDType[i]->GetYaxis()->SetBinLabel(1,"timeout");
    eFEDType[i]->GetYaxis()->SetBinLabel(2,"wrong channel");
    eFEDType[i]->GetYaxis()->SetBinLabel(3,"wrong pix/dcol");
    eFEDType[i]->GetYaxis()->SetBinLabel(4,"wrong roc");
    eFEDType[i]->GetYaxis()->SetBinLabel(5,"pix=0");
    eFEDType[i]->GetYaxis()->SetBinLabel(6,"double pixel");
    //eFEDType[i]->GetYaxis()->SetBinLabel(7,"timeout");
    //eFEDType[i]->GetYaxis()->SetBinLabel(8,"trailer");
    //eFEDType[i]->GetYaxis()->SetBinLabel(9,"event number");
    //eFEDType[i]->GetYaxis()->SetBinLabel(10,"timeout");
    eFEDType[i]->GetYaxis()->SetBinLabel(11,"event number");
    eFEDType[i]->GetYaxis()->SetBinLabel(12,"# of roc");
    eFEDType[i]->GetYaxis()->SetBinLabel(13,"fsm");
    eFEDType[i]->GetYaxis()->SetBinLabel(14,"overflow");
    eFEDType[i]->GetYaxis()->SetBinLabel(15,"trailer");
    eFEDType[i]->GetYaxis()->SetBinLabel(16,"fifo");
    eFEDType[i]->GetYaxis()->SetBinLabel(17,"reset/resync");
  }

  sumhFED = hits.make<TH2D>("sumhFED", "Data Output of Each FED", 138, 0., 138., 200, 0., 10000.0);
  sumhFED->GetXaxis()->SetTitle("FED Number");
  sumhFED->GetYaxis()->SetTitle("Number of Bytes");
  sumhFED->SetOption("COLZ");

  sumeFED = hits.make<TH2D>("sumeFED", "Number of Errors in Each FED", 138, 0., 138., 15, 0., 15.0);
  sumeFED->GetXaxis()->SetTitle("FED Number");
  sumeFED->GetYaxis()->SetTitle("Number of Errors");
  sumeFED->SetOption("COLZ");

  sumType = hits.make<TH2D>("sumType", "Error Types in Each FED", 138, 0., 138., 17, 1., 18.);
  sumType->GetXaxis()->SetTitle("FED Number");
  sumType->GetYaxis()->SetTitle("Error Type");
  sumType->SetOption("COLZ");
  //sumType->GetYaxis()->SetBinLabel(0," ");
  sumType->GetYaxis()->SetBinLabel(1,"timeout");
  sumType->GetYaxis()->SetBinLabel(2,"wrong channel");
  sumType->GetYaxis()->SetBinLabel(3,"wrong pix/dcol");
  sumType->GetYaxis()->SetBinLabel(4,"wrong roc");
  sumType->GetYaxis()->SetBinLabel(5,"pix=0");
  sumType->GetYaxis()->SetBinLabel(6,"double pixel");
  //sumType->GetYaxis()->SetBinLabel(7,"timeout");
  //sumType->GetYaxis()->SetBinLabel(8,"trailer");
  //sumType->GetYaxis()->SetBinLabel(9,"event number");
  //sumType->GetYaxis()->SetBinLabel(10,"timeout");
  sumType->GetYaxis()->SetBinLabel(11,"event number");
  sumType->GetYaxis()->SetBinLabel(12,"# of roc");
  sumType->GetYaxis()->SetBinLabel(13,"fsm");
  sumType->GetYaxis()->SetBinLabel(14,"overflow");
  sumType->GetYaxis()->SetBinLabel(15,"trailer");
  sumType->GetYaxis()->SetBinLabel(16,"fifo");
  sumType->GetYaxis()->SetBinLabel(17,"reset/resync");

  PKAMCount = pkam.make<TH1F>("PKAMCount", "Number of PKAMs in Each Layer", 5, 1, 6);
  PKAMCount->GetXaxis()->SetTitle("Layer");
  PKAMCount->GetYaxis()->SetTitle("Number of Occurances");
  //for (unsigned int i = 0;i<lyr1.size();i++) {
  //  binLabel = to_string(lyr1[i][1]) + "\n" + to_string(lyr1[i][2]);
  //  PKAMCount->GetXaxis()->SetBinLabel((lyr1[i][0]+1),binLabel.c_str());
  //}
  for (int i=1;i<5;i++) {
    PKAMCount->GetXaxis()->SetBinLabel(i,to_string(i).c_str());
  }
  PKAMCount->GetXaxis()->SetBinLabel(5,"FPix");


  NoiseCount = noise.make<TH1F>("NoiseCount", "Number of Noise Events", 5, 1, 6);
  NoiseCount->GetXaxis()->SetTitle("Layer");
  NoiseCount->GetYaxis()->SetTitle("Number of Occurances");
  for (int i=1;i<5;i++) {
    NoiseCount->GetXaxis()->SetBinLabel(i,to_string(i).c_str());
  }
  NoiseCount->GetXaxis()->SetBinLabel(5,"FPix");
 

#ifdef OUTFILE
  outfile.open("pixfed.csv");
  for(int i = 0; i < n_of_FEDs; ++i) {if(i<fedIdBpixMax_sup) outfile<<i+1200<<","; else outfile<<i+1200<<endl;}
#endif

}
//-----------------------------------------------------------------------
void SiPixelRawDump::analyze(const  edm::Event& ev, const edm::EventSetup& es) {
  const bool printEventInfo = false;

  // Access event information
  int run       = ev.id().run();
  int event     = ev.id().event();
  int lumiBlock = ev.luminosityBlock();
  int bx        = ev.bunchCrossing(); // CMSSW bx
  //int orbit     = ev.orbitNumber();
  static int oldEvent=0; // ,LS=0;
 
  if (event != lastEvent){
    totalEvents++;
    lastEvent = event;
    EventCount->Fill(0);
  }

  edm::Service<TFileService> fs;
  TFileDirectory pkam = fs->mkdir("pkam");
  TFileDirectory noise = fs->mkdir("noise");

  TStyle* gStyle = new TStyle("gStyle","Let's get fancy!");
  //gStyle->SetOptStat(0);
  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);
  gStyle->SetCanvasColor(16);
  gStyle->cd();
 
  countAllEvents++;
  //hevent->Fill(float(event));
  //hlumi0->Fill(float(lumiBlock));
  //hbx0->Fill(float(bx));
  //horbit->Fill(float(orbit));

  if(printEventInfo) {
    // if(countAllEvents<100 || (countAllEvents&1000)==0 )
    //   cout<<" Run "<<run<<" LS "<<lumiBlock<<" event "<<event<<" bx "<<bx
    // 	  <<" all "<<countAllEvents<<endl;
    //if( lumiBlock != LS) {
      cout<<" Run "<<run<<" LS "<<lumiBlock<<" event "<<event<<" bx "<<bx
	  <<" all "<<countAllEvents<<endl;
      //LS=lumiBlock;
      //}
      //if(event<oldEvent) 
      cout<<"   Lower event number: Run "<<run<<" LS "<<lumiBlock<<" event "<<event<<" bx "<<bx
	  <<" all "<<countAllEvents<<" "<<oldEvent<<endl;
    //oldEvent=event;
    //return; // skip the rest 
  }

  if(printHeaders) 
    cout<<"Event = "<<countAllEvents<<" Event number "<<event<<" Run "<<run
	<<" LS "<<lumiBlock<<endl;


  // Get lumi info (does not work for raw)
//  edm::LuminosityBlock const& iLumi = ev.getLuminosityBlock();
//   edm::Handle<LumiSummary> lumi;
//   iLumi.getByLabel("lumiProducer", lumi);
//   edm::Handle<edm::ConditionsInLumiBlock> cond;
//   float intlumi = 0, instlumi=0;
//   int beamint1=0, beamint2=0;
//   iLumi.getByLabel("conditionsInEdm", cond);
//   // This will only work when running on RECO until (if) they fix it in the FW
//   // When running on RAW and reconstructing, the LumiSummary will not appear
//   // in the event before reaching endLuminosityBlock(). Therefore, it is not
//   // possible to get this info in the event
//   if (lumi.isValid()) {
//     intlumi =(lumi->intgRecLumi())/1000.; // integrated lumi per LS in -pb
//     instlumi=(lumi->avgInsDelLumi())/1000.; //ave. inst lumi per LS in -pb
//     beamint1=(cond->totalIntensityBeam1)/1000;
//     beamint2=(cond->totalIntensityBeam2)/1000;
//   } else {
//     std::cout << "** ERROR: Event does not get lumi info\n";
//   }
//   cout<<instlumi<<" "<<intlumi<<" "<<lumiBlock<<endl;

//   hinstl->Fill(float(lumiBlock),float(instlumi));
//   hintgl->Fill(float(lumiBlock),float(intlumi));


  edm::Handle<FEDRawDataCollection> buffers;
  //static std::string label = theConfig.getUntrackedParameter<std::string>("InputLabel","source");
  //static std::string instance = theConfig.getUntrackedParameter<std::string>("InputInstance","");  
  //ev.getByLabel( label, instance, buffers);
  ev.getByToken(rawData , buffers);  // the new bytoken 

#ifdef PHASE1
  std::pair<int,int> fedIds(1200,1338); // phase 1
  const int fedId0=1200;
  //std::pair<int,int> fedIds(0,138);
  //const int fedId0=0;
#else // phase0
  std::pair<int,int> fedIds(FEDNumbering::MINSiPixelFEDID, FEDNumbering::MAXSiPixelFEDID); //0
  const int fedId0=0;
#endif


  //PixelDataFormatter formatter(0);  // only for digis
  //bool dummyErrorBool;

  //typedef unsigned int Word32;
  //typedef long long Word64;
  typedef uint32_t Word32;
  typedef uint64_t Word64;
  int status=0;
  int countPixels=0;
  int eventId = -1;
  int countErrorsPerEvent=0;
  int countErrorsPerEvent1=0;
  int countErrorsPerEvent2=0;
  double aveFedSize = 0.;
  int stat1=-1, stat2=-1;
  int fedchannelsize[n_of_Channels];
  bool wrongBX=false;
  int countErrors[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  //unsigned int hitsInFed[139];
  //for (int i=0;i<139;i++){
  //  hitsInFed[i] = 0;
  //}
  unsigned int errInFed[139];
  for (int i=0;i<139;i++){
    errInFed[i] = 0;
  }
  unsigned int FEDsize[139];
  for (int i=0;i<139;i++){
    FEDsize[i] = 0;
  }

  string FED_string;
  string channel_string;
  string event_string;
  string modLabel;
  string modTitle;
  const char *modLabel_char;
  const char *modTitle_char;

  //For decoding errors
  const unsigned int rocMask = 0x3e00000;
  const unsigned int errorMask =0x3e00000;
  const unsigned int trailError = 0x3c00000;
  const unsigned int fsmMask = 0x700;
  const unsigned int PKAMreset = 0x200;
  const unsigned int channelMask = 0xfc000000;
  const unsigned int rocShift = 21;
  const unsigned int channelShift = 26;

  //For plotting row and col
  const unsigned int plsmsk = 0xff;   // pulse height                                                                      
  const unsigned int pxlmsk = 0xff00; // pixel index                                                                       
  const unsigned int dclmsk = 0x1f0000;

  // Loop over FEDs
  for (int fedId = fedIds.first; fedId < (fedIds.second + 1); fedId++) {

    //edm::DetSetVector<PixelDigi> collection;
    PixelDataFormatter::Errors errors;

    //get event data for this fed
    const FEDRawData& rawData = buffers->FEDData( fedId );

    FEDsize[fedId-fedId0] = rawData.size();

    if(printHeaders) cout<<"Get data For FED = "<<fedId<<" size in bytes "<<rawData.size()<<endl;
    if(rawData.size()==0) continue;  // skip if not data for this fed

    for(int i=0;i<n_of_Channels;++i) fedchannelsize[i]=0;

    int nWords = rawData.size()/sizeof(Word64);
    //cout<<" size "<<nWords<<endl;

    sumFedSize += float(nWords);    
    if(fedId<fedIdBpixMax) aveFedSize += double(2.*nWords);

    //hFED[fedId - fedId0]->Fill(float(rawData.size()));

    //hsize->Fill(float(2*nWords)); // fed buffer size in words (32bit)
    //hsize0->Fill(float(2*nWords)); // fed buffer size in words (32bit)
    //if(fedId<fedIdBpixMax_sup) hsize1->Fill(float(2*nWords)); // bpix fed buffer size in words (32bit)
    //else hsize2->Fill(float(2*nWords)); // fpix fed buffer size in words (32bit)

#ifdef IND_FEDS
    //hsizeFeds[fedId-fedId0]->Fill(float(2*nWords)); // size, includes errors and dummy words
#endif
    //hsize2d->Fill(float(fedId-fedId0),float(2*nWords));  // 2d 
    //hsizep->Fill(float(fedId-fedId0),float(2*nWords)); // profile 
    //if(fedId<fedIdBpixMax_sup) hsizels->Fill(float(lumiBlock),float(2*nWords)); // bpix versu sls

    // check headers
    const Word64* header = reinterpret_cast<const Word64* >(rawData.data()); 
    //cout<<hex<<*header<<dec<<endl;

    unsigned int bxid = 0;
    eventId = MyDecode::header(*header, fedId, printHeaders, bxid);
    //if(fedId = fedIds.first) 
    if(bx != int(bxid) ) { 
      wrongBX=true;
      if(printErrors && printBX && !phase1) 
	cout<<" Inconsistent BX: for event "<<event<<" (fed-header event "<<eventId<<") for LS "<<lumiBlock
	    <<" for run "<<run<<" for bx "<<bx<<" fed bx "<<bxid<<endl;
    }
    if(bx<0) bx=bxid; // if cmssw bx=-1 use the fed bx (for pilot)
    //hbx1->Fill(float(bxid));


    const Word64* trailer = reinterpret_cast<const Word64* >(rawData.data())+(nWords-1);
    //cout<<hex<<*trailer<<dec<<endl;
    status = MyDecode::trailer(*trailer,fedId, printHeaders);

    int countPixelsInFed=0;
    int countErrorsInFed=0;
    int countErrorsInFed1=0;
    int countErrorsInFed2=0;
    int fedChannel = 0;
    int num=0;
    int plotChannel = 0;
    unsigned int hitsInChannel[48];
    for (int i=0;i<48;i++){
      hitsInChannel[i]=0;
    }
    unsigned int errInChannel[48];
    for (int i=0;i<48;i++){
      errInChannel[i]=0;
    }
    int plotRoc = 0;
    int PKAMChannel = -1;
    int PKAMEvent = -1;
    int PKAMRoc = -1;
    int layer = 1;
    int dcol;
    int pix;
    int adc;
    int row;
    int col;
    //int col_count[416];
    //for (int i=0;i<416;i++) {
    //  col_count[i]=0;
    //}
    vector<int> col_count (416,0);
    int col_max;
    int NoiseChannel = -1;
    int NoisefedId = -1;

    vector<int> channelWords;
    int lastChannel = 1;

    // Loop over payload words
    for (const Word64* word = header+1; word != trailer; word++) {
      static const Word64 WORD32_mask  = 0xffffffff;

      for(int ipart=0;ipart<2;++ipart) {
	Word32 w = 0;
	if(ipart==0) {
	  w =  *word       & WORD32_mask;  // 1st word
	  //w1=w;
	} else if(ipart==1) {
	  w =  *word >> 32 & WORD32_mask;  // 2nd word
	}

	num++;
	if(printLocal>3) cout<<" "<<num<<" "<<hex<<w<<dec<<endl;

        plotRoc = ((w&rocMask)>>rocShift);
        plotChannel = ((w&channelMask)>>channelShift); 

        if ((plotChannel != lastChannel) && (channelWords.size() != 0)) {

          PKAMEvent = event;
          NoiseChannel = (channelWords[0]&channelMask)>>channelShift;

          //if (plotChannel != 1) {
          //  NoiseChannel = plotChannel - 1;
          //} else {
          //  NoiseChannel = 48;
          //}

          //if (plotChannel < lastChannel) {
          //  NoisefedId = fedId - 1;
          //} else {
          //  NoisefedId = fedId;       
          //}

          if (NoiseChannel == 1) {
            if (fedId == 1200) {
              NoisefedId = 1339;
            } else {
              NoisefedId = fedId - 1;
            }
          } else {
            NoisefedId = fedId;
          }
 
          if (NoisefedId > 1295) {
            layer = 5;
          } else {
            for (unsigned int it = 0; it < map.size(); it++) {
              if (map[it][1] == NoisefedId && map[it][2] == NoiseChannel) {
                layer = map[it][0];
                break;
              }
            }
          } 

          for (unsigned int i=0;i < channelWords.size(); i++) {
            PKAMRoc = (channelWords[i]&rocMask)>>rocShift;
            if (PKAMRoc <= 16) {
              if (layer == 1) {
                dcol = (channelWords[i]&0x1F8000)>>15;
                col = dcol;
              } else {
                dcol=(channelWords[i]&dclmsk)>>16;
                pix=(channelWords[i]&pxlmsk)>>8;
                col = dcol*2 + pix%2;
              }
              col = (PKAMRoc-1)*52 + col;
              //cout << col << endl;
              col_count[col]++;
             
            }
          }

          for (int i=0;i<416;i+=2){
            //cout << col_count[i] << endl;
            if (col_max < (col_count[i]+col_count[i+1])) {
              col_max = col_count[i] + col_count[i+1];
            }
            col_count[i] = 0;
            col_count[i+1] = 0;
          }

          //cout << "col_max = " << col_max << endl;

          if (col_max > 100) {
            if (layer < 5) {
              cout << "We got us a noise boi in Layer " << layer << ", FED " << NoisefedId << ", Channel " << NoiseChannel << " with " << col_max << " hits" << endl;
            } else {
              cout << "We got us a noise boi in FPix, FED " << NoisefedId << ", Channel " << NoiseChannel << " with " << col_max << " hits" << endl;
            }

            col_max = 0;           
            NoiseCount->Fill(layer); 

            if (layer == 1) {

              FED_string = to_string((NoisefedId-fedId0));
              channel_string = to_string(NoiseChannel);
              event_string = to_string(PKAMEvent);
              modLabel = "Layer1FED" + FED_string + "Channel" + channel_string;
              FED_string = to_string(NoisefedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hNoise = noise.make<TH2D>(modLabel_char, modTitle_char, 104, 0., 104., 80, 0., 80.);
              hNoise->GetXaxis()->SetTitle("column");
              hNoise->GetYaxis()->SetTitle("row");
              hNoise->SetOption("COLZ");

            } else if (layer == 2) {

              FED_string = to_string((NoisefedId-fedId0));
              channel_string = to_string(NoiseChannel);
              event_string = to_string(PKAMEvent);
              modLabel = "Layer2FED" + FED_string + "Channel" + channel_string;
              FED_string = to_string(NoisefedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hNoise = noise.make<TH2D>(modLabel_char, modTitle_char, 208, 0., 208., 80, 0., 80.);
              hNoise->GetXaxis()->SetTitle("column");
              hNoise->GetYaxis()->SetTitle("row");
              hNoise->SetOption("COLZ");

            } else {

              FED_string = to_string((NoisefedId-fedId0));
              channel_string = to_string(NoiseChannel);
              event_string = to_string(PKAMEvent);
              if (layer < 5) {
                modLabel = "Layer" + to_string(layer) + "FED" + FED_string + "Channel" + channel_string;
              } else {
                modLabel = "FPixFED" + FED_string + "Channel" + channel_string;
              }
              FED_string = to_string(NoisefedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hNoise = noise.make<TH2D>(modLabel_char, modTitle_char, 416, 0., 416., 160, 0., 160.);
              hNoise->GetXaxis()->SetTitle("column");
              hNoise->GetYaxis()->SetTitle("row");
              hNoise->SetOption("COLZ");

            }
 
            for (unsigned int i=0;i < channelWords.size(); i++) {
              PKAMRoc = (channelWords[i]&rocMask)>>rocShift;
              if (PKAMRoc <= 16) {
                if (layer == 1) {
                  pix = (channelWords[i]&0x7F00)>>8;   // 7 bit Y ~ row?
                  dcol = (channelWords[i]&0x1F8000)>>15;  // 6 Bit  X ~ column?
                  adc = (channelWords[i]&plsmsk);
                  
                  row = pix;
                  col = dcol;
                }
                else {
                  dcol=(channelWords[i]&dclmsk)>>16;
                  pix=(channelWords[i]&pxlmsk)>>8;
                  adc=(channelWords[i]&plsmsk);
                  
                  row = 80 - (pix/2);
                  col = dcol*2 + pix%2;
 
                  //cout << "FED " << fedId << ", Channel " << PKAMChannel << ", ROC " << PKAMRoc << " has dcol " << dcol << " -> " << col << " -> " << (PKAMRoc-1)*52+col << endl;
                  //cout << "FED " << fedId << ", Channel " << PKAMChannel << ", ROC " << PKAMRoc << " has pix " << pix << " -> " << pix2 << " -> " << ((pix2&0x1c0)>>6) << ((pix2&0x38)>>3) << (pix2&0x7) << " -> " << row << " -> " << abs((PKAMRoc/9)*160 - row) << endl;
                }
 
                col = (PKAMRoc-1)*52 + col;
                row = abs((PKAMRoc/9)*160 - row);

                //Fill histogram
                hNoise->Fill(col,row,adc);  
              }             
            }
          }

          for (int i=0;i<416;i++) {
            col_count[i]=0;
          }
          col_max = 0;
          lastChannel = plotChannel;
          channelWords.erase (channelWords.begin(),channelWords.end());
          channelWords.push_back(w);
        } else {
          channelWords.push_back(w);
        }

        if (plotRoc <= 16) { 
          hitsInChannel[plotChannel]++;   
        }
        else if (plotRoc > 24) {
          errInChannel[plotChannel-1]++;
          errInFed[fedId - fedId0]++;
          
          if (((w&errorMask) == trailError) && ((w&fsmMask) == PKAMreset) && (channelWords.size() > 200)){
            PKAMChannel = plotChannel;
            PKAMEvent = event;
            
            if (fedId > 1295) {
              layer = 5;
            } else {
              for (unsigned int it = 0; it < map.size(); it++) {
                //cout << map[it][0] << " " << map[it][1] << " " << map[it][2] << endl;
                if (map[it][1] == fedId && map[it][2] == PKAMChannel) {
                  layer = map[it][0];
                  //cout << "FED " << fedId << ", Channel " << PKAMChannel << " is layer " << layer << endl;
                  break;
                }
              }
            }

            if (layer < 5) {
              cout << "We got us a PKAM bois in Layer " << layer << ", FED " << fedId << ", Channel " << PKAMChannel  << endl;
            } else {
              cout << "We got us a PKAM bois in FPix, FED " << fedId << ", Channel " << PKAMChannel  << endl;
            }

            PKAMCount->Fill(layer);

            if (layer == 1) {

              FED_string = to_string((fedId-fedId0));
              channel_string = to_string(plotChannel);
              event_string = to_string(PKAMEvent);
              modLabel = "Layer1FED" + FED_string + "Channel" + channel_string;
              FED_string = to_string(fedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hModule = pkam.make<TH2D>(modLabel_char, modTitle_char, 104, 0., 104., 80, 0., 80.);
              hModule->GetXaxis()->SetTitle("column");
              hModule->GetYaxis()->SetTitle("row");
              hModule->SetOption("COLZ");

              //for (unsigned int i=0;i<lyr1.size();i++) {
              //  if (lyr1[i][1] == fedId && lyr1[i][2] == PKAMChannel) {
              //    PKAMCount->Fill(lyr1[i][0]);
              //    break;
              //  }
              //}

            } else if (layer == 2) {

              FED_string = to_string((fedId-fedId0));
              channel_string = to_string(plotChannel);
              event_string = to_string(PKAMEvent);
              modLabel = "Layer2FED" + FED_string + "Channel" + channel_string;
              FED_string = to_string(fedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hModule = pkam.make<TH2D>(modLabel_char, modTitle_char, 208, 0., 208., 80, 0., 80.);
              hModule->GetXaxis()->SetTitle("column");
              hModule->GetYaxis()->SetTitle("row");
              hModule->SetOption("COLZ");

            } else {
 
              FED_string = to_string((fedId-fedId0));
              channel_string = to_string(plotChannel);
              event_string = to_string(PKAMEvent);
              if (layer < 5) {
                modLabel = "Layer" + to_string(layer) + "FED" + FED_string + "Channel" + channel_string;
              } else {
                modLabel = "FPixFED" + FED_string + "Channel" + channel_string;
              }
              FED_string = to_string(fedId);
              modTitle = "Hits in Event " + event_string + ", FED " + FED_string + ", Channel " + channel_string;
              modLabel_char = modLabel.c_str();
              modTitle_char = modTitle.c_str();
              hModule = pkam.make<TH2D>(modLabel_char, modTitle_char, 416, 0., 416., 160, 0., 160.);
              hModule->GetXaxis()->SetTitle("column");
              hModule->GetYaxis()->SetTitle("row");
              hModule->SetOption("COLZ");
 
            }        

            for (unsigned int i=0;i < channelWords.size(); i++) {
              PKAMRoc = (channelWords[i]&rocMask)>>rocShift;
              if (PKAMRoc <= 16) {  
                if (layer == 1) {
                  pix = (channelWords[i]&0x7F00)>>8;   // 7 bit Y ~ row?
                  dcol = (channelWords[i]&0x1F8000)>>15;  // 6 Bit  X ~ column?
                  adc = (channelWords[i]&plsmsk);              

                  row = pix;
                  col = dcol;
                }
                else {
                  dcol=(channelWords[i]&dclmsk)>>16;
                  pix=(channelWords[i]&pxlmsk)>>8;
                  adc=(channelWords[i]&plsmsk);
             
                  row = 80 - (pix/2);
                  col = dcol*2 + pix%2;
 
                  //cout << "FED " << fedId << ", Channel " << PKAMChannel << ", ROC " << PKAMRoc << " has dcol " << dcol << " -> " << col << " -> " << (PKAMRoc-1)*52+col << endl; 
                  //cout << "FED " << fedId << ", Channel " << PKAMChannel << ", ROC " << PKAMRoc << " has pix " << pix << " -> " << pix2 << " -> " << ((pix2&0x1c0)>>6) << ((pix2&0x38)>>3) << (pix2&0x7) << " -> " << row << " -> " << abs((PKAMRoc/9)*160 - row) << endl;
                }
            
                col = (PKAMRoc-1)*52 + col; 
                row = abs((PKAMRoc/9)*160 - row);

                //Fill histogram
                hModule->Fill(col,row,adc);
              }
            }
          }

        }

	//status = MyDecode::data(w,fedChannel, fedId, stat1, stat2, printData);
	status = decode.data(w,fedChannel, fedId, stat1, stat2, printData);
	//int layer = MyDecode::checkLayerLink(fedId, fedChannel); // get bpix layer, works only for phase0 
	if(phase1) layer=0;
	if(layer>10) layer = layer-10; // ignore 1/2 modules 
	if(status>0) {  // data
	  countPixels++;
	  countPixelsInFed++;
	  fedchannelsize[fedChannel-1]++;
	  
	  //int adc = decode.get_adc();
	  //if(layer==1)      
	    //{hadc1->Fill(float(adc));hadc1ls->Fill(float(lumiBlock),float(adc));hadc1bx->Fill(float(bx),float(adc));}
	  //else if(layer==2) 
	    //{hadc2->Fill(float(adc));hadc2ls->Fill(float(lumiBlock),float(adc));hadc2bx->Fill(float(bx),float(adc));}
	  //else if(layer==3) 
	    //{hadc3->Fill(float(adc));hadc3ls->Fill(float(lumiBlock),float(adc));hadc3bx->Fill(float(bx),float(adc));}
	  //else if(layer==0) 
	    //{hadc0->Fill(float(adc));hadc0ls->Fill(float(lumiBlock),float(adc));hadc0bx->Fill(float(bx),float(adc));}
	  //cout<<adc<<endl;

	} else if(status<0) {  // error word
	  countErrorsInFed++;
	  //if( status == -6 || status == -5) 
	  if(printErrors) cout<<"    Bad stats for FED "<<fedId<<" Event "<<eventId<<"/"<<(eventId%256)
			      <<" count "<<countAllEvents<<" chan "<<fedChannel<<" status "<<status<<endl;
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

	    //htimeoutFed->Fill(float(fedId-fedId0));
	    countErrors[10]++;
	    fedErrorsTime[fedId-fedId0][(fedChannel-1)]++;
	    //hfed2DErrors10->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors10ls->Fill(float(lumiBlock),float(fedId-fedId)); //errors

	    //herrorTimels->Fill(float(lumiBlock));
	    //if(layer==1)      herrorTimels1->Fill(float(lumiBlock));
	    //else if(layer==2) herrorTimels2->Fill(float(lumiBlock));
	    //else if(layer==3) herrorTimels3->Fill(float(lumiBlock));
	    //else if(layer==0) herrorTimels0->Fill(float(lumiBlock));

	    break; } 

	  case(14) : {  // OVER

	    //hoverflowFed->Fill(float(fedId-fedId0));
	    countErrors[14]++;
	    fedErrorsOver[fedId-fedId0][(fedChannel-1)]++;
	    //hfed2DErrors14->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors14ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors

	    //herrorOverls->Fill(float(lumiBlock));
	    //if(layer==1)      herrorOverls1->Fill(float(lumiBlock));
	    //else if(layer==2) herrorOverls2->Fill(float(lumiBlock));
	    //else if(layer==3) herrorOverls3->Fill(float(lumiBlock));
	    //else if(layer==0) herrorOverls0->Fill(float(lumiBlock));
	    break; }

	  case(11) : {  // ENE

	    //heneFed->Fill(float(fedId-fedId0));
	    countErrors[11]++;
	    //hfed2DErrors11->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors11ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    fedErrorsENE[fedId-fedId0][(fedChannel-1)]++;
	    break; }

	  case(16) : { //FIFO

	    countErrors[16]++;
	    //hfed2DErrors16->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors16ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(12) : {  // NOR

	    //hnorFed->Fill(float(fedId-fedId0));
	    countErrors[12]++;
	    //hfed2DErrors12->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors12ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(15) : {  // TBM Trailer

	    countErrors[15]++;
	    //hfed2DErrors15->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors15ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(13) : {  // FSM

	    countErrors[13]++;
	    //hfed2DErrors13->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors13ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(3) : {  //  inv. pix-dcol
	    //hpixFed->Fill(float(fedId-fedId0));
	    countErrors[3]++;
	    //hfed2DErrors3->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors3ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(4) : {  // inv roc
	    countErrors[4]++;
	    //hfed2DErrors4->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors4ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  case(5) : {  // pix=0
	    countErrors[5]++;
	    //hfed2DErrors5->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors5ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors

	    //hroc000->Fill(float(stat1)); // count rocs
	    //hcount000->Fill(float(stat2));
	    //hcount0002->Fill(float(fedId-fedId0),float(stat2));
	    break; }

	  case(6) : {  // double pix

	    countErrors[6]++;
	    //hfed2DErrors6->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors6ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors

	    //hrocDouble->Fill(float(stat1)); // count rocs
	    //hcountDouble->Fill(float(stat2));
	    //hcountDouble2->Fill(float(fedId-fedId0),float(stat2));
	    break; }

	  case(1) : {  // unknown
	    countErrors[1]++;
	    //hfed2DErrors1->Fill(float(fedId-fedId0),float(fedChannel));
	    //hfed2DErrors1ls->Fill(float(lumiBlock),float(fedId-fedId0)); //errors
	    break; }

	  }  // end switch
	  
	  if(status<20) errorType[status]++;

          if (plotRoc > 24) {
            eFEDType[fedId-fedId0]->Fill(plotChannel,status);
            sumType->Fill((fedId-fedId0),status);
            //sumType->Fill((fedId-fedId0),(plotRoc-22));
          }

	  //herrorType0->Fill(float(status));
	  //herrorFed0->Fill(float(fedId));
	  //herrorChan0->Fill(float(fedChannel));

	  //hfed2d->Fill(float(fedId-fedId0),float(status)); // fed # versus type
	  
	  if(status>=10) {  // hard errors
	    // Type - 1 Errors

	    countErrorsInFed1++;
	    //hfedErrorType1ls->Fill(float(lumiBlock),float(fedId-fedId0)); // hard errors
	    //hfed2DErrorsType1->Fill(float(fedId-fedId0),float(fedChannel));

	    //herrorType1->Fill(float(status));
	    //herrorType1Fed->Fill(float(fedId-fedId0));
	    //herrorType1Chan->Fill(float(fedChannel));

	    fedErrors[fedId-fedId0][(fedChannel-1)]++;

	  } else if(status>0) {  // decode errors
	    // Type 2 errprs

	    countErrorsInFed2++;
	    //hfedErrorType2ls->Fill(float(lumiBlock),float(fedId-fedId0)); // decode errors
	    //hfed2DErrorsType2->Fill(float(fedId-fedId0),float(fedChannel));

	    //herrorType2->Fill(float(status));
	    //herrorType2Fed->Fill(float(fedId-fedId0));
	    //herrorType2Chan->Fill(float(fedChannel));

	    if(status==5)      decodeErrors000[fedId-fedId0][(fedChannel-1)]++;
	    else if(status==6) decodeErrorsDouble[fedId-fedId0][(fedChannel-1)]++;
	    else               decodeErrors[fedId-fedId0][(fedChannel-1)]++;
	  }

	}
      } // for  1/2 word

    } // loop over longlong  words

    countTotErrors += countErrorsInFed;
    countErrorsPerEvent += countErrorsInFed;
    countErrorsPerEvent1 += countErrorsInFed1;
    countErrorsPerEvent2 += countErrorsInFed2;

    //convert data to digi (dummy for the moment)
    //formatter.interpretRawData( dummyErrorBool, fedId, rawData, collection, errors);
    //cout<<dummyErrorBool<<" "<<digis.size()<<" "<<errors.size()<<endl;

    if(countPixelsInFed>0)  {
      sumFedPixels[fedId-fedId0] += countPixelsInFed;
    }

    //hpixels->Fill(float(countPixelsInFed));
    //hpixels0->Fill(float(countPixelsInFed));
    //if(countPixelsInFed>0) hpixels1->Fill(float(countPixelsInFed));
    //if(countPixelsInFed>0 && fedId<fedIdBpixMax_sup)  hpixels2->Fill(float(countPixelsInFed));
    //if(countPixelsInFed>0 && fedId>=fedIdBpixMax_sup) hpixels3->Fill(float(countPixelsInFed));
    //herrors->Fill(float(countErrorsInFed));

    for(int i=0;i<n_of_Channels;++i) { 
      //hfedchannelsize->Fill( float(fedId-fedId0), float(i+1), float(fedchannelsize[i]) );
      if(fedId<fedIdBpixMax) {
	//hfedchannelsizeb->Fill( float(fedchannelsize[i]) );
	int layer = MyDecode::checkLayerLink(fedId, i); // get bpix layer 
	if(layer>10) layer = layer-10; // ignore 1/2 modules 
	//if(layer==3)      hfedchannelsizeb3->Fill( float(fedchannelsize[i]) );  // layer 3
	//else if(layer==2) hfedchannelsizeb2->Fill( float(fedchannelsize[i]) );  // layer 2
	//else if(layer==1) hfedchannelsizeb1->Fill( float(fedchannelsize[i]) );  // layer 1
	// else cout<<" Cannot be "<<layer<<" "<<fedId<<" "<<i<<endl;
      } //else         hfedchannelsizef->Fill( float(fedchannelsize[i]) );  // fpix
    }
    //    if(fedId == fedIds.first || countPixelsInFed>0 || countErrorsInFed>0 )  {
    //       eventId = MyDecode::header(*header, true);
    //       if(countPixelsInFed>0 || countErrorsInFed>0 ) cout<<"fed "<<fedId<<" pix "<<countPixelsInFed<<" err "<<countErrorsInFed<<endl;
    //       status = MyDecode::trailer(*trailer,true);
    //     }
   
    //Look over channels
    for (int i = 0; i < n_of_Channels; i++) {
      //hitsInChannel[i] = (hitsInChannel[i] + 2) * 4;
      hFEDChannel[fedId - fedId0]->Fill(i,hitsInChannel[i]);
      //hitsInFed[fedId - fedId0] = hitsInFed[fedId - fedId0] + hitsInChannel[i];
      hitsInChannel[i] = 0;
      eFEDChannel[fedId - fedId0]->Fill(i,errInChannel[i]);
      errInChannel[i] = 0;
    }

    //cout << "Fed " << (fedId - fedId0) << " had " << hitsInFed[fedId - fedId0] << " hits" << endl; 

#ifdef OUTFILE
    // print number of bytes per fed, CSV
    if(fedId == fedIds.second) outfile<<(nWords*8)<<endl;
    else                       outfile<<(nWords*8)<<",";  
#endif

  } // loop over feds

  int unusedFED[32] = {10,11,22,23,34,35,46,47,58,59,70,71,82,83,94,95,103,104,105,106,107,115,116,117,118,119,127,128,129,130,131,21};

  for (int i=fedIds.first;i<(fedIds.second+1);i++) {
    //hFED[i - fedId0]->Fill(double(hitsInFed[i - fedId0]));
    //hitsInFed[i - fedId0] = 0;
    //eFED[i - fedId0]->Fill(double(errInFed[i - fedId0]));
    
    if (find(begin(unusedFED), end(unusedFED), (i-fedId0)) == end(unusedFED)){
      //hitsInFed[i-fedId0] = (hitsInFed[i-fedId0]+2)*4; 
      //sumhFED->Fill((i - fedId0),double(hitsInFed[i - fedId0]));
      sumeFED->Fill((i - fedId0),double(errInFed[i - fedId0]));
      sumhFED->Fill((i-fedId0),double(FEDsize[i-fedId0]));
    }
    
    //hitsInFed[i - fedId0] = 0;
    errInFed[i - fedId0] = 0;
    FEDsize[i-fedId0]=0;
  }

  //htotPixels->Fill(float(countPixels));
  //htotPixels0->Fill(float(countPixels));
  if(wrongBX && printBX && !phase1) {
    cout<<" Inconsistent BX: for event "<<event<<" (fed-header event "<<eventId<<") for LS "<<lumiBlock
	<<" for run "<<run<<" for bx "<<bx<<" pix= "<<countPixels<<endl;
    //htotPixels2->Fill(float(countPixels));
    //hbx2->Fill(float(bx));
  }
  //htotErrors->Fill(float(countErrorsPerEvent));

  //htotPixelsls->Fill(float(lumiBlock),float(countPixels));
  //htotPixelsbx->Fill(float(bx),float(countPixels));

  //herrorType1ls->Fill(float(lumiBlock),float(countErrorsPerEvent1));
  //herrorType2ls->Fill(float(lumiBlock),float(countErrorsPerEvent2));

  //herror1ls->Fill(float(lumiBlock),float(countErrors[1]));
  //herror3ls->Fill(float(lumiBlock),float(countErrors[3]));
  //herror4ls->Fill(float(lumiBlock),float(countErrors[4]));
  //herror5ls->Fill(float(lumiBlock),float(countErrors[5]));
  //herror6ls->Fill(float(lumiBlock),float(countErrors[6]));
  //herror10ls->Fill(float(lumiBlock),float(countErrors[10]));
  //herror11ls->Fill(float(lumiBlock),float(countErrors[11]));
  //herror12ls->Fill(float(lumiBlock),float(countErrors[12]));
  //herror13ls->Fill(float(lumiBlock),float(countErrors[13]));
  //herror14ls->Fill(float(lumiBlock),float(countErrors[14]));
  //herror15ls->Fill(float(lumiBlock),float(countErrors[15]));
  //herror16ls->Fill(float(lumiBlock),float(countErrors[16]));


  //herrorType1bx->Fill(float(bx),float(countErrorsPerEvent1));
  //herrorType2bx->Fill(float(bx),float(countErrorsPerEvent2));

  //aveFedSize /= 32.;
  //hsize3->Fill(aveFedSize);
  ////hsize2dls->Fill(float(lumiBlock),aveFedSize);

  ////havsizels->Fill(float(lumiBlock),aveFedSize);
  //havsizebx->Fill(float(bx),aveFedSize);

  if(countPixels>0) {
    //hlumi->Fill(float(lumiBlock));
    //hbx->Fill(float(bx));
    //htotPixels1->Fill(float(countPixels));

    //cout<<"EVENT: "<<countEvents<<" "<<eventId<<" pixels "<<countPixels<<" errors "<<countTotErrors<<endl;
    sumPixels += countPixels;
    countEvents++;
    //int dummy=0;
    //cout<<" : ";
    //cin>>dummy;
  }  // end if

 
} // end analyze

// 2 - wrong channel
// 4 - wrong roc
// 3 - wrong pix or dcol 
// 5 - pix=0
// 6 - double pix

  // 10 - timeout ()
  // 11 - ene ()
  // 12 - mum pf rocs error ()
  // 13 - fsm ()
  // 14 - overflow ()
  // 15 - trailer ()
  // 16 - fifo  (30)
  // 17 - reset/resync NOT INCLUDED YET

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(SiPixelRawDump);
