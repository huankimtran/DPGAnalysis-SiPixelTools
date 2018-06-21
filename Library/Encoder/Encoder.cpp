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
// checks if pixel is already stored and adds to container if not
#include "Encoder.h"

int Pixel_Store::add(int event,
                     int fed,
                     int layer,
                     int ch,
                     int roc,
                     uint32_t row,
                     uint32_t col,
                     uint32_t adc) {
  // merge row and col into unique number by bit shifting them
  uint32_t rowcol = (row << 16 | col << 8);

  if (!check(event, fed, layer, ch, roc, rowcol)) {
    storage[event][fed][layer][ch][roc][rowcol] = adc;
    // return 0 for no duplicate counting
    return 0;
  } else
    // duplicate found, return 1
    return 1;
}

// checks if pixel is stored in container
bool Pixel_Store::check(int event,
                        int fed,
                        int layer,
                        int chan,
                        int roc,
                        uint32_t rowcol) {
  Events::iterator ev = storage.find(event);
  if (ev == storage.end())
    return false;
  FEDs::iterator fe = ev->second.find(fed);
  if (fe == ev->second.end())
    return false;
  Layers::iterator ly = fe->second.find(layer);
  if (ly == fe->second.end())
    return false;
  Chans::iterator ch = ly->second.find(chan);
  if (ch == ly->second.end())
    return false;
  ROCs::iterator rc = ch->second.find(roc);
  if (rc == ch->second.end())
    return false;
  Pixels::iterator pi = rc->second.find(rowcol);
  if (pi == rc->second.end())
    return false;
  return true;
}

// returns count of FEDs stored
// if total = true, returns total feds from all events
// if total = false, returns fed count from single event
int Pixel_Store::Get_FEDCount(bool total, int EventID) {
  int FEDCt = 0;
  if (total)
    for (auto const& event : storage)
      FEDCt += event.second.size();
  else
    FEDCt = storage[EventID].size();
  return FEDCt;
}

// returns count of channels stored
int Pixel_Store::Get_ChanCount(bool total, int EventID, int FEDID) {
  int ChCt = 0;
  if (total) {
    for (auto const& event : storage)
      for (auto const& fed : event.second)
        for (auto const& lay : fed.second)
          ChCt += lay.second.size();
  }

  else
    for (auto const& lay : storage[EventID][FEDID])
      ChCt += lay.second.size();

  return ChCt;
}

// returns count of rocs stored
int Pixel_Store::Get_ROCCount(bool total,
                              int EventID,
                              int FEDID,
                              int layer,
                              int ChanID) {
  int ROCCt = 0;
  if (total) {
    for (auto const& event : storage)
      for (auto const& fed : event.second)
        for (auto const& lay : fed.second)
          for (auto const& ch : lay.second)
            ROCCt += ch.second.size();
  }

  else
    ROCCt = storage[EventID][FEDID][layer][ChanID].size();

  return ROCCt;
}

// returns count of pixels stored
int Pixel_Store::Get_PixelCount(bool total,
                                int EventID,
                                int FEDID,
                                int layer,
                                int ChanID,
                                int ROCID) {
  int PixCt = 0;
  // if total true, return count of all pixels
  if (total) {
    for (auto const& event : storage)
      for (auto const& fed : event.second)
        for (auto const& ly : fed.second)
          for (auto const& ch : ly.second)
            for (auto const& roc : ch.second)
              PixCt += roc.second.size();
  }

  // if total false then return pixel count inside roc
  else
    PixCt = storage[EventID][FEDID][layer][ChanID][ROCID].size();

  return PixCt;
}

