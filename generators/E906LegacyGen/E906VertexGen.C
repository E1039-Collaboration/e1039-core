#include "E906VertexGen.h"

#include <iostream>

#include <phgeom/PHGeomUtility.h>

E906VertexGen::E906VertexGen() 
{
    inited = false;
}
E906VertexGen::~E906VertexGen() {}

void E906VertexGen::InitRun(PHCompositeNode* topNode)
{
    TGeoManager* geoManager = PHGeomUtility::GetTGeoManager(topNode);
    traverse(geoManager->GetTopNode(), 0);
}

void E906VertexGen::traverse(TGeoNode* node, int level)
{
    if(node == NULL) return;
    for(int i = 0; i < level; ++i) std::cout << "-";
    std::cout << " " << node->GetVolume()->GetName() << std::endl;
    //node->GetVolume()->Print();

    for(int i = 0; i < node->GetNdaughters(); ++i)
    {
        traverse(node->GetDaughter(i), level+1);
    }
}
