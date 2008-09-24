#include <R.h>
#include <Rinternals.h>

SEXP make_wiggle_vector( SEXP start, SEXP end, SEXP value, SEXP chrlength )
{
   SEXP res;
   int i, j;
  
   PROTECT( res = allocVector( REALSXP, INTEGER(chrlength)[0] ) );
   memset( REAL(res), 0, length(res) * sizeof(double) );

   for( i = 0; i < length(start); i++ ) {
      if( INTEGER(end)[i] < INTEGER(start)[i] ) {
         char buf[200];
	 snprintf( buf, 200, "end[%d] < start[%d]", i+1, i+1 );
         error( buf );
      }
      if( INTEGER(end)[i] > length(res) )
	 error( "'chrlength' is too small" );	 
      for( j = INTEGER(start)[i]; j <= INTEGER(end)[i]; j++ )
	 REAL(res)[j-1] += REAL(value)[i];
   }
      
   UNPROTECT( 1 );  
   return res;
}
