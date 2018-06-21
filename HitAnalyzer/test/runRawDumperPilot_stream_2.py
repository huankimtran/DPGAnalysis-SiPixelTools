#
import FWCore.ParameterSet.Config as cms

process = cms.Process("d")

import HLTrigger.HLTfilters.hltHighLevel_cfi as hlt
# accept if 'path_1' succeeds
process.hltfilter = hlt.hltHighLevel.clone(
# Min-Bias
#    HLTPaths = ['HLT_Physics_v*'],  # not in ZB stream
#    HLTPaths = ['DST_Physics_v*'], useless 
#    HLTPaths = ['HLT_Random_v*'],
    HLTPaths = ['HLT_ZeroBias*'],
#    HLTPaths = ['HLT_L1Tech54_ZeroBias*'],
# Commissioning:
#    HLTPaths = ['HLT_L1_Interbunch_BSC_v*'],
#    HLTPaths = ['HLT_L1_PreCollisions_v1'],
#    HLTPaths = ['HLT_BeamGas_BSC_v*'],
#    HLTPaths = ['HLT_BeamGas_HF_v*'],
# LumiPixels
#    HLTPaths = ['AlCa_LumiPixels_Random_v*'],
#    HLTPaths = ['AlCa_LumiPixels_ZeroBias_v*'],
#    HLTPaths = ['AlCa_LumiPixels_v*'],
    
# examples
#    HLTPaths = ['p*'],
#    HLTPaths = ['path_?'],
    andOr = True,  # False = and, True=or
    throw = False
    )

# process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.MessageLogger = cms.Service("MessageLogger",
    debugModules = cms.untracked.vstring('dumper'),
    destinations = cms.untracked.vstring('cout'),
#    destinations = cms.untracked.vstring("log","cout"),
    cout = cms.untracked.PSet(
        threshold = cms.untracked.string('WARNING')
    )
#    log = cms.untracked.PSet(
#        threshold = cms.untracked.string('DEBUG')
#    )
)
#process.MessageLogger.cerr.FwkReport.reportEvery = 1
#process.MessageLogger.cerr.threshold = 'Debug'

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(-1)
)

process.TFileService = cms.Service("TFileService",
    fileName = cms.string('raw_2.root')
)

#myfilelist = cms.untracked.vstring()
#myfilelist.extend([
#"/store/data/Run2016A/ZeroBias1/RAW/v1/000/271/056/00000/0097F016-0C09-E611-AA06-02163E011AE6.root",
#])

