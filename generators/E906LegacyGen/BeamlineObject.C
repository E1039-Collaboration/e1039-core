#include "BeamlineObject.h"

#include <TMath.h>

BeamlineObject::BeamlineObject() {}

BeamlineObject::BeamlineObject(const TGeoMaterial* pMaterial)
{
    std::cout << pMaterial->GetName() << std::endl;
}

bool BeamlineObject::operator < (const BeamlineObject& obj) const
{
    return z0 < obj.z0;
}

std::ostream& operator << (std::ostream& os, const BeamlineObject& obj)
{
    os << obj.name;
    os << std::endl;
}

double BeamlineObject::getZ()
{
    return z_up - nucIntLen*TMath::Log(1. - attenuationSelf*1.); //Todo Fix this place holders
}

bool BeamlineObject::inAcceptance(double x, double y)
{
    return true;
}