// returns total hits for all events found in a single fed
int Pixel_Store::Get_FEDhits(int FEDID, bool total, int eventID) {
  int hitNum = 0;
  if (total) {
    for (auto const& event : storage)
      for (auto const& fed : event.second)
        if (fed.first == FEDID)
          for (auto const& lay : fed.second)
            for (auto const& chan : lay.second)
              for (auto const& roc : chan.second)
                hitNum += roc.second.size();
  }

  // returns hits in fed for single event
  else {
    for (auto const& chan : storage[eventID][FEDID])
      for (auto const& lay : chan.second)
        for (auto const& roc : lay.second)
          hitNum += roc.second.size();
  }
  return hitNum;
}

// return fedid of fed with highest avg hits per event
int Pixel_Store::Get_FEDhighest() {
  int FEDID = 1200;
  int FEDCount = 1338;
  int EventCount = Get_EventCount();
  int hits = 0;
  int avg = 0;
  int highest_avg = 0;
  int highest_fed = 0;
  for (int i = FEDID; i < FEDCount; i++) {
    hits = Get_FEDhits(i);
    if (hits > 0) {
      avg = hits / EventCount;
      if (avg > highest_avg) {
        highest_avg = avg;
        highest_fed = i;
      }
    }
  }
  return highest_fed;
}

// return roc with highest hits for one event in fedid
std::vector<int> Pixel_Store::Get_RocHits(int FEDID) {
  std::vector<int> rochits;
  rochits.push_back(0);
  rochits.push_back(0);
  rochits.push_back(0);
  for (auto const& event : storage) {
    for (auto const& fed : event.second) {
      if (fed.first == FEDID) {
        for (auto const lay : fed.second) {
          for (auto const ch : lay.second) {
            for (auto const roc : ch.second) {
              if (roc.second.size() > (unsigned int)rochits[2]) {
                rochits[0] = ch.first;
                rochits[1] = roc.first;
                rochits[2] = roc.second.size();
              }
            }
          }
        }
      }
    }
  }
  return rochits;
}


