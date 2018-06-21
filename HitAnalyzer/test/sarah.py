import FWCore.ParameterSet.Config as cms

process = cms.Process("sample")

import HLTrigger.HLTfilters.hltHighLevel_cfi as hlt
process.hltfilter = hlt.hltHighLevel.clone(
    HLTPaths = ['HLT_ZeroBias*'],
   andOr = True,  # False = and, True=or
    throw = False
    )

process.MessageLogger = cms.Service("MessageLogger",
    debugModules = cms.untracked.vstring('dumper'),
    destinations = cms.untracked.vstring('cout'),
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING')
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)
process.TFileService = cms.Service("TFileService",
    fileName = cms.string('raw.root')
)
process.source = cms.Source("NewEventStreamFileReader",
	#Change the input source right here
	#This work with RAW data file that has extension .dat
	#But not with root files
	#The data.dat file has been moved to the current folder
    fileNames = cms.untracked.vstring("file:./data.dat")
   )
process.output = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RAW'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(1048576),
    fileName = cms.untracked.string('t.root'),
    outputCommands = cms.untracked.vstring("keep *_*_*_*"),
    splitLevel = cms.untracked.int32(0)
)

process.d = cms.EDAnalyzer("SiPixelRawDump", 
    Timing = cms.untracked.bool(False),
    IncludeErrors = cms.untracked.bool(True),
    InputLabel = cms.untracked.string('rawDataCollector'),
   CheckPixelOrder = cms.untracked.bool(False),
    Verbosity = cms.untracked.int32(0),
    PrintThreshold = cms.untracked.double(0.001)
)
process.p = cms.Path(process.d)
process.ep = cms.EndPath(process.output)
