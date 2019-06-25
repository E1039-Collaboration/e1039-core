#ifndef EvtFilter_H
#define EvtFilter_H

// ROOT
#include <TString.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <list>
#include <map>
#include <string>

class SQEvent;


class EvtFilter : public SubsysReco
{
public:

public:
    EvtFilter(const std::string &name = "EvtFilter");
    virtual ~EvtFilter();

#ifndef __CINT__
    int Init(PHCompositeNode *topNode);
#endif
    
    //! module initialization
    int InitRun(PHCompositeNode *topNode);
    
    //! event processing
    int process_event(PHCompositeNode *topNode);

    int End(PHCompositeNode *topNode);

  unsigned short get_trigger_req() const {
    return _trigger_req;
  }

  void set_trigger_req(unsigned short triggerReq) {
    _trigger_req = triggerReq;
  }

  int get_event_id_req() const {
    return _event_id_req;
  }

  void set_event_id_req(int eventIdReq) {
    _event_id_req = eventIdReq;
  }

private:

    int GetNodes(PHCompositeNode *topNode);

    size_t _event;

    unsigned short _trigger_req;
    int _event_id_req;


    SQEvent* _event_header;
};


#endif
