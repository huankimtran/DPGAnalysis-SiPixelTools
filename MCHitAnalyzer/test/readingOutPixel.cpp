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
bool wrongBX=false;
const unsigned int plsmsk = 0xff;   // pulse height                                                                      
const unsigned int pxlmsk = 0xff00; // pixel index                                                                       
const unsigned int dclmsk = 0x1f0000;
for (int fedId = fedIds.first; fedId < (fedIds.second + 1); fedId++) {

	const FEDRawData& rawData = buffers->FEDData( fedId ); 
	const Word64* header = reinterpret_cast<const Word64* >(rawData.data()); 
//cout<<hex<<*header<<dec<<endl;

	int nWords = rawData.size()/sizeof(Word64);
	unsigned int bxid = 0;
	eventId = MyDecode::header(*header, fedId, printHeaders, bxid);
//if(fedId = fedIds.first) 
	if(bx != int(bxid) ) { 
		wrongBX=true;
		if(printErrors && printBX && !phase1) 
			cout<<" Inconsistent BX: for event "<<event<<" (fed-header event "<<eventId<<") for LS "<<lumiBlock
		<<" for run "<<run<<" for bx "<<bx<<" fed bx "<<bxid<<endl;
	}
	if(bx<0) bx=bxid;
	const Word64* trailer = reinterpret_cast<const Word64* >(rawData.data())+(nWords-1);
//cout<<hex<<*trailer<<dec<<endl;
	status = MyDecode::trailer(*trailer,fedId, printHeaders);
	int fedChannel = 0;
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
status = decode.data(w,fedChannel, fedId, stat1, stat2, printData);
}
}
}