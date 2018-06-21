// PixelEncoder
// Anthony McIntyre - June 2018
// Using pixel data stored in a root TTree;
// finds the fed id with the highest avg hits,
// encodes pixel addresses into a binary format
// for FED testing.
//
// It might be possible to merge this code into
// a CMSSW framework analyzer program.
//
// Format:
// 	for hits per roc 			for pixel addresses
// 	SRAMhit#.bin 				SRAMpix#.bin
// 	 32 bit blocks				  32 bit blocks
//   	  0x[roc1][roc2]...				  0x[25:16][13:8][7:0]
// 	  layer 1:					  0x[row][col][adc]
//	    2 rocs per 32bits
//	  layer 2:
//	    4 rocs per 32bits
//	  layer 3-4 and FPix
//      8 rocs per 32bits

#ifndef ENCODER_H
#define ENCODER_H
#include <time.h>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <TF2.h>
#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2F.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TROOT.h>
#include <TStyle.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>

// Container types
// row, col as key, adc as value
//    row, col is bit shifted
//	  row is shifted 16, col is shifted 8
using Pixels = std::unordered_map<uint32_t, uint32_t>;
// roc id as key and row map as value
using ROCs = std::unordered_map<int, Pixels>;
// channel id as key and roc map
using Chans = std::unordered_map<int, ROCs>;
// layer number as key, chans map as value
//     0 is unknown, 1-4 is BPix, 5+ is FPix
using Layers = std::unordered_map<int, Chans>;
// fed id as key and channel map
using FEDs = std::unordered_map<int, Layers>;
// event id as key and fed map for list of feds
using Events = std::unordered_map<int, FEDs>;

class Pixel_Store {
  // Multiple Pixel Storage Class
  // stores pixels in a nested map structure
 public:
  Events storage;  // main storage
 public:
  Pixel_Store() {}           // constructor
  virtual ~Pixel_Store() {}  // destructor

  // adds a pixel to class
  int add(int event,
          int fed,
          int layer,
          int ch,
          int roc,
          uint32_t row,
          uint32_t col,
          uint32_t adc);
  // checks if pixel is already stored
  bool check(int event, int fed, int layer, int ch, int roc, uint32_t rowcol);

  // returns number of elements in each level of container
  // if total argument is true, returns total count of element
  // if total argument is false, returns count of element at location in
  // container
  int Get_EventCount() { return storage.size(); }
  int Get_FEDCount(bool total = true, int EventID = 0);
  int Get_ChanCount(bool total = true, int EventID = 0, int FEDID = 0);
  int Get_ROCCount(bool total = true,
                   int EventID = 0,
                   int FEDID = 0,
                   int layer = 0,
                   int ChanID = 0);
  int Get_PixelCount(bool total = true,
                     int EventID = 0,
                     int FEDID = 0,
                     int layer = 0,
                     int ChanID = 0,
                     int ROCID = 0);

  // returns hits in fed.
  // if total = true, then return total hits for all events
  // if total = false, then return hits in fed for event id
  int Get_FEDhits(int FEDID, bool total = true, int eventID = 0);
  // returns roc with highest hits in 1 event for fedid
  // the vector index 0 is ch id, 1 is roc id, 2 is hit count
  std::vector<int> Get_RocHits(int FEDID);
  // returns id of fed with highest avg hits
  int Get_FEDhighest();
};

// outputs 12 binary files of "hits per roc" and pixel addresses
// for the fed during all events in data file. Half of the files
// are looped to the max file size. The other half are not looped.
void encoder(int targetFED, Events& events);
void PixelPacker(TFile* f);
int main1(int argc, char* argv[]);
#endif