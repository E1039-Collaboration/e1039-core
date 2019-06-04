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

class mTrkEveDisplay : public mPHEveModuleBase
{

 public:

  mTrkEveDisplay(boost::shared_ptr<PHEveDisplay>);
  ~mTrkEveDisplay();
  
  void init(PHCompositeNode* topNode);
  void init_run(PHCompositeNode* topNode);
  bool event(PHCompositeNode* topNode);
  void end(PHCompositeNode* topNode);
  
  void create_nodes(PHCompositeNode* topNode);
  void draw_tracks();
  bool pid_cut(int pid);
  void clear();
    
 private:

  boost::shared_ptr<PHEveDisplay> _evedisp;
  SQHitVector *_sqhit_col;

  TEveTrackPropagator* _prop;
  TEveTrackList* _reco_tracks;

  int verbosity; 
};

#endif // __MTRKEVEDISPLAY_H__
