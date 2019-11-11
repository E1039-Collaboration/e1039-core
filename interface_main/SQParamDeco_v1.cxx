///SQParamDeco_v1.cxx
#include "SQParamDeco_v1.h"

using namespace std;

ClassImp(SQParamDeco_v1);

SQParamDeco_v1::SQParamDeco_v1()
{
  ;
}

SQParamDeco_v1::~SQParamDeco_v1()
{
  ;
}

void SQParamDeco_v1::identify(ostream& os) const
{
  os << "---SQParamDeco_v1--------------------" << endl;
}

int SQParamDeco_v1::isValid() const
{
  return 1;
}

SQParamDeco* SQParamDeco_v1::Clone() const
{
  return new SQParamDeco_v1(*this);
}

void SQParamDeco_v1::Reset()
{
  m_map.clear();
}

bool SQParamDeco_v1::has_variable(const std::string name) const
{
  return m_map.find(name) != m_map.end();
}

std::string SQParamDeco_v1::get_variable(const std::string name) const
{
  return has_variable(name) ? m_map.at(name) : "";
}

void SQParamDeco_v1::set_variable(const std::string name, const std::string value)
{
  m_map[name] = value;
}
