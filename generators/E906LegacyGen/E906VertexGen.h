#ifndef __E906VERTEXGEN_H__
#define __E906VERTEXGEN_H__

#include <vector>

#include <TString.h>
#include <TGeoManager.h>
#include <TGeoNode.h>
#include <TF2.h>

class PHCompositeNode;

class E906VertexGen
{
public:
    E906VertexGen();
    ~E906VertexGen();;

    //Initialize at the begining of Run
    void InitRun(PHCompositeNode* topNode);

    //Tree traversal
    void traverse(TGeoNode* node, int level);

private:
    //Array of beamline objects
    unsigned int nPieces;
    double probSum;
    double accumulatedProb[100]; //for now set to no more than 100 objects

    //Beam profile
    TF2* beamProfile;

    //flag to test if the generator has been initialized
    bool inited;
};

#endif
