#Batch Job Scripti
set CMSSW_PROJECT_SRC="scratch0/CMSSW_9_2_0_patch4/src/"
set CFG_FILE="DPGAnalysis-SiPixelTools/HitAnalyzer/test/runRawDumperPilot_stream.py"
#set OUTPUT_FILE="Analyzer_Output.root"
set TOP="$PWD"

cd /afs/cern.ch/user/n/nimenend/$CMSSW_PROJECT_SRC
eval `scramv1 runtime -csh`
cd $TOP
#cd DPGAnalysis-SiPixelTools/HitAnalyzer/test/
cmsRun /afs/cern.ch/user/n/nimenend/$CMSSW_PROJECT_SRC/$CFG_FILE
#cmsRun $CFG_FILE

