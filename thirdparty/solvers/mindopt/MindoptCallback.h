#ifndef _MINDOPT_CALLBACK_H_
#define _MINDOPT_CALLBACK_H_

#include "MindoptType.h"

#ifdef __cplusplus
extern "C" {
#endif


 
MDODLL int MDOAPI MDOsetcallbackfunc(
    MDOmodel* model,
 	int (*cb)(const MDOmodel* model, void* cbdata, int where, const void* usrdata),
 	void* usrdata
);


 
MDODLL int MDOAPI MDOgetcallbackfunc(
    MDOmodel* model,
 	int	(**cb)(const MDOmodel* model, void* cbdata, int where, const void* usrdata)
);


 
MDODLL int MDOAPI MDOcbget(
    void* cbdata,
 	int	where,
 	int	what,
 	void* resultP
);


 
MDODLL int MDOAPI MDOcbcut(
    void* cbdata,
 	int	cutlen,
 	const int* cutind,
 	const double* cutval,
 	char cutsense,
 	double cutrhs
);


 
MDODLL int MDOAPI MDOcbbranch(
    void* cbdata,
    int index,
    double value,
    int way
);


 
MDODLL int MDOAPI MDOcbsolution(
    void* cbdata,
 	const double* solution,
 	double* objP
);

#ifdef __cplusplus
}
#endif

#endif