// outputs 12 binary files of "hits per roc" and pixel addresses
// for the fed during all events in data file. Half of the files
// are looped to the max file size. The other half are not looped.
void encoder(int targetFED, Events& events) {
  std::string filename;
  std::ofstream glibhit[3];
  std::ofstream glibpix[3];

  std::unordered_map<int, uint32_t> hits;

  // These are buffers for writing the data to files
  // 1 for each block of the hit files. 4 blocks per file.
  std::vector<uint32_t> RocHits[12];
  std::vector<uint32_t> PixAdd[3];

  std::unordered_map<int, std::unordered_map<int, int>> chPlay;
  std::unordered_map<int, bool> maxOut;
  for(int i=MIN_LAYER;i<=MAX_LAYER;i++)
	  maxOut[i]=false;

  // convert data in map structure to binary format
  // and place in a buffer for file writing.
  
  for (auto const evt : events) {
    for (auto const fed : evt.second) {
      // check for target fed
      if (fed.first == targetFED) {
        for (auto const lay : fed.second) {
          for (auto const ch : lay.second) {
            chPlay[lay.first][ch.first] += 1;
            for (auto const roc : ch.second) {
              // get rocid and hits on roc
              hits[roc.first] = (uint32_t)(roc.second.size());
              if(roc.second.size()>15)
            	  maxOut[lay.first]=true;
              // convert pixel addresses
              for (auto const pix : roc.second) {
                uint32_t addressBuffer = 0;
                addressBuffer = (pix.first | pix.second);
                if (ch.first < 17)
                  PixAdd[0].push_back(addressBuffer);
                if ((ch.first < 33) && (ch.first > 16))
                  PixAdd[1].push_back(addressBuffer);
                if (ch.first > 32)
                  PixAdd[2].push_back(addressBuffer);
              }
            }
            // use rocid and hits on roc for conversion
            uint32_t hitBuffer = 0;
            // diiferent layers have differenct # of rocs
            switch (lay.first) {
              case 1:
                for (int rc = 1; rc < 3; rc++) {
                  if (hits.count(rc) > 0)
                    hitBuffer = (hitBuffer << 16 | hits[rc]);
                  else
                    hitBuffer <<= 16;
                }
                break;
              case 2:
                for (int rc = 1; rc < 5; rc++) {
                  if (hits.count(rc) > 0)
                    hitBuffer = (hitBuffer << 8 | hits[rc]);
                  else
                    hitBuffer <<= 8;
                }
                break;
              default:
                for (int rc = 1; rc < 9; rc++) {
                  if (hits.count(rc) > 0)
                    hitBuffer = (hitBuffer << 4 | hits[rc]);
                  else
                    hitBuffer <<= 4;
                }
                break;
            }
            hits.clear();
            //push hit buffer onto correct block of binary file.
            if (ch.first < 5)
              RocHits[0].push_back(hitBuffer);
            if ((ch.first > 4) && (ch.first < 9))
              RocHits[1].push_back(hitBuffer);
            if ((ch.first > 8) && (ch.first < 13))
              RocHits[2].push_back(hitBuffer);
            if ((ch.first > 12) && (ch.first < 17))
              RocHits[3].push_back(hitBuffer);
            if ((ch.first > 16) && (ch.first < 21))
              RocHits[4].push_back(hitBuffer);
            if ((ch.first > 20) && (ch.first < 25))
              RocHits[5].push_back(hitBuffer);
            if ((ch.first > 24) && (ch.first < 29))
              RocHits[6].push_back(hitBuffer);
            if ((ch.first > 28) && (ch.first < 33))
              RocHits[7].push_back(hitBuffer);
            if ((ch.first > 32) && (ch.first < 37))
              RocHits[8].push_back(hitBuffer);
            if ((ch.first > 36) && (ch.first < 41))
              RocHits[9].push_back(hitBuffer);
            if ((ch.first > 40) && (ch.first < 45))
              RocHits[10].push_back(hitBuffer);
            if (ch.first > 44)
              RocHits[11].push_back(hitBuffer);
          }
        }
      }
    }
  }

  // begin writing the files

  // These files have to be an exact file size.
  // So it loops over the data until the file size is met.
  // The size in this case is 2^21 32bit blocks or around 8.39 MB
  for (int i = 0; i < 3; i++) {
    filename = "SRAMhit" + std::to_string(i) + ".bin";
    glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    filename = "SRAMpix" + std::to_string(i) + ".bin";
    glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    int count = 0;

    // write the SRAMhit files
    // The SRAMhit files are divided into 4 blocks
    for (int j = 0; j < 4; j++) {
      count = 0;
      for (int k = 0; k < 524288; k++) {
        if ((unsigned int)count >= RocHits[j + (i * 4)].size())
          count = 0;
        glibhit[i].write((char*)&RocHits[j + (i * 4)][count], 4);
        count++;
      }
    }
    // write the SRAMpix files
    count = 0;
    for (int j = 0; j < 2097152; j++) {
      if ((unsigned int)count >= PixAdd[i].size())
        count = 0;
      glibpix[i].write((char*)&PixAdd[i][count], 4);
      count++;
    }

    glibhit[i].close();
    glibpix[i].close();
  }

  // this loop is for the non looped files
  // these files are only constrained to a max file size of around 8.4 MB
  for (int i = 0; i < 3; i++) {
    filename = "SRAMhit" + std::to_string(i) + "_noloop.bin";
    glibhit[i].open(filename.c_str(), std::ios::binary | std::ios::out);
    filename = "SRAMpix" + std::to_string(i) + "_noloop.bin";
    glibpix[i].open(filename.c_str(), std::ios::binary | std::ios::out);

    // SRAMhit files
    for (int j = 0; j < 4; j++) {
      for (unsigned int k = 0; k < RocHits[j + (i * 4)].size(); k++) {
        glibhit[i].write((char*)&RocHits[j + (i * 4)][k], 4);
      }
    }
    // SRAMpix files
    for (unsigned int j = 0; j < PixAdd[i].size(); j++) {
      glibpix[i].write((char*)&PixAdd[i][j], 4);
    }

    glibhit[i].close();
    glibpix[i].close();
  }
}

