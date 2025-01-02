#ifndef _MINDOPT_ENV_H_
#define _MINDOPT_ENV_H_

#include "MindoptType.h"

#ifdef __cplusplus
extern "C" {
#endif

 
MDODLL int MDOAPI MDOloadenv(
    MDOenv** envP,
    const char* logfilename
);


 
MDODLL int MDOAPI MDOemptyenv(
    MDOenv** envP
);


 
MDODLL int MDOAPI MDOstartenv(
    MDOenv* env
);


 
MDODLL void MDOAPI MDOfreeenv(
    MDOenv* env
);


 
MDODLL void MDOAPI MDOmsg(
    MDOenv* env,
    const char* message
);


 
MDODLL const char* MDOAPI MDOexplainerror(
    int code
);


 
MDODLL char* MDOAPI MDOgeterrormsg(
    MDOenv* env
);

 
MDODLL void MDOAPI MDOversion(
    int* majorP,
    int* minorP,
    int* technicalP
);

 
MDODLL int MDOAPI MDOsetlogcallback(
    MDOenv* env,
    void (*logcb)(const char* msg, void* userdata),       
    void* userdata
);


#ifdef __cplusplus
}
#endif

#endif
