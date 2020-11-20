/*=====================================================================================================
Author: Abinash Pun, Kun Liu
Sep, 2019
Goal: Beamline volumes work to import the primary vertex generator of E906 experiment(DPVertexGenerator)
from Kun to E1039 experiment in Fun4All framework
========================================================================================================*/

#ifndef __SQBEAMLINEOBJECT_H__
#define __SQBEAMLINEOBJECT_H__

#include <iostream>

#include <TGeoMaterial.h>
#include <TString.h>

class SQBeamlineObject
{
public:
    SQBeamlineObject();
    SQBeamlineObject(const TGeoMaterial* pMaterial);
    //BeamlineObject(const TGeoMixture* pMaterial);
    //! get the expotential z distribution
    double getZ();

    //! check if the beam is in acceptance
    bool inAcceptance(double x, double y);

    bool operator < (const SQBeamlineObject& obj) const;
    friend std::ostream& operator << (std::ostream& os, const SQBeamlineObject& obj);

public:
    TString name;

    //intialized by geometry
    double z_up, z_down, z0; //! the z position of upstram/downstream face and center
    double length;           //! length of the stuff
    double radiusX;          //! radiusX
    double radiusY;          //! radiusY

    //initialized by material property
    double nucIntLen;        //! nuclear interaction length in cm
    double density;          //! density in g/cm3
    double Z, A, N;          //! number of protons, nucleons, neutrons

    //intialized by its neighbours
    double attenuationSelf;  //! beam attenuation percentage by itself
    double attenuation;      //! beam attenuation factor by this object
    double prob;             //! probability of having collision
    double accumulatedProb;  //! sum of all the previous/upstream probs

    double protonPerc() { return Z/A; } //! percentage of protons = Z/A;
};


#endif