#process.source = cms.Source("PoolSource",
process.source = cms.Source("NewEventStreamFileReader",
#  noEventSort=cms.untracked.bool(False),
#  firstEvent=cms.untracked.uint32(1),

# fileNames =  myfilelist
    fileNames = cms.untracked.vstring(                          
# data 2016
#"/store/data/Run2016A/ZeroBias1/RAW/v1/000/271/056/00000/0097F016-0C09-E611-AA06-02163E011AE6.root",
#"root://eoscms//eos/cms/tier0/store/data/Commissioning2016/MinimumBias/RAW/v1/000/265/510/00000/02474E86-4BDC-E511-8222-02163E01364A.root",
# "root://eoscms//eos/cms/tier0/store/data/Run2015D/ZeroBias/RAW/v1/000/258/655/00000/",

# "/store/t0streamer/Minidaq/A/000/277/108/run277108_ls0001_streamA_StorageManager.dat",
# "/store/t0streamer/Minidaq/A/000/277/150/run277150_ls0001_streamA_StorageManager.dat",
# "/store/t0streamer/Minidaq/A/000/277/349/run277349_ls0001_streamA_StorageManager.dat",
# "/store/t0streamer/Data/Express/000/297/180/run297180_ls0003_streamExpress_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/291/run297291_ls0001_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/291/run297291_ls0002_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/291/run297291_ls0003_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/496/run297496_ls0001_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/496/run297496_ls0002_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/496/run297496_ls0003_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/496/run297496_ls0004_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/496/run297496_ls0005_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/605/run297605_ls0224_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/605/run297605_ls0225_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/605/run297605_ls0226_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/605/run297605_ls0227_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/605/run297605_ls0228_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/562/run297562_ls0075_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/562/run297562_ls0083_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/297/557/run297557_ls0029_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/297/557/run297557_ls0030_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/297/557/run297557_ls0031_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/297/557/run297557_ls0032_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/297/557/run297557_ls0033_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/723/run297723_ls0014_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/723/run297723_ls0015_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/723/run297723_ls0016_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/723/run297723_ls0017_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/723/run297723_ls0018_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/673/run297673_ls0001_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/673/run297673_ls0002_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/673/run297673_ls0003_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/673/run297673_ls0004_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/297/673/run297673_ls0005_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0083_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0084_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0085_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0086_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0087_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0088_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0089_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0090_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0091_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/653/run298653_ls0092_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0039_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0040_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0041_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0042_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0043_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0090_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0091_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0092_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0093_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0094_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0095_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0096_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0097_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0098_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0099_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0300_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0301_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0302_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0303_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/809/run298809_ls0304_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/298/853/run298853_ls0437_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/853/run298853_ls0437_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias3/000/298/853/run298853_ls0437_streamPhysicsZeroBias3_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/298/853/run298853_ls0438_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/298/853/run298853_ls0438_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias3/000/298/853/run298853_ls0438_streamPhysicsZeroBias3_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/299/593/run299593_ls0001_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/299/593/run299593_ls0001_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias3/000/299/593/run299593_ls0001_streamPhysicsZeroBias3_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/299/593/run299593_ls0002_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/299/593/run299593_ls0002_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias3/000/299/593/run299593_ls0002_streamPhysicsZeroBias3_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias1/000/299/593/run299593_ls0003_streamPhysicsZeroBias1_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias2/000/299/593/run299593_ls0003_streamPhysicsZeroBias2_StorageManager.dat",
# "/store/t0streamer/Data/PhysicsZeroBias3/000/299/593/run299593_ls0003_streamPhysicsZeroBias3_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias1/000/300/027/run300027_ls0002_streamPhysicsZeroBias1_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias2/000/300/027/run300027_ls0002_streamPhysicsZeroBias2_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias3/000/300/027/run300027_ls0002_streamPhysicsZeroBias3_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias4/000/300/027/run300027_ls0002_streamPhysicsZeroBias4_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias5/000/300/027/run300027_ls0002_streamPhysicsZeroBias5_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias1/000/300/027/run300027_ls0003_streamPhysicsZeroBias1_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias2/000/300/027/run300027_ls0003_streamPhysicsZeroBias2_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias3/000/300/027/run300027_ls0003_streamPhysicsZeroBias3_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias4/000/300/027/run300027_ls0003_streamPhysicsZeroBias4_StorageManager.dat",
"/store/t0streamer/Data/PhysicsZeroBias5/000/300/027/run300027_ls0003_streamPhysicsZeroBias5_StorageManager.dat",




   )
)

process.output = cms.OutputModule("PoolOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('RAW'),
        filterName = cms.untracked.string('')
    ),
    eventAutoFlushCompressedSize = cms.untracked.int32(1048576),
    fileName = cms.untracked.string('t.root'),
    outputCommands = cms.untracked.vstring("keep *_*_*_*"),
#    outputCommands = cms.untracked.vstring("drop *","keep *_siPixelRawData_*_*"),
    splitLevel = cms.untracked.int32(0)
)


#process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('257487:50-257487:9999')
#process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('273725:83-273725:9999')
#process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('274314:97-274314:9999')
#process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('275828:69-275828:9999')
#process.source.lumisToProcess = cms.untracked.VLuminosityBlockRange('277069:80-277069:9999')

process.d = cms.EDAnalyzer("SiPixelRawDump", 
    Timing = cms.untracked.bool(False),
    IncludeErrors = cms.untracked.bool(True),
#   In 2015 data, label = rawDataCollector, extension = _LHC                                
    InputLabel = cms.untracked.string('rawDataCollector'),
# for MC
#    InputLabel = cms.untracked.string('siPixelRawData'),
#   For PixelLumi stream                           
#    InputLabel = cms.untracked.string('hltFEDSelectorLumiPixels'),
# old
#    InputLabel = cms.untracked.string('siPixelRawData'),
#    InputLabel = cms.untracked.string('source'),
    CheckPixelOrder = cms.untracked.bool(False),
# 0 - nothing, 1 - error , 2- data, 3-headers, 4-hex
    Verbosity = cms.untracked.int32(0),
# threshold, print fed/channel num of errors if tot_errors > events * PrintThreshold, default 0,001 
    PrintThreshold = cms.untracked.double(0.001)
)

#process.p = cms.Path(process.hltfilter*process.d)
process.p = cms.Path(process.d)

#process.ep = cms.EndPath(process.out)


