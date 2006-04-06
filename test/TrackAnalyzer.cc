#include <memory>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/Framework/interface/ESHandle.h"
//#include "FWCore/Framework/interface/EventSetup.h"

// #include "TrackingTools/TransientRecHit/interface/TransientRecHit.h"
// #include "TrackingTools/TransientRecHit/interface/TransientRecHitBuilder.h"
// #include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPos.h"
// #include "TrackingTools/TransientTrackerRecHit2D/interface/TSiStripRecHit2DLocalPos.h"
// #include "DataFormats/TrackerRecHit2D/interface/SiStripRecHit2DLocalPosCollection.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"

#include <iostream>
#include <string>

using namespace edm;

class TrackAnalyzer : public edm::EDAnalyzer {
 public:
  TrackAnalyzer(const edm::ParameterSet& pset) {}

  ~TrackAnalyzer(){}

  virtual void analyze(const edm::Event& event, const edm::EventSetup& setup){

    //
    // extract tracker geometry
    //
    edm::ESHandle<TrackerGeometry> theG;
    setup.get<TrackerDigiGeometryRecord>().get(theG);

    using namespace std;

    std::cout << "\nEvent ID = "<< event.id() << std::endl ;

    edm::Handle<reco::TrackCollection> trackCollection;
    //    event.getByLabel("trackp", trackCollection);
    event.getByType(trackCollection);
    
    const reco::TrackCollection tC = *(trackCollection.product());

    std::cout << "Reconstructed "<< tC.size() << " tracks" << std::endl ;

    int i=1;
    for (reco::TrackCollection::const_iterator track=tC.begin(); track!=tC.end(); track++){
      std::cout << "Track number "<< i << std::endl ;
      std::cout << "\tmomentum: " << track->momentum()<< std::endl;
      std::cout << "\tPT: " << track->pt()<< std::endl;
      std::cout << "\tvertex: " << track->vertex()<< std::endl;
      std::cout << "\timpact parameter: " << track->d0()<< std::endl;
      std::cout << "\tcharge: " << track->charge()<< std::endl;
      std::cout << "\tnormalizedChi2: " << track->normalizedChi2()<< std::endl;
      i++;
      cout<<"\tFrom EXTRA : "<<endl;
      cout<<"\t\touter PT "<< track->outerPt()<<endl;
      //
      // try and access Hits
      //
      cout <<"\t\tNumber of RecHits "<<track->recHitsSize()<<endl;
      for (trackingRecHit_iterator it = track->recHitsBegin();  it != track->recHitsEnd(); it++){
	cout <<"\t\t\tRecHit on det "<<(*it)->geographicalId().rawId()<<endl;
	cout <<"\t\t\tRecHit in LP "<<(*it)->localPosition()<<endl;
	cout <<"\t\t\tRecHit in GP "<<theG->idToDet((*it)->geographicalId())->surface().toGlobal((*it)->localPosition()) <<endl;
	
      }
    }
  }
};

DEFINE_SEAL_MODULE();
DEFINE_ANOTHER_FWK_MODULE(TrackAnalyzer);

