//#include "TInterpreter.h"
//#include "TCanvas.h"
//#include "TSystem.h"
//#include "TFile.h"
//#include "TH2.h"
//#include "TH1.h"
//#include "TNtuple.h"
//#include "TPaveLabel.h"
//#include "TPaveText.h"
//#include "TFrame.h"
//#include "TSystem.h"
//#include "TInterpreter.h"
//#include "TDirectoryFile.h"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

void test_macro() {
  TFile *f = new TFile("raw.root");
  TDirectory *d;// = gDirectory;
  d->cd();
  //.ls();
  TH2D * hFEDChannel1 = (TH2D*)d->Get("hFEDChannel1");
  hFEDChannel1->Draw();
}
