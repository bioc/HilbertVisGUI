#include "simple_regex_pp.h"

#include <iostream>

using namespace std;


void regex_pp( const string regex, const string & s,  
   vector< regex_match_t > & res, int cflags, int eflags ) 
{
   // Compile Regex:
   regex_t preg;
   int err;
   err = regcomp( &preg, regex.c_str(), cflags | REG_EXTENDED );
   if( err != 0 ) {
      char errbuf[ 300 ];
      regerror( err, &preg, errbuf, 300 );
      regex_exception_compilation_failed e;
      e.error_message = errbuf;
      regfree( &preg );
      throw e;
   }	    
   
   // Get number of opening parantheses:
   int nmatch = 1;
   for( int i = 0; i < regex.length(); i++ )
      if( regex[i] == '(' )
         nmatch++;
   
   // Execute regex
   regmatch_t * rm = new regmatch_t[ nmatch ];
   if( regexec( &preg, s.c_str(), nmatch, rm, eflags ) != 0 ) {
      regfree( &preg );  
      delete[] rm; 
      throw regex_exception_no_match( );
   }
      
   // Fill result vector
   res.clear();
   for( int i = 0; i < nmatch; i++ ) {
      if( rm[i].rm_so == -1 )
         continue;
      regex_match_t m;
      m.begin  = rm[i].rm_so;
      m.length = rm[i].rm_eo - rm[i].rm_so;
      m.s      = s.substr( m.begin, m.length );
      res.push_back( m );
   }

   delete[] rm; 
   regfree( &preg );
}   

bool regex_match_pp( const std::string regex, const std::string & s,  
   int cflags, int eflags )
{
   static vector< regex_match_t > dummy;
   try {
      regex_pp( regex, s, dummy, cflags, eflags );
   } catch( regex_exception_no_match e ) {
      return false;
   }
   return true;
}   

