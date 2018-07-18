#ifndef MYDECODE_H
#define MYDECODE_H
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
#include "MyDecodeGlobalVariables.h"
using namespace std;

#define PHASE1

const bool printErrors=false;
const bool printData=false;  
const bool printHeaders=false; 
const bool printBX=false; 
const bool CHECK_PIXELS=true;
const bool PRINT_BASELINE=false;
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
//Decoding helper functions
class MyDecode {
	public:
		MyDecode();
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
#endif
