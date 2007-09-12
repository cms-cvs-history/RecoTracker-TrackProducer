#include "RecoTracker/TrackProducer/interface/TrackProducerBase.h"

/// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h" 
#include "TrackingTools/Records/interface/TrackingComponentsRecord.h" 
#include "TrackingTools/Records/interface/TransientRecHitRecord.h" 

#include "TrackingTools/PatternTools/interface/TrajectoryFitter.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"


//destructor
TrackProducerBase::~TrackProducerBase(){ }

// member functions
// ------------ method called to produce the data  ------------

void TrackProducerBase::getFromES(const edm::EventSetup& setup,
				  edm::ESHandle<TrackerGeometry>& theG,
				  edm::ESHandle<MagneticField>& theMF,
				  edm::ESHandle<TrajectoryFitter>& theFitter,
				  edm::ESHandle<Propagator>& thePropagator,
				  edm::ESHandle<TransientTrackingRecHitBuilder>& theBuilder)
{
  //
  //get geometry
  //
  LogDebug("TrackProducer") << "get geometry" << "\n";
  setup.get<TrackerDigiGeometryRecord>().get(theG);
  //
  //get magnetic field
  //
  LogDebug("TrackProducer") << "get magnetic field" << "\n";
  setup.get<IdealMagneticFieldRecord>().get(theMF);  
  //
  // get the fitter from the ES
  //
  LogDebug("TrackProducer") << "get the fitter from the ES" << "\n";
  std::string fitterName = conf_.getParameter<std::string>("Fitter");   
  setup.get<TrackingComponentsRecord>().get(fitterName,theFitter);
  //
  // get also the propagator
  //
  LogDebug("TrackProducer") << "get also the propagator" << "\n";
  std::string propagatorName = conf_.getParameter<std::string>("Propagator");   
  setup.get<TrackingComponentsRecord>().get(propagatorName,thePropagator);
  //
  // get the builder
  //
  LogDebug("TrackProducer") << "get also the TransientTrackingRecHitBuilder" << "\n";
  std::string builderName = conf_.getParameter<std::string>("TTRHBuilder");   
  setup.get<TransientRecHitRecord>().get(builderName,theBuilder);

  

}

void TrackProducerBase::getFromEvt(edm::Event& theEvent,edm::Handle<TrackCandidateCollection>& theTCCollection)
{
  //
  //get the TrackCandidateCollection from the event
  //
  LogDebug("TrackProducer") << 
    "get the TrackCandidateCollection from the event, source is " << src_<<"\n";
  if (pro_=="") {
    theEvent.getByLabel(src_,theTCCollection );  
  } else {
    theEvent.getByLabel(pro_,src_,theTCCollection );
  }
}

void TrackProducerBase::getFromEvt(edm::Event& theEvent,edm::Handle<reco::TrackCollection>& theTCollection)
{
  //
  //get the TrackCollection from the event
  //
  LogDebug("TrackProducer") << 
    "get the TrackCollection from the event, source is " << src_<<"\n";
  if (pro_=="") {
    theEvent.getByLabel(src_,theTCollection );  
  } else {
    theEvent.getByLabel(pro_,src_,theTCollection );
  }
}

