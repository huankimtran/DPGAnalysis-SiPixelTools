#ifndef MyDecode_H
#define MyDecode_H
#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include <TF1.h>
#include <TH2F.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TStyle.h>

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
#endif
