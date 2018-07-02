#ifndef PIXEL_H
#define PIXEL_H
//config Mydecode.h
#ifndef PHASE1
#define PHASE1
#endif
#include "TROOT.h"
#include "MyDecode.h"
class Pixel{
public:
	Pixel(Int_t eventID,Int_t fedID,Int_t layer,Int_t channel,Int_t ROC,Int_t row,Int_t col,Int_t adc);
	Pixel(){};
	void fill(Int_t eventID, Int_t fedID,Int_t layer,MyDecode d);
	void fillEmpty(Int_t eventID, Int_t fedID);
	
	Int_t _eventID,_fedID,_layer,_channel,_ROC,_row,_col,_adc;
};
#endif