void PixelPacker(TFile* f) {
  // the real main function

  // clock to record process time
  clock_t t1, t2, st1, st2, et1, et2;
  t1 = clock();

  // decode data from TTree
  TTreeReader reader("HighFedData", f);
  TTreeReaderValue<int> event(reader, "Data._eventID");
  TTreeReaderValue<int> fed(reader, "Data._fedID");
  TTreeReaderValue<int> layer(reader, "Data._layer");
  TTreeReaderValue<int> chan(reader, "Data._channel");
  TTreeReaderValue<int> roc(reader, "Data._ROC");
  TTreeReaderValue<int> row(reader, "Data._row");
  TTreeReaderValue<int> col(reader, "Data._col");
  TTreeReaderValue<int> adc(reader, "Data._adc");

  // declare output file to store output information
  std::ofstream outputFile;
  outputFile.open("output.txt");

  Pixel_Store pStore;

  st1 = clock();
  std::cout << "\nStoring pixels...\n";
  // stores duplicate pixel amount
  int duplicates = 0;
  // loop through TTree and store data in Pixel_Store
  while (reader.Next()) {
    duplicates += pStore.add(*event, *fed, *layer, *chan, *roc, (uint32_t)*row,
                             (uint32_t)*col, (uint32_t)*adc);
  }
  st2 = clock();
  std::cout << "Done storing pixels. Store time of "
            << (((float)st2 - (float)st1) / CLOCKS_PER_SEC)
            << " seconds.\nProcessing Pixels...\n\n";

  // get total events
  int eventCt = pStore.Get_EventCount();
  // get fedid with highest average hits per event
  int highest_FEDId = pStore.Get_FEDhighest();
  // get total hits for fed with highest avg hits
  int hitCtFED = pStore.Get_FEDhits(highest_FEDId);
  // get total hits stored
  int hitCtTotal = pStore.Get_PixelCount();
  std::vector<int> rochits = pStore.Get_RocHits(highest_FEDId);
  // output is stored in a string to print both to a file and terminal
  std::string output;
  output = "Total duplicate pixels: " + std::to_string(duplicates) +
           "\nTotal events: " + std::to_string(eventCt) +
           "\nTotal hits: " + std::to_string(hitCtTotal) +
           "\n\nHighest Avg Hit FED Id: " + std::to_string(highest_FEDId) +
           "\nWith an avg hit count of: " + std::to_string(hitCtFED / eventCt) +
					 "\n\nRoc with highest hits for single event in FED: ch " +
           std::to_string(rochits[0]) + " roc " + std::to_string(rochits[1]) +
           "\nWith a hit count of: " + std::to_string(rochits[2]);

  std::cout << output;   // print to terminal
  outputFile << output;  // print to file

  et1 = clock();
  std::cout << "\n\nEncoding binary files...\n";
  encoder(highest_FEDId, pStore.storage);
  et2 = clock();
  std::cout << "Done encoding with an encoding time of "
            << (((float)et2 - (float)et1) / CLOCKS_PER_SEC) << " seconds.";

  // output process time in seconds
  t2 = clock();
  float seconds = ((float)t2 - (float)t1) / CLOCKS_PER_SEC;
  std::cout << "\n\nDone!\nProgram runtime: " << seconds << " seconds.\n\n";
}

int main1(int argc, char* argv[]) {
  // takes file argument and checks it before passing to main function
  if (argc != 2) {  // if no arguments
    std::cout << "usage: " << argv[0] << " <filename>\n";
  } else {
    TFile* file = new TFile(argv[1]);
    // check if file loaded correctly
    if (file->IsOpen()) {
      std::cout << "Program start.\n";
      // pass file to main function
      PixelPacker(file);
    } else
      std::cout << "Couln't open file.\n";
  }
  return 1;
}
