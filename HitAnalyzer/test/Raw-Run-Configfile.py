"""
	@Author	:	Huan Kim Tran
	@Date	:	05/31/2018
	@Source	:	runRawDumperPilot_stream.py
"""
import FWCore.ParameterSet.Config as cms

#Creating a process named Mote-Carlos-Analyzer
#Do not change the name of the process variable 
#It is an 'unlablelable' variable
process = cms.Process("MonteCarlosAnalyzer")
#Configuring the output stream for the Analyzer
process.MessageLogger = cms.Service("MessageLogger",
    debugModules = cms.untracked.vstring('dumper'),
    destinations = cms.untracked.vstring('cout'),
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING')
    )
)
# -1 means processing all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(51)
)
#Setting up a service to write out the root file containing histograms
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('raw.root')
)
#Setting up the input files
#Do not change the name of the attribute .source
#It is another 'unlableable' variable
process.source = cms.Source("NewEventStreamFileReader",    fileNames = cms.untracked.vstring(
		"file:/home/hkt2/data.dat"
	)
)
#Setting up the analyzer
#SiPixelRawDump is the name of the class we need to use
#Check out the file SipixelRawDump
#SiPixelRawDump is the name of the class we need to use
#Check out the file SipixelRawDump.cc to see how the analyzer works 
process.an = cms.EDAnalyzer("SiPixelRawDump",  #Name of the EDAnlyzer class
    Timing = cms.untracked.bool(False),
    IncludeErrors = cms.untracked.bool(True),
    InputLabel = cms.untracked.string('rawDataCollector'),#the name of the module creating the object holding the FED raw data. This is to filter out the object we need from the events
    CheckPixelOrder = cms.untracked.bool(False),
    Verbosity = cms.untracked.int32(0),
    PrintThreshold = cms.untracked.double(0.001)
)
#Setting up the path that the cmsRun will iterate through for each event
process.p = cms.Path(process.an)
