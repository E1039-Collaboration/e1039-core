#ifndef __BEAMLINEOBJECT_H__
#define __BEAMLINEOBJECT_H__

#include <iostream>

#include <TGeoMaterial.h>
#include <TString.h>

class BeamlineObject
{
public:
    BeamlineObject();
    BeamlineObject(const TGeoMaterial* pMaterial);
    //BeamlineObject(const TGeoMixture* pMaterial);
    //! get the expotential z distribution
    double getZ();

    //! check if the beam is in acceptance
    bool inAcceptance(double x, double y);

    bool operator < (const BeamlineObject& obj) const;
    friend std::ostream& operator << (std::ostream& os, const BeamlineObject& obj);

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
    double protonPerc;       //! percentage of protons = Z/A;

    //intialized by its neighbours
    double attenuationSelf;  //! beam attenuation percentage by itself
    double attenuation;      //! beam attenuation factor by this object
    double prob;             //! probability of having collision
    double accumulatedProb;  //! sum of all the previous/upstream probs

  
};


#endif
