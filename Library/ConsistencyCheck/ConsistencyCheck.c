//------------------------------------------------------------------------------
//@Author	Huan Tran
//@Purpose	Read distribution binary files and produce hit-per-channel plot
//@Version	1.0
//@Limited	Only use for FPIX FED
//------------------------------------------------------------------------------
#include <iostream>			//Input output
#include <cstdint>			//uint32_t uint64_t
#include <fstream>			//Reading file
#include <string>			//Use string
#include <vector>			//Use vector as buffer
#include <numeric>			//Count sum
#include <cstdlib>			//Random number
#include <TROOT.h>			//Using root
#include <TH2D.h>			//Using TH2D plot in root			
#include <TCanvas.h>		//Using Canvas to print out the plot

#define NUMB_CHANNEL_PER_FILE 16
#define NUMB_PARTITION_PER_FILE 4
#define NUMB_CHANNEL_PER_PARTITION 4

using HitCount = uint16_t;
using SumHit = unsigned long int;

unsigned short const MODE_MASK= 3;
unsigned int const WORD_MASK=0xffff;
unsigned int const BYTE_MASK=0xff;
unsigned int const NIBBLE_MASK=0x0f;

const std::string BIN_PREFIX="SRAM";
const std::string HIT="hit";
const std::string EXT=".bin";

int decodeFile(std::ifstream& f,std::vector<HitCount> *hits,std::vector<SumHit>& sumHit);
int getRand(int be,int en);

int main(int argc, char* argv[]){
	//Create canvas
	TCanvas *canvas = new TCanvas("canvas");
	//Create the plot and set up its represetation
	TH2D p("plot","Hit per channel",48,1.,49.,500,-0.5,509.5);
	p.SetOption("COLZ");
	p.GetXaxis()->SetTitle("Channels");
	p.GetYaxis()->SetTitle("Number of Hit");
	//Loop and open each file
	for(int i=0;i<3;i++){
		//file name (default has prefix SRAM)
		std::string s="SRAM";
		//If bin files have different prefix, use it
		if(argc > 1){
			s=std::string(argv[1]);
			std::cout<<"File name prefix is "<<s<<std::endl;
		}
		//Bin files fstream
		std::ifstream f;
		std::cout<<"Opening file "<<s+HIT+std::to_string(i)+EXT<<std::endl;
		f.open(s+HIT+std::to_string(i)+EXT,std::ios::binary | std::ios::in);
		//Check for file openning errors
		if(!f.is_open()){
			std::cout<<"Error opening file "<<s+HIT+std::to_string(i)+EXT<<std::endl;
			return -1;
		}
		std::vector<HitCount> hits[NUMB_CHANNEL_PER_FILE];
		std::vector<SumHit> sumHit(NUMB_CHANNEL_PER_FILE,0);
		//Decode the file
		std::cout<<"Decodeing the file part"<<i<<std::endl;
		decodeFile(f,hits,sumHit);
		//Fill the histogram
		std::cout<<"Filling the histogram with new data"<<std::endl;
		for(char j=0;j<16;j++){
			std::cout<<"Total hit distribution channel "<<16*i+(j+1)<<": "<<hits[j].size()<<std::endl;
			std::cout<<"Total hit channel "<<16*i+(j+1)<<": "<<sumHit[j]<<std::endl;
			for(int m=0;m<hits[j].size();m++)
				p.Fill(16*i+(j+1),hits[j][m]);
		}
		f.close();
	}
	std::cout<<"Creating pdf file"<<std::endl;
	canvas->Print("DecodeBinHitDistribution.pdf[");
	p.Draw();
	canvas->Print("DecodeBinHitDistribution.pdf","Hit Per Channel Decoded from binary file");
	canvas->Print("DecodeBinHitDistribution.pdf]");
	std::cout<<"Program finished"<<std::endl;
	return 0;
}
int decodeFile(std::ifstream& f,std::vector<HitCount> *hits,std::vector<SumHit>& sumHit){
	//Buffer to read header byte and save format of each partition
	char header[NUMB_PARTITION_PER_FILE];
	char headerByte;
	int byteRead=0,fileSize;
	f.seekg(0,std::ios::end);
	fileSize=f.tellg();
	f.seekg(std::ios::beg);
	//Buffer to save hit distribution; 16 channels per file
	std::vector<char> hitDist[NUMB_CHANNEL_PER_FILE];
	//Get the header byte
	f.read(&headerByte,1);
	byteRead++;
	//Decode header
	for(unsigned char i=0;i<NUMB_PARTITION_PER_FILE;i++){
		header[i]=MODE_MASK&(headerByte>>(i*2));
		std::cout<<"Partition "<<int(i)<<" mode: "<<(int)header[i]<<std::endl;
	}
	//Decode the hit distribution
	int channel;
	for(unsigned char j=0;j<NUMB_PARTITION_PER_FILE;j++){
		char data=0;
		for(int i=0;i<1048576;i++){
			//Pick a random channel in four channels of this partition
			channel=getRand(4*j,4*j+NUMB_CHANNEL_PER_PARTITION);
			//Decode according to the header byte format
			switch(header[j]){
				case 0://2 rocs, 32 bit
					{
						//First Roc
						f.read(&data,1);
						byteRead++;
						HitCount tmp=data;
						f.read(&data,1);
						byteRead++;
						tmp=(tmp<<8)&data;
						hits[channel].push_back(data);
						sumHit[channel]+=data;
						//increase word counter
						i++;
						//Second Roc
						f.read(&data,1);
						byteRead++;
						tmp=data;
						f.read(&data,1);
						byteRead++;
						tmp=(tmp<<8)&data;
						hits[channel].push_back(data);
						sumHit[channel]+=data;
					}
						break;
				case 1://4 rocs, 32 bit
					//First Roc
					{
						f.read(&data,1);
						byteRead++;
						hits[channel].push_back(data);
						//Second Roc
						f.read(&data,1);
						byteRead++;
						hits[channel].push_back(data);
						sumHit[channel]+=data;
						//increase word counter
						i++;
						//Third Roc
						f.read(&data,1);
						byteRead++;
						hits[channel].push_back(data);
						sumHit[channel]+=data;
						//Fourth Roc
						f.read(&data,1);
						hits[channel].push_back(data);
						sumHit[channel]+=data;
					}
						break;
				case 2://8rocs, 32 bit
					{
						uint8_t roc1,roc2;
						for(char k=0;k<4;k++){
							//Decode one byte
							f.read(&data,1);
							byteRead++;
							//First Roc in byte
							roc1=data&NIBBLE_MASK;
							//Second Roc in byte
							roc2=(data>>4)&NIBBLE_MASK;
							//Save to channel
							hits[channel].push_back(roc1);
							hits[channel].push_back(roc2);
							sumHit[channel]+=(roc1+roc2);
						}
						//Increase word counter
						i++;
					}
					break;
				default://8 rocs, 64 bit
					{
						for(char k=0;k<8;k++){
							//Decode one byte
							f.read(&data,1);
							byteRead++;
							hits[channel].push_back(data);
							sumHit[channel]+=data;
						}					
						i+=3;
					}
					break;
			}
		}
	}
	std::cout<<"Total byte read:"<<byteRead<<std::endl;
	if(byteRead!=fileSize){
		std::cout<<"File has not been read completely"<<std::endl;
	}else{
		std::cout<<"Whole file has been read"<<std::endl;
	}
	return 0;
}
//Return random number between be and en (not including en)
int getRand(int be,int en){
	if (be==en) return 0;
	if (be>en){
		int tmp=be;
		be=en;
		en=tmp;
	}
	return be+(rand()%(be-en));
}
