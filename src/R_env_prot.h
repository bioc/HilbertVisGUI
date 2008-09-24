/* 
This is a simple tool to protect R objects from R's garbage
collector when the usual stack-based protect/unprotect mechanism 
cannot be used because C code wants to keep hold of objects even after
returning control to the R main loop. This happens e.g. if a SEXP 
is not just a local variable in a C function called from R
but stays alive after returning from the fucntion because it has
been stored in a static or global variable or in an object on
the C heap. See below for details.

(c) Simon Anders, European Bioinformatics Institute, sanders@fs.tum.de
Version of 2008-05-01.
Released under the GNU General Public Licence (version 2 or newer).
*/

#ifndef R_ENV_PROT_H
#define R_ENV_PROT_H

#include <R.h>

#ifndef R_NO_REMAP
#  define R_NO_REMAP
#  include <Rinternals.h>
#  undef R_NO_REMAP
#else
#  include <Rinternals.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


SEXP init_prot_env( void );

/* This function must be called from R at least once before either 
of the other two functions are called from C. Call it as
   protEnv <- .Call( "init_prot_env" )
and keep the returned environment (here: protEnv) in your package's 
internal name space so that it is safe from the garbage collector. */


SEXP env_protect( SEXP obj );

/* Call this function from within C to protect the SEXP 'obj'. It will be
put into the protective environment, using its address (i.e., the value
of the SEXP pointer) as hash key. A simple reference counter is implemented
so that you can call env_protect on the same object mutiple times.
The function always returns Rf_NilValue. */


SEXP env_unprotect( SEXP obj );

/* Call this function from within C to unprotect the SEXP 'obj'. It will 
be found in the protective environment (by looking at its address) and its
reference counter decreased by one. If zero is reached, the object is
removed in order to allow for its collection by the GC. 
The function always returns Rf_NilValue. */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* R_ENV_PROT_H */
