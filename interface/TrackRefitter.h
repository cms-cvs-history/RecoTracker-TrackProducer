#ifndef TrackRefitter_h
#define TrackRefitter_h

/** \class TrackRefitter
 *  Refit Tracks: Produce Tracks from TrackCollection. It performs a new final fit on a TrackCollection.
 *
 *  $Date: 2007/03/26 10:13:49 $
 *  $Revision: 1.1 $
 *  \author cerati
 */

#include "RecoTracker/TrackProducer/interface/TrackProducerBase.h"

class TrackRefitter : public TrackProducerBase, public edm::EDProducer {
public:

  /// Constructor
  explicit TrackRefitter(const edm::ParameterSet& iConfig);

  /// Implementation of produce method
  virtual void produce(edm::Event&, const edm::EventSetup&);

private:
  TrackProducerAlgorithm theAlgo;

};

#endif
