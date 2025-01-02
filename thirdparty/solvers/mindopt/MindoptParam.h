#ifndef _MINDOPT_PARAM_H_
#define _MINDOPT_PARAM_H_

#include "MindoptType.h"

#ifdef __cplusplus
extern "C" {
#endif


 
MDODLL int MDOAPI MDOgetdblparam(
    MDOenv* env,
    const char* paramname,
    double* valueP
);


 
MDODLL int MDOAPI MDOgetintparam(
    MDOenv* env,
    const char* paramname,
    int* valueP
);


 
MDODLL int MDOAPI MDOgetstrparam(
    MDOenv* env,
    const char* paramname,
    char* valueP
);


 
MDODLL int MDOAPI MDOsetdblparam(
    MDOenv* env,
    const char* paramname,
    double newvalue
);


 
MDODLL int MDOAPI MDOsetintparam(
    MDOenv* env,
    const char* paramname,
    int newvalue
);


 
MDODLL int MDOAPI MDOsetstrparam(
    MDOenv* env,
    const char* paramname,
    const char* newvalue
);


 
MDODLL int MDOAPI MDOgetdblparaminfo(
    MDOenv* env,
    const char* paramname,
    double* valueP,
    double* minP,
    double* maxP,
    double* defaultP
);


 
MDODLL int MDOAPI MDOgetintparaminfo(
    MDOenv* env,
    const char* paramname,
    int* valueP,
    int* minP,
    int* maxP,
    int* defaultP
);


 
MDODLL int MDOAPI MDOgetstrparaminfo(
    MDOenv* env,
    const char* paramname,
    char* valueP,
    char* defaultP
);


 
MDODLL int MDOAPI MDOreadparams(
    MDOenv* env,
    const char* filename
);


 
MDODLL int MDOAPI MDOwriteparams(
    MDOenv* env,
    const char* filename
);


 
MDODLL void MDOAPI MDOresetparams(
    MDOenv* env
);

#ifdef __cplusplus
}
#endif

#endif