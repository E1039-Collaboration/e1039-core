#include "E906LegacyGen.h"
#include "E906VertexGen.h"

#include <fun4all/Fun4AllReturnCodes.h>

E906LegacyGen::E906LegacyGen(const std::string& name) 
    : SubsysReco(name)
{
    _vertexGen = new E906VertexGen();
    return;
}

E906LegacyGen::~E906LegacyGen()
{
    delete _vertexGen;
}

int E906LegacyGen::Init(PHCompositeNode* topNode)
{
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::InitRun(PHCompositeNode* topNode)
{
    
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::process_event(PHCompositeNode* topNode)
{
    _vertexGen->InitRun(topNode);
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::ResetEvent(PHCompositeNode* topNode)
{
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::End(PHCompositeNode* topNode)
{
    return Fun4AllReturnCodes::EVENT_OK;
}

