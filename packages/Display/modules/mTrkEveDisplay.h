/*
        \file mTrkEveDisplay.h
        \author Sookhyun Lee
        \brief reconstructed charged tracks and their clusters
        \version $Revision: 1.1 $
        \date    $Date: 07/26/2016
*/

#ifndef __MTRKEVEDISPLAY_H__
#define __MTRKEVEDISPLAY_H__

#include "mPHEveModuleBase.h"
#include <boost/shared_ptr.hpp>
#include <set>

class TEveManager;
class TEveTrackPropagator;
class TEveElementList;
class TEveTrackList;
class PHEveDisplay;
class TH2F;

class SQHit;
class SQHitVector;
class SRecEvent;

class TEveQuadSet;
class GeomSvc;

#define NDETPLANES 63

class mTrkEveDisplay : public mPHEveModuleBase
{

 public:

  mTrkEveDisplay(boost::shared_ptr<PHEveDisplay>);
  ~mTrkEveDisplay();
  
  void init(PHCompositeNode* topNode);
  void init_run(PHCompositeNode* topNode);
  bool event(PHCompositeNode* topNode);
  void end(PHCompositeNode* topNode);
  
  void get_nodes(PHCompositeNode* topNode);
  /** Get a wire param from detector id and element id
   *  @param[in]   det,    elm input
   *  @param[out]  x, y    1 end point of the wire
   *  @param[out]  dx, dy  wire span in x and y
   */
  int hit_to_wire(const int det, const int elm, double & x, double & y, double &dx, double &dy);
  void draw_hits();
  void draw_tracks();
  bool pid_cut(int pid);
  void clear();
    
 private:

  boost::shared_ptr<PHEveDisplay> _evedisp;
  SQHitVector *_sqhit_col;
  SRecEvent *_recEvent;

  TEveTrackPropagator* _prop;
  TEveTrackList* _reco_tracks;
  TEveQuadSet* _hit_wires[NDETPLANES];

  GeomSvc* _geom_svc;

  int verbosity; 
};

#endif // __MTRKEVEDISPLAY_H__
