#include <fstream>
#include <sstream>
#include "data_loading.h"
#include "maqmap_m.h"

#include <glib.h>

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
   if( ! line[wend] )
      throw no_word_found_exception( );
   for( wend = wbeg; line[wend]; wend++ )
      if( line[wend] == ' ' || line[wend] == '\t' )
         break;
   return pair<int,int>( wbeg, wend - wbeg );
}

bool starts_with( const string & s, const string & st )
// checks whether s starts with st, ignoring leading whitespace and case
{
   int pos, pos2;
   for( pos = 0; s[pos]; pos++ )
      if( s[pos] != ' ' && s[pos] != '\t' )
         break;
   if( pos + st.length() >= s.length() )
      return false;
   for( pos2 = 0; s[pos] && st[pos2]; pos++, pos2++ )
      if( tolower( s[pos] ) != tolower( st[pos2] ) ) {
         return false;
      }
   return true;
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

enum wiggle_subformat { bed, fixedStep, variableStep };

set<string> get_wiggle_toc( const string & filename )
{
   set< string > res;
   ifstream infile( filename.c_str() );   
   GRegex * re_trackname = g_regex_new( 
      "name\\s*=\\s*(\"[^\"]*\"|\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   GRegex * re_chromname = g_regex_new( 
      "chrom\\s*=\\s*(\"[^\"]*\"|\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   string trackname = "unnamed_track";
   string chromname = "unnamed_chromosome";
   wiggle_subformat wsf = bed;
   while( infile ) {
      string line;
      get_full_line( infile, line );
      if( line == "" )
         break;
      
      if( starts_with( line, "#" ) )
         continue;
      if( starts_with( line, "browser" ) )
         continue;
      if( starts_with( line, "track" ) ) {
         GMatchInfo * mi;
         if( g_regex_match( re_trackname, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
            trackname = g_match_info_fetch( mi, 1 );
         else
            trackname = "track_name_missing";
         g_match_info_free( mi );
         wsf = bed;
         chromname = "unnamed_chromosome";
         continue;
      }
      if( starts_with( line, "fixedStep" ) || starts_with( line, "variableStep" ) ) {
         GMatchInfo * mi;
         if( g_regex_match( re_chromname, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
            chromname = g_match_info_fetch( mi, 1 );
         else
            chromname = "chromosome_name_missing";
         g_match_info_free( mi );
         res.insert( trackname + " // " + chromname );
         if( starts_with( line, "fixedStep" ) )
            wsf = fixedStep;
         else 
            wsf = variableStep;
      }
      if( wsf == bed) {
         pair<int,int> pos = find_word( line );
         string s = line.substr( pos.first, pos.second );
         if( s != chromname ) {          
            chromname = s;
            res.insert( trackname + " // " + chromname );
         }
      }
   }
   infile.close();
   g_regex_unref( re_trackname );
   g_regex_unref( re_chromname );   
   return res;
}

step_vector<double> * load_wiggle_data( const string & filename, const string & seqname )
{
   step_vector<double> * sv = new step_vector<double>( numeric_limits<long int>::max() );
   long int maxidx = numeric_limits<long int>::min();
   ifstream infile( filename.c_str() );   

   GRegex * re_trackname = g_regex_new( 
      "name\\s*=\\s*(\"[^\"]*\"|\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   GRegex * re_chromname = g_regex_new( 
      "chrom\\s*=\\s*(\"[^\"]*\"|\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   GRegex * re_span = g_regex_new( 
      "span\\s*=\\s*(\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   GRegex * re_step = g_regex_new( 
      "step\\s*=\\s*(\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   GRegex * re_start = g_regex_new( 
      "start\\s*=\\s*(\\w*)", G_REGEX_CASELESS, GRegexMatchFlags( 0 ), NULL );
   string trackname = "unnamed_track";
   string chromname = "unnamed_chromosome";
   wiggle_subformat wsf = bed;
   long int begin, step_interval, span;
   while( infile ) {
      string line;
      get_full_line( infile, line );
      if( line == "" )
         break;
      
      try {
         if( starts_with( line, "#" ) )
            continue;
         if( starts_with( line, "browser" ) )
            continue;
         if( starts_with( line, "track" ) ) {
            GMatchInfo * mi;
            if( g_regex_match( re_trackname, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
               trackname = g_match_info_fetch( mi, 1 );
            else
               trackname = "track_name_missing";
            g_match_info_free( mi );
            wsf = bed;
            chromname = "unnamed_chromosome";
            continue;
         }
         
         if( starts_with( line, "fixedStep" ) || starts_with( line, "variableStep" ) ) {
            GMatchInfo * mi;

            if( g_regex_match( re_chromname, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
               chromname = g_match_info_fetch( mi, 1 );
            else
               chromname = "chromosome_name_missing";
            g_match_info_free( mi );
            
            if( g_regex_match( re_span, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
               span = from_string< long int >( g_match_info_fetch( mi, 1 ) ); 
            else
               span = 1;
            g_match_info_free( mi );

            if( starts_with( line, "fixedStep" ) ) {
               wsf = fixedStep;

               if( g_regex_match( re_step, line.c_str(), GRegexMatchFlags( 0 ), &mi ) ) {
                  step_interval = from_string< long int >( g_match_info_fetch( mi, 1 ) );
                  begin -= step_interval;
               } else
                  throw data_loading_exception( "fixedStep track without 'step' argument." );
               g_match_info_free( mi );

               if( g_regex_match( re_start, line.c_str(), GRegexMatchFlags( 0 ), &mi ) )
                  begin = from_string< long int >( g_match_info_fetch( mi, 1 ) ) - 1;
               else
                  throw data_loading_exception( "fixedStep track without 'start' argument." );
               g_match_info_free( mi );
               
            } else 
               wsf = variableStep;
            continue;
         }

         pair<int,int> pos[5];
         // Find first word
         pos[0] = find_word( line );            

         if( wsf == bed) {
            chromname = line.substr( pos[0].first, pos[0].second );
         }

         if( trackname + " // " + chromname != seqname )
            continue;
            
         // Find next words:
         for( int i = 1; i < 5; i++ ) {
            try {
               pos[i] = find_word( line, pos[i-1].first + pos[i-1].second );
            } catch( no_word_found_exception e ) {
               pos[i] = pair<int,int>( 0, 0 );
            }
         }

         long int end;
         double score;
         switch( wsf ) {
            case bed:
               begin  = from_string< long int >( line.substr( pos[1].first, pos[1].second ) );
               end    = from_string< long int >( line.substr( pos[2].first, pos[2].second ) ) - 1;
               try {
                  score  = from_string< double   >( line.substr( pos[3].first, pos[3].second ) );
               } catch( conversion_failed_exception e ) {
                  score  = from_string< double   >( line.substr( pos[4].first, pos[4].second ) );
                  // The BED format is ambiguous about whether the score is in the 4th or 5th column.
               }
               break;
            case variableStep:
               begin  = from_string< long int >( line.substr( pos[0].first, pos[0].second ) ) - 1;
               end    = begin + span;
               score  = from_string< double   >( line.substr( pos[1].first, pos[1].second ) );
               break;
            case fixedStep:
               score  = from_string< double   >( line.substr( pos[0].first, pos[0].second ) );
               begin  += step_interval;
               end    = begin + span;
               break;
         }

         sv->add_value( begin, end, score );
         if( end > maxidx )
            maxidx = end;
         
      } catch( conversion_failed_exception e ) {
         throw data_loading_exception( "Failed to parse a numerical value in the following line:\n" + line );
      }
   }
   infile.close();
   g_regex_unref( re_trackname );
   g_regex_unref( re_chromname );
   g_regex_unref( re_span );
   g_regex_unref( re_step );
   g_regex_unref( re_start );
   
   sv->max_index = maxidx;   
   return sv;
}

