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
#process.MessageLogger = cms.Service("MessageLogger",
#    debugModules = cms.untracked.vstring('dumper'),
#    destinations = cms.untracked.vstring('cout'),
#    cout = cms.untracked.PSet(
#        threshold = cms.untracked.string('WARNING')
#    )
#)
# -1 means processing all events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
#Setting up a service to write out the root file containing histograms
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('decodedData.root')
)
#Setting up the input files
#Do not change the name of the attribute .source
#It is another 'unlableable' variable
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
#		"file:/storage0/data/mck4/HydJet_B0/step2_1.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_2.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_3.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_4.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_5.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_6.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_7.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_8.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_9.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_10.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_11.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_12.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_13.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_14.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_15.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_16.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_17.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_18.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_19.root",
#		"file:/storage0/data/mck4/HydJet_B0/step2_20.root"
															
		"file:/storage1/users/mck4/HydJet_MinBias/step2_1.root",
	)
)
#Setting up the analyzer
#SiPixelRawDump is the name of the class we need to use
#Check out the file SipixelRawDump
#SiPixelRawDump is the name of the class we need to use
#Check out the file SipixelRawDump.cc to see how the analyzer works 

process.an = cms.EDAnalyzer("MCHitAnalyzer",  #Name of the EDAnlyzer class
#    CablingMapLabel = cms.string(''),
	Timing = cms.untracked.bool(False),
    IncludeErrors = cms.untracked.bool(True),
    InputLabel = cms.untracked.string('rawDataCollector'),#the name of the module creating the object holding the FED raw data. This is to filter out the object we need from the events
    CheckPixelOrder = cms.untracked.bool(False),
    Verbosity = cms.untracked.int32(0),
    PrintThreshold = cms.untracked.double(0.001)
)

#Setting up the path that the cmsRun will iterate through for each event
process.p = cms.Path(process.an)
