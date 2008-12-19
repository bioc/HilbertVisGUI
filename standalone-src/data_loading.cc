#include <fstream>
#include <sstream>
#include "data_loading.h"
#include "maqmap_m.h"

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

class no_word_found_exception : public data_loading_exception {};
class format_error : public data_loading_exception {};

inline pair< int, int > find_word( const string & line, int start_at = 0 )
// returns (pos, length)
{
   int wbeg, wend;
   for( wbeg = start_at; line[wbeg]; wbeg++ )
      if( line[wbeg] != ' ' && line[wbeg] != '\t' )
         break;
   for( wend = wbeg; line[wend]; wend++ )
      if( line[wend] == ' ' || line[wend] == '\t' )
         break;
   if( ! line[wend] )
      throw no_word_found_exception( );
   return pair<int,int>( wbeg, wend - wbeg );
}

template <class T> T from_string( const string & s )
{
  std::istringstream iss( s );
  T t;
  iss >> t;
  if( iss.fail() )
     throw conversion_failed_exception( );
  return t;
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
      try{ 
         pair<int,int> pos = find_word( line );
         res.insert( line.substr( pos.first, pos.second ) );
      } catch( no_word_found_exception e ) {
         continue;
      }
   }
   infile.close();
   return res;
}

step_vector<double> * load_gff_data( const string & filename, const string & seqname )
{
   step_vector<double> * sv = new step_vector<double>( numeric_limits<long int>::max() );
   long int maxidx = numeric_limits<long int>::min();
   ifstream infile( filename.c_str() );   
   
   while( infile ) {
      string line;
      pair<int,int> pos[6];
      get_full_line( infile, line );
      if( line=="" )
         break;
      
      // Find first word
      try{ 
         pos[0] = find_word( line );            
         if( line.substr( pos[0].first, pos[0].second ) != seqname )
            continue;
         
         // Find next words:
         for( int i = 1; i < 6; i++ )
            pos[i] = find_word( line, pos[i-1].first + pos[i-1].second );
      } catch( no_word_found_exception e ) {
         cerr << "Not enough words in following line: " << line << endl;
      }

      try{        
         long int begin  = from_string< long int >( line.substr( pos[3].first, pos[3].second ) );
         long int end    = from_string< long int >( line.substr( pos[4].first, pos[4].second ) );
         double   score  = from_string< double   >( line.substr( pos[5].first, pos[5].second ) );
         sv->add_value( begin, end, score );
         if( end > maxidx )
            maxidx = end;
      } catch( conversion_failed_exception e ) {
         cerr << "Could not parse the following line: " << line << endl;
      }

   }

   infile.close();
   sv->max_index = maxidx;
   
   //for( step_vector<double>::const_iterator i = sv->begin(); i != sv->end(); i++ )
   //   cout << i->first << " " << i->second << endl;
   
   return sv;
}

template< int max_readlen >
set< string > get_maqmap_toc_templ( const string & filename ) 
{
   gzFile fp = gzopen( filename.c_str(), "rb" );
   if( !fp ) 
      throw data_loading_exception( "Failed to open Maq map file." );
      
   int i;   
   gzread( fp, &i, sizeof(int) );
   if( i != MAQMAP_FORMAT_NEW ) {
      gzclose( fp );
      throw data_loading_exception( "Selected file is not a MAQ map file" ); 
   }
   i = gzrewind( fp );
      
      
   maqmap_T<max_readlen> * mm = maqmap_read_header<max_readlen>( fp );
   set< string > res;
   
   for( i = 0; i < mm->n_ref; i++ )
      res.insert( mm->ref_name[i] );

   maq_delete_maqmap<max_readlen>( mm );

   return res;
}   

set< string > get_maqmap_toc( const string & filename ) 
{
   return get_maqmap_toc_templ<MAX_READLEN_NEW>( filename );
}

set< string > get_maqmap_old_toc( const string & filename )
{
   return get_maqmap_toc_templ<MAX_READLEN_OLD>( filename );
}

template< int max_readlen >
step_vector<double> * load_maqmap_data_templ( const string & filename, const string & seqname, int minqual )
{
   step_vector<double> * sv = new step_vector<double>( numeric_limits<long int>::max() );
   long int maxidx = numeric_limits<long int>::min();
   
   gzFile fp = gzopen( filename.c_str(), "rb" );
   if( !fp ) 
      throw data_loading_exception( "Failed to open Maq map file." );
      
   int seqidx;
   maqmap_T<max_readlen> * mm = maqmap_read_header<max_readlen>( fp );
   for( seqidx = 0; seqidx < mm->n_ref; seqidx++ ) {
      if( seqname == mm->ref_name[seqidx] )
         break;
   }         
   if( seqidx >= mm->n_ref ) {
      throw data_loading_exception( "Sequence vanished from file. This should not happen." );
      maq_delete_maqmap<max_readlen>( mm );
   }
   
   for( long i = 0; i < mm->n_mapped_reads; i++ ) {
      maqmap1_T<max_readlen> read;
      maqmap_read1<max_readlen>( fp, &read );
      if( read.seqid != seqidx )
         continue;
      if( read.map_qual < minqual )
         continue;
      long end = ( read.pos >> 1 ) + read.size;
      sv->add_value( read.pos >> 1, end, 1 );
      if( end > maxidx )
         maxidx = end;
   }
   
   maq_delete_maqmap<max_readlen>( mm );
   sv->max_index = maxidx;
   return sv;
}

step_vector<double> * load_maqmap_data( const string & filename, const string & seqname, int minqual )
{
   return load_maqmap_data_templ<MAX_READLEN_NEW>( filename, seqname, minqual );
}

step_vector<double> * load_maqmap_old_data( const string & filename, const string & seqname, int minqual )
{
   return load_maqmap_data_templ<MAX_READLEN_OLD>( filename, seqname, minqual );
}

