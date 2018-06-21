//Run this file in root
//Required the decodedData.root in the same folder
//the decodedData.root can be obtained by running the Simulator-Run-Configfile.py in the python folder with cmsRun
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
class Pixel{
public:
	Pixel(Int_t eventID,Int_t fedID,Int_t channel,Int_t ROC,Int_t row,Int_t col,Int_t adc){
		_eventID=fedID;
		_eventID=fedID;
		_channel=channel;
		_ROC=ROC;
		_row=row;
		_col=col;
		_adc=adc;
	}
	Pixel(){};
	void fill(Int_t eventID, Int_t fedID,MyDecode d){
		_eventID=eventID;
		_fedID=fedID;
		_channel=d.get_channel();
		_ROC=d.get_roc();
		_row=d.get_row();
		_col=d.get_col();
		_adc=d.get_adc();
	}
	Int_t _eventID,_fedID,_channel,_ROC,_row,_col,_adc;
};
void readDecodedData(){
	TFile f("decodedData.root");
	TTree *t;
	f.GetObject("DecodedData",t);
	Pixel a;
	t->SetBranchAddress("Data",&a);
	t->GetEntry(0);
	cout<<"Total number of entries:"<<t->GetEntries()<<endl;
	cout<<"Col is "<<a._col<<endl;
	cout<<"Row is "<<a._row<<endl;
		}
