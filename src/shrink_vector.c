#include <limits.h>
#include <R.h>
#include <Rinternals.h>
#include <R_ext/Arith.h>

SEXP shrink_vector_int( SEXP vector, SEXP new_size ) 
{
   const double step = (double) LENGTH(vector) / INTEGER(new_size)[0];
   int curbin, nxtbin, i, j;
   SEXP res;

   PROTECT( res = allocVector( INTSXP, INTEGER(new_size)[0] ) );
   nxtbin = 0;
   for( i = 0; i < LENGTH(res); i++ ) {
      curbin = nxtbin;
      nxtbin = (int) round( step * (i+1) );
      if( nxtbin > LENGTH(vector) )
         nxtbin = LENGTH(vector);
      INTEGER(res)[i] = INT_MIN;
      for( j = curbin; j < nxtbin; j++ )
         if( INTEGER(vector)[j] > INTEGER(res)[i] )
	    INTEGER(res)[i] = INTEGER(vector)[j];
   }
   UNPROTECT(1);
   return res;
}   

SEXP shrink_vector_double( SEXP vector, SEXP new_size ) 
{
   const double step = (double) LENGTH(vector) / INTEGER(new_size)[0];
   int curbin, nxtbin, i, j;
   SEXP res;

   PROTECT( res = allocVector( REALSXP, INTEGER(new_size)[0] ) );
   nxtbin = 0;
   for( i = 0; i < LENGTH(res); i++ ) {
      curbin = nxtbin;
      nxtbin = (int) round( step * (i+1) );
      if( nxtbin > LENGTH(vector) )
         nxtbin = LENGTH(vector);
      REAL(res)[i] =  R_NegInf;
      for( j = curbin; j < nxtbin; j++ )
         if( REAL(vector)[j] > REAL(res)[i] )
	    REAL(res)[i] = REAL(vector)[j];
   }
   UNPROTECT(1);
   return res;
}   
