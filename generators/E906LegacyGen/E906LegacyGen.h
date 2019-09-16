#ifndef __E906LEGACYGEN_H__
#define __E906LEGACYGEN_H__

#include <string>

#include <TString.h>

#include <fun4all/SubsysReco.h>

class PHCompositeNode;
class E906VertexGen;

class E906LegacyGen : public SubsysReco
{
public:
    E906LegacyGen(const std::string& name = "E906LegacyGen");
    virtual ~E906LegacyGen();

    int Init(PHCompositeNode* topNode);
    int InitRun(PHCompositeNode* topNode);
    int process_event(PHCompositeNode* topNode);
    int ResetEvent(PHCompositeNode* topNode);
    int End(PHCompositeNode* topNode);

    //Configuraation of the vertex distribution
    void enableTarget() { _targetVtx = true; }
    void enableDump()   { _dumpVtx = true;   }
    void enableOthers() { _otherVtx = true;  }

private:
    E906VertexGen* _vertexGen;

    bool _targetVtx;
    bool _dumpVtx;
    bool _otherVtx;
};

#endif
