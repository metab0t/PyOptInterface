#ifndef _MINDOPT_MODEL_H_
#define _MINDOPT_MODEL_H_

#include "MindoptType.h"

#ifdef __cplusplus
extern "C" {
#endif

 
MDODLL int MDOAPI MDOloadmodel(
    MDOenv* env,
    MDOmodel** modelP,
    const char* Pname,
    int numvars,
    int numconstrs,
    int objsense,
    double objcon,
    double* obj,
    char* sense,
    double* rhs,
    int* vbeg,
    int* vlen,
    int* vind,
    double* vval,
    double* lb,
    double* ub,
    char* vtype,
    const char** varnames,
    const char** constrnames
);


 
MDODLL int MDOAPI MDOnewmodel(
    MDOenv* env,
    MDOmodel** modelP,
    const char* Pname,
    int numvars,
    double* obj,
    double* lb,
    double* ub,
    char* vtype,
    const char** varnames
);


 
MDODLL MDOmodel* MDOAPI MDOcopymodel(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOaddconstr(
    MDOmodel* model,
    int numnz,
    int* cind,
    double* cval,
    char sense,
    double rhs,
    const char* constrname
);


 
MDODLL int MDOAPI MDOaddqconstr(
    MDOmodel* model,
    int numlnz,
    int *lind,
    double* lval,
    int numqnz,
    int* qrow,
    int* qcol,
    double* qval,
    char sense,
    double rhs,
    const char* constrname
);


 
MDODLL int MDOAPI MDOaddsos(
    MDOmodel* model,
    int numsos,
    int nummembers,
    int* types,
    int* beg,
    int* ind,
    double* weight
);


 
MDODLL int MDOAPI MDOaddgenconstrIndicator(
    MDOmodel* model,
    const char* name,
    int binvar,
    int binval,
    int nvars,
    const int* vars,
    const double* vals,
    char sense,
    double rhs
);


 
MDODLL int MDOAPI MDOaddpsdconstr(
    MDOmodel* model,
    int numnz,
    int numpsdvars,
    int numpsdnz,
    int* cind,
    double* cval,
    int* psdvarind,
    int* mbgn,
    int* indices,
    double* values,
    char sense,
    double rhs,
    const char* constrname
);


 
MDODLL int MDOAPI MDOaddconstrs(
    MDOmodel* model,
    int numconstrs,
    int numnz,
    int* cbeg,
    int* cind,
    double* cval,
    char* sense,
    double* rhs,
    const char** constrnames
);

 
MDODLL int MDOAPI MDOaddqpterms(
    MDOmodel* model,
    int numqnz,
    int* qrow,
    int* qcol,
    double* qval
);


 
MDODLL int MDOAPI MDOaddrangeconstr(
    MDOmodel* model,
    int numnz,
    int* cind,
    double* cval,
    double lower,
    double upper,
    const char* constrname
);

 
MDODLL int MDOAPI MDOaddrangepsdconstr(
    MDOmodel* model,
    int numnz,
    int numpsdvars,
    int numpsdnz,
    int* cind,
    double* cval,
    int* psdvarind,
    int* mbgn,
    int* indices,
    double* values,
    double lower,
    double upper,
    const char* constrname
);


 
MDODLL int MDOAPI MDOaddrangeconstrs(
    MDOmodel* model,
    int numconstrs,
    int numnz,
    int* cbeg,
    int* cind,
    double* cval,
    double* lower,
    double* upper,
    const char** constrnames
);


 
MDODLL int MDOAPI MDOaddvar(
    MDOmodel* model,
    int numnz,
    int* vind,
    double* vval,
    double obj,
    double lb,
    double ub,
    char vtype,
    const char* varname
);


 
MDODLL int MDOAPI MDOaddpsdvar(
    MDOmodel* model,
    int dim,
    int numobjnz,
    int* objind,
    double* objval,
    const char* varname
);


 
MDODLL int MDOAPI MDOaddvars(
    MDOmodel* model,
    int numvars,
    int numnz,
    int* vbeg,
    int* vind,
    double* vval,
    double* obj,
    double* lb,
    double* ub,
    char* vtype,
    const char** varnames
);


 
MDODLL int MDOAPI MDOchgcoeffs(
    MDOmodel* model,
    int numchgs,
    int* cind,
    int* vind,
    double* val
);


 
MDODLL int MDOAPI MDOdelconstrs(
    MDOmodel* model,
    int numdel,
    int* ind
);


 
MDODLL int MDOAPI MDOdelqconstrs(
    MDOmodel* model,			
    int numdel,
    int* ind
);


 
MDODLL int MDOAPI MDOdelsos(
    MDOmodel* model,
    int len,
    int* ind
);


 
MDODLL int MDOAPI MDOdelgenconstrs(
    MDOmodel* model,
    int len,
    int* ind
);


 
MDODLL int MDOAPI MDOdelq(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOdelvars(
    MDOmodel* model,
    int numdel,
    int* ind
);


 
MDODLL int MDOAPI MDOfreemodel(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOoptimize(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOcomputeIIS(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOreset(
    MDOmodel* model,
    int clearall
);


 
MDODLL int MDOAPI MDOgetcoeff(
    MDOmodel* model,
    int constrind,
    int varind,
    double* valP
);


 
MDODLL int MDOAPI MDOgetpsdconstr(
    MDOmodel* model,
    int rind,
    int* numnzs,
    int* numpsdvars,
    int* nummatnz,
    int* cind,
    double* cval,
    int* psdvarind,
    int* mbgn,
    int* mind,
    double* mval
);


 
MDODLL int MDOAPI MDOgetconstrbyname(
    MDOmodel* model,
    const char* name,
    int* constrnumP
);


 
MDODLL int MDOAPI MDOgetqconstrbyname(
    MDOmodel* model,
    const char* name,
    int* qconstr
);


 
MDODLL int MDOAPI MDOgetconstrs(
    MDOmodel* model,
    int* numnzP,
    int* cbeg,
    int* cind,
    double* cval,
    int start,
    int len
);


 
MDODLL int MDOAPI MDOgetqconstr(
    MDOmodel* model,
    int qconstr,
    int* numlnz,
    int* lind,
    double* lval,
    int* numqnz,
    int* qrow,
    int* qcol,
    double* qval
);


 
MDODLL int MDOAPI MDOgetsos(
    MDOmodel* model,
    int* nummembersP,
    int* sostype,
    int* beg,
    int* ind,
    double* weight,
    int start,
    int len
);


 
MDODLL int MDOAPI MDOgetgenconstrIndicator(
    MDOmodel* model,
    int genconstr,
    int* binvarP,
    int* binvalP,
    int* nvarsP,
    int* vars,
    double* vals,
    char* senseP,
    double* rhsP
);


 
MDODLL int MDOAPI MDOgetconstrcoeff(
    MDOmodel* model,
    int rind,
    int cind,
    double* coeff
);

 
MDODLL int MDOAPI MDOsetconstrcoeff(
    MDOmodel* model,
    int rind,
    int cind,
    double coeff
);


 
MDODLL MDOenv* MDOAPI MDOgetenv(
    MDOmodel* model
);


 
MDODLL int MDOAPI MDOgetq(
    MDOmodel* model,
    int* numqnzP,
    int* qrow,
    int* qcol,
    double* qval
);


 
MDODLL int MDOAPI MDOgetvarbyname(
    MDOmodel* model,
    const char* name,
    int* varnumP
);


 
MDODLL int MDOAPI MDOgetvars(
    MDOmodel* model,
    int* numnzP,
    int* vbeg,
    int* vind,
    double* vval,
    int start,
    int len
);


 
MDODLL int MDOAPI MDOreadmodel(
    MDOenv* env,
    const char* filename,
    MDOmodel** modelP
);


 
MDODLL int MDOAPI MDOread(
    MDOmodel* model,
    const char* filename
);


 
MDODLL int MDOAPI MDOwrite(
    MDOmodel* model,
    const char* filename
);

 
MDODLL int MDOAPI MDOterminate(
    MDOmodel* model
);

#ifdef __cplusplus
}
#endif

#endif