void TrackProducerBase::putInEvt(edm::Event& evt,
				 std::auto_ptr<TrackingRecHitCollection>& selHits,
				 std::auto_ptr<reco::TrackCollection>& selTracks,
				 std::auto_ptr<reco::TrackExtraCollection>& selTrackExtras,
				 std::auto_ptr<std::vector<Trajectory> >&   selTrajectories,
				 AlgoProductCollection& algoResults)
{

  TrackingRecHitRefProd rHits = evt.getRefBeforePut<TrackingRecHitCollection>();
  reco::TrackExtraRefProd rTrackExtras = evt.getRefBeforePut<reco::TrackExtraCollection>();

  edm::Ref<reco::TrackExtraCollection>::key_type idx = 0;
  edm::Ref<reco::TrackExtraCollection>::key_type hidx = 0;
  edm::Ref<reco::TrackCollection>::key_type iTkRef = 0;
  edm::Ref< std::vector<Trajectory> >::key_type iTjRef = 0;
  std::map<unsigned int, unsigned int> tjTkMap;

  for(AlgoProductCollection::iterator i=algoResults.begin(); i!=algoResults.end();i++){
    Trajectory * theTraj = (*i).first;
    if(trajectoryInEvent_) {
      selTrajectories->push_back(*theTraj);
      iTjRef++;
    }
    const TrajectoryFitter::RecHitContainer& transHits = theTraj->recHits();

    reco::Track * theTrack = (*i).second.first;
    PropagationDirection seedDir = (*i).second.second;
    
    LogDebug("TrackProducer") << "In TrackProducerBase::putInEvt - seedDir=" << seedDir;

    reco::Track t = * theTrack;
    selTracks->push_back( t );
    iTkRef++;

    // Store indices in local map (starts at 0)
    if(trajectoryInEvent_) tjTkMap[iTjRef-1] = iTkRef-1;
    
    //sets the outermost and innermost TSOSs
    TrajectoryStateOnSurface outertsos;
    TrajectoryStateOnSurface innertsos;
    unsigned int innerId, outerId;
    if (theTraj->direction() == alongMomentum) {
      outertsos = theTraj->lastMeasurement().updatedState();
      innertsos = theTraj->firstMeasurement().updatedState();
      outerId = theTraj->lastMeasurement().recHit()->geographicalId().rawId();
      innerId = theTraj->firstMeasurement().recHit()->geographicalId().rawId();
    } else { 
      outertsos = theTraj->firstMeasurement().updatedState();
      innertsos = theTraj->lastMeasurement().updatedState();
      outerId = theTraj->firstMeasurement().recHit()->geographicalId().rawId();
      innerId = theTraj->lastMeasurement().recHit()->geographicalId().rawId();
   }
    //build the TrackExtra
    GlobalPoint v = outertsos.globalParameters().position();
    GlobalVector p = outertsos.globalParameters().momentum();
    math::XYZVector outmom( p.x(), p.y(), p.z() );
    math::XYZPoint  outpos( v.x(), v.y(), v.z() );
    v = innertsos.globalParameters().position();
    p = innertsos.globalParameters().momentum();
    math::XYZVector inmom( p.x(), p.y(), p.z() );
    math::XYZPoint  inpos( v.x(), v.y(), v.z() );

    reco::TrackExtraRef teref= reco::TrackExtraRef ( rTrackExtras, idx ++ );
    reco::Track & track = selTracks->back();
    track.setExtra( teref );
    selTrackExtras->push_back( reco::TrackExtra (outpos, outmom, true, inpos, inmom, true,
						 outertsos.curvilinearError(), outerId,
						 innertsos.curvilinearError(), innerId,seedDir));

    reco::TrackExtra & tx = selTrackExtras->back();
    size_t i = 0;
    for( TrajectoryFitter::RecHitContainer::const_iterator j = transHits.begin();
	 j != transHits.end(); j ++ ) {
      if ((**j).hit()!=0){
	TrackingRecHit * hit = (**j).hit()->clone();
	track.setHitPattern( * hit, i ++ );
	selHits->push_back( hit );
	tx.add( TrackingRecHitRef( rHits, hidx ++ ) );
      }
    }
    delete theTrack;
    delete theTraj;
  }

  LogTrace("TrackProducer") << "========== TrackProducer Info ===================";
  LogTrace("TrackProducer") << "number of finalTracks: " << selTracks->size();
  for (reco::TrackCollection::const_iterator it = selTracks->begin(); it != selTracks->end(); it++) {
    LogTrace("TrackProducer") << "track's n valid and invalid hit, chi2, pt : " 
				      << it->found() << " , " 
				      << it->lost()  <<" , " 
				      << it->normalizedChi2() << " , "
				      << it->pt();
  }
  LogTrace("TrackProducer") << "=================================================";
  
  rTracks_ = evt.put( selTracks );
  evt.put( selTrackExtras );
  evt.put( selHits );
  
  if(trajectoryInEvent_) {
    edm::OrphanHandle<std::vector<Trajectory> > rTrajs = evt.put(selTrajectories);

    // Now Create traj<->tracks association map
    std::auto_ptr<TrajTrackAssociationCollection> trajTrackMap( new TrajTrackAssociationCollection() );
    for ( std::map<unsigned int, unsigned int>::iterator i = tjTkMap.begin(); 
          i != tjTkMap.end(); i++ ) {
      edm::Ref<std::vector<Trajectory> > trajRef( rTrajs, (*i).first );
      edm::Ref<reco::TrackCollection>    tkRef( rTracks_, (*i).second );
      trajTrackMap->insert( edm::Ref<std::vector<Trajectory> >( rTrajs, (*i).first ),
                            edm::Ref<reco::TrackCollection>( rTracks_, (*i).second ) );
    }
    evt.put( trajTrackMap );
  }
}

