#ifndef PIXEL_H
#define PIXEL_H
#include "TROOT.h"
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
#endif


