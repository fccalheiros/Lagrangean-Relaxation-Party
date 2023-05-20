#ifndef _RGPCUT_H
#define _RGPCUT_H

#include "Constraint.h"
#include "Variable.h"

class RGPCut : public Constraint {

 public:

    RGPCut();
    RGPCut(float rhs);
    virtual ~RGPCut();

    virtual void  InsertVariable(Variable *var, float coef);
  

 public:

    bool Compare( RGPCut *rest );

    //bool operator== (RGPCut *rest);

protected:

    int _key;
  
};


#endif
