import FWCore.ParameterSet.Config as cms

demo = cms.EDAnalyzer('MCHitAnalyzer'
     ,tracks = cms.untracked.InputTag('ctfWithMaterialTracks')
)
