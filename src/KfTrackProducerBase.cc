#include "RecoTracker/TrackProducer/interface/KfTrackProducerBase.h"
// system include files
#include <memory>
// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "TrackingTools/GeomPropagators/interface/Propagator.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"

#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

void KfTrackProducerBase::putInEvt(edm::Event& evt,
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
    
    LogDebug("TrackProducer") << "In KfTrackProducerBase::putInEvt - seedDir=" << seedDir;

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

  LogTrace("TrackingRegressionTest") << "========== TrackProducer Info ===================";
  LogTrace("TrackingRegressionTest") << "number of finalTracks: " << selTracks->size();
  for (reco::TrackCollection::const_iterator it = selTracks->begin(); it != selTracks->end(); it++) {
    LogTrace("TrackingRegressionTest") << "track's n valid and invalid hit, chi2, pt : " 
				       << it->found() << " , " 
				       << it->lost()  <<" , " 
				       << it->normalizedChi2() << " , "
				       << it->pt();
  }
  LogTrace("TrackingRegressionTest") << "=================================================";
  
  
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