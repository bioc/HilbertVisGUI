/*
See header file R_env_prot.h for documentation.

(c) Simon Anders, European Bioinformatics Institute, sanders@fs.tum.de
Version of 2008-05-01.
Released under the GNU General Public Licence (version 2 or newer).
*/


#include <stdio.h>
#include "R_env_prot.h"

SEXP prot_env = NULL;

SEXP init_prot_env( void )
{
   SEXP call;
   if( prot_env )
      return prot_env;
   else {
      Rf_protect( call = Rf_allocList( 3 ) );
      SET_TYPEOF( call, LANGSXP );

      SETCAR( call, Rf_install( "new.env" ) );

      /* hash = TRUE */
      SET_TAG( CDR(call), Rf_install( "hash" ) );
      SETCAR( CDR(call),  Rf_allocVector( LGLSXP, 1 ) );
      LOGICAL( CADR(call) )[0] = TRUE;
      
      /* parent = baseenv() */
      SET_TAG( CDDR(call), Rf_install( "parent" ) );
      SETCAR( CDDR(call), R_BaseEnv );

      prot_env = Rf_eval( call, R_BaseEnv );
      Rf_unprotect( 1 );   
      return prot_env;
   }
}

SEXP env_protect( SEXP obj )
{
   char buf[100];
   SEXP pair, count;
   
   if( !prot_env )
      Rf_error( "env_protect: 'init_prot_env' has not yet been called!" );   
   
   snprintf( buf, 100, "%p", obj );
   pair = Rf_findVar( Rf_install( buf ), prot_env );

   if( pair == R_UnboundValue ) {
      pair = Rf_allocVector( VECSXP, 2 );
      Rf_protect( pair );
      SET_VECTOR_ELT( pair, 0, obj );
      count = Rf_allocVector( INTSXP, 1 );
      INTEGER(count)[0] = 1;
      SET_VECTOR_ELT( pair, 1, count );
      Rf_defineVar( Rf_install( buf ), pair, prot_env );
      Rf_unprotect( 1 );
   } else {
      INTEGER( VECTOR_ELT( pair, 1 ) )[0] += 1;
   }
   return R_NilValue;
}

SEXP env_unprotect( SEXP obj )
{
   char buf[100];
   SEXP pair, call;

   if( !prot_env )
      Rf_error( "env_unprotect: 'init_prot_env' has not yet been called!" );

   snprintf( buf, 100, "%p", obj );
   pair = Rf_findVar( Rf_install( buf ), prot_env );

   if( pair == R_UnboundValue )
      Rf_error( "env_unprotect: Attempt to env_unprotect a non-env_protected object." );
   
   INTEGER( VECTOR_ELT( pair, 1 ) )[0] -= 1;
   
   if( INTEGER( VECTOR_ELT( pair, 1 ) )[0] == 0 ) {
      Rf_protect( call = Rf_allocList( 3 ) );
      SET_TYPEOF( call, LANGSXP );

      SETCAR( call, Rf_install("rm") );

      SET_TAG( CDR(call), Rf_install("list") );
      SETCAR( CDR(call),  Rf_allocVector( STRSXP, 1 ) );
      SET_STRING_ELT( CADR(call), 0, Rf_mkChar( buf ) );

      SET_TAG( CDDR(call), Rf_install("envir") );
      SETCAR( CDDR(call),  prot_env );

      Rf_eval( call, R_BaseEnv );
      Rf_unprotect( 1 );   
   }
   return R_NilValue;
}

