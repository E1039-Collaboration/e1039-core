/// SQParamDeco_v1.h
#ifndef _H_SQ_PARAM_DECO_V1_H_
#define _H_SQ_PARAM_DECO_V1_H_
#include "SQParamDeco.h"

class SQParamDeco_v1 : public SQParamDeco {
public:
  SQParamDeco_v1();
  virtual ~SQParamDeco_v1();

  void identify(std::ostream& os = std::cout) const;
  int        isValid() const;
  SQParamDeco* Clone() const;
  void         Reset();

  bool        has_variable(const std::string name) const;
  std::string get_variable(const std::string name) const;
  void        set_variable(const std::string name, const std::string value);

  ParamConstIter begin() const { return m_map.begin(); }
  ParamConstIter   end() const { return m_map.end(); }
  
private:
  ParamMap m_map;

  ClassDef(SQParamDeco_v1, 1);
};

#endif /* _H_SQ_PARAM_DECO_V1_H_ */
