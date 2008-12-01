#include <fstream>
#include "data_loading.h"
#include <iostream>

void get_full_line( ifstream & infile, string & line )
{
   do{
      if( ! infile ) {
         line = "";
         return;
      }
      getline( infile, line );
      
      while( line[ line.length()-1 ] == '\\' ) {
         line.erase( line.length()-1 );
         string cline;
         getline( infile, cline );
	 line.append( cline );
      }    
      // remove comments:
      int p = line.find('#');  
      if( p != string::npos )
         line.erase( p );	 

      // ignore empty lines
      for( p = 0; line[p]; p++ )	 
	 if( (line[p] != ' ') && (line[p] != '\t') )
	    break;
      if( line[p]  )
         break;
      
   } while( true );
}

set< string > get_gff_toc( const string & filename ) 
{
   set< string > res;
   ifstream infile( filename.c_str() );   
   while( infile ) {
      string line;
      get_full_line( infile, line );
      if( line=="" )
         break;
      
      // Find first word
      int begin, end;
      for( begin=0; line[begin]; begin++ )
         if( line[begin] != ' ' && line[begin] != '\t' )
	    break;
      for( end=begin; line[end]; end++ )
         if( line[end] == ' ' || line[end] == '\t' )
	    break;
	    
      res.insert( line.substr( begin, end-begin ) );
   }
   infile.close();
   return res;
}
