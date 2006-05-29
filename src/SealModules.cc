#include "FWCore/Framework/interface/MakerMacros.h"

#include "RecoTracker/TrackProducer/interface/TrackProducer.h"
#include "RecoTracker/TrackProducer/interface/TrackRefitter.h"

DEFINE_SEAL_MODULE();
DEFINE_ANOTHER_FWK_MODULE(TrackProducer)
DEFINE_ANOTHER_FWK_MODULE(TrackRefitter)