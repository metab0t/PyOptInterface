#ifndef _MINDOPT_ATTR_H_
#define _MINDOPT_ATTR_H_

#include "MindoptType.h"

#ifdef __cplusplus
extern "C" {
#endif

 
MDODLL int MDOAPI MDOgetattrinfo(
    MDOmodel* model,
    const char* attrname,
    int* datatypeP,
    int* attrtypeP,
    int* settableP
);


 
MDODLL int MDOAPI MDOgetintattr(
    MDOmodel* model,
    const char* attrname,
    int* valueP
);


 
MDODLL int MDOAPI MDOsetintattr(
    MDOmodel* model,
    const char* attrname,
    int newvalue
);


 
MDODLL int MDOAPI MDOgetintattrelement(
    MDOmodel* model,                
    const char* attrname,           
    int element,
    int* valueP
);


 
MDODLL int MDOAPI MDOsetintattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    int newvalue
);


 
MDODLL int MDOAPI MDOgetintattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    int* values
);


 
MDODLL int MDOAPI MDOsetintattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    int* values
);


 
MDODLL int MDOAPI MDOgetintattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    int* values
);


 
MDODLL int MDOAPI MDOsetintattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    int* values
);


 
MDODLL int MDOAPI MDOgetdblattr(
    MDOmodel* model,
    const char* attrname,
    double* valueP
);


 
MDODLL int MDOAPI MDOsetdblattr(
    MDOmodel* model,
    const char* attrname,
    double newvalue
);


 
MDODLL int MDOAPI MDOgetdblattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    double* valueP
);


 
MDODLL int MDOAPI MDOsetdblattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    double newvalue
);


 
MDODLL int MDOAPI MDOgetdblattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    double* values
);


 
MDODLL int MDOAPI MDOsetdblattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    double* values
);


 
MDODLL int MDOAPI MDOgetdblattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    double* values
);


 
MDODLL int MDOAPI MDOsetdblattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    double* values
);


 
MDODLL int MDOAPI MDOgetcharattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    char* valueP
);


 
MDODLL int MDOAPI MDOsetcharattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    char newvalue
);


 
MDODLL int MDOAPI MDOgetcharattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    char* values
);


 
MDODLL int MDOAPI MDOsetcharattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    char* values
);


 
MDODLL int MDOAPI MDOgetcharattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    char* values
);


 
MDODLL int MDOAPI MDOsetcharattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    char* values
);


 
MDODLL int MDOAPI MDOgetstrattr(
    MDOmodel* model,
    const char* attrname,
    char** valueP
);


 
MDODLL int MDOAPI MDOsetstrattr(
    MDOmodel* model,
    const char* attrname,
    const char* newvalue
);


 
MDODLL int MDOAPI MDOgetstrattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    char** valueP
);


 
MDODLL int MDOAPI MDOsetstrattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    char* newvalue
);


 
MDODLL int MDOAPI MDOgetstrattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    char** values
);

 
MDODLL int MDOAPI MDOsetstrattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    char** values
);


 
MDODLL int MDOAPI MDOgetstrattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    char** values
);


 
MDODLL int MDOAPI MDOsetstrattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* ind,
    char** values
);

 
MDODLL int MDOAPI MDOgetmatattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    int* nummatnz,
    int* rows,
    int* cols,
    int* indices,
    double* values
);

 
MDODLL int MDOAPI MDOsetmatattrelement(
    MDOmodel* model,
    const char* attrname,
    int element,
    int nummatnz,
    int rows,
    int cols,
    int* indices,
    double* values
);

 
MDODLL int MDOAPI MDOgetmatattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    int* nummatnz,
    int* mbgn,
    int* rows,
    int* cols,
    int* indices,
    double* values
);

 
MDODLL int MDOAPI MDOsetmatattrarray(
    MDOmodel* model,
    const char* attrname,
    int start,
    int len,
    int nummatnz,
    int* mbgn,
    int* rows,
    int* cols,
    int* indices,
    double* values
);


 
MDODLL int MDOAPI MDOgetmatattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* mind,
    int* nummatnz,
    int* mbgn,
    int* rows,
    int* cols,
    int* indices,
    double* values
);

 
MDODLL int MDOAPI MDOsetmatattrlist(
    MDOmodel* model,
    const char* attrname,
    int len,
    int* mind,
    int nummatnz,
    int* mbgn,
    int* rows,
    int* cols,
    int* indices,
    double* values
);

#ifdef __cplusplus
}
#endif

#endif