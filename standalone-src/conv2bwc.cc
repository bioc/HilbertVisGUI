#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <zlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include "simple_regex_pp.h"

using namespace std;


class conversion_failed_exception {};

template <class T> T from_string( const string & s )
{
  std::istringstream iss( s );
  T t;
  iss >> t;
  if( iss.fail() )
     throw conversion_failed_exception( );
  return t;
}

void write_rep( gzFile f, gdouble v, guint32 rep )
{
   gzwrite( f, &v, sizeof( gdouble ) );
   gzwrite( f, &rep, sizeof( guint32 ) );
}

void make_valid_filename( char * buf ) 
{
   for( int i = 0; buf[i] != 0; i++ )
      if( ! g_ascii_isalnum( buf[i] ) && buf[i] != '.' )
         buf[i] = '_';
}

enum file_type_t { ext, wiggle, gff };  // 'ext' means: choose according to extension

enum track_type_t { bed, variableStep, fixedStep, gffTrack };

void conv2bwc( const string & infile_name, const string & outfile_prefix,
      map< string, long > & seq_lengths, bool force, file_type_t file_type )
{
   gzFile out_file = NULL;
   track_type_t track_type;
   int track_num = 0;   
   vector< regex_match_t > m;
   string chrom = "__no_chromosome__";
   string prev_chrom = "__invalid__";
   string track_name = "__unnamed__";
   long start;
   long stop;
   double value;
   long span;
   long step; 
   long pos;  
   
   ifstream infile( infile_name.c_str() );   
   if( !infile ) {
      cerr << "Error: Failed to open file " << infile_name << endl;
      exit( 1 );
   }

   if( file_type == ext ) {
      if( regex_match_pp( "\\.wig$", infile_name, REG_ICASE ) )
         file_type = wiggle;
      else if( regex_match_pp( "\\.gff$", infile_name, REG_ICASE ) )
         file_type = gff;
      else {
         cerr << "Error: Cannot determine file type of file '" << infile_name 
	    << "'\n   from its extension. Use option '-g' or '-w'.\n";
	 exit( 1 );
      }
   }   

   if( file_type == gff ) 
      track_type = gffTrack;	 
   else	 
      track_type = bed;

   cout << "Reading " << ( file_type==wiggle ? "wiggle" : "GFF") << " file " << infile_name << endl;

   long linecount = -1;
   while( infile ) {
      try {
	 string line;
	 getline( infile, line );
	 linecount++;
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
	 if( regex_match_pp( "^[[:space:]]*$", line ) )
            continue; 
	    
	 if( file_type == wiggle ) {
	    
	    // ignore 'browser' lines	 
	    if( regex_match_pp( "^[[:space:]]*browser", line, REG_ICASE ) )
               continue; 

	    if( regex_match_pp( "^[[:space:]]*track", line, REG_ICASE ) ) {
               if( ! regex_match_pp( "type[[:space:]]*=[[:space:]]*wiggle_0", 
		     line, REG_ICASE ) ) {
		  cerr << "Error: Track found which is not of type 'wiggle_0' in line " << 
	             linecount << ".\n";
		  exit( 1 );
	       }
	       track_num++;
	       try {
		  try {
  		     regex_pp( "name[[:space:]]*=[[:space:]]*\"([^\"]+)\"", line, m );
		  } catch( regex_exception_no_match e ) {
		     regex_pp( "name[[:space:]]*=[[:space:]]*([^[:space:]]+)", line, m );
		  }
		  track_name = m[1].s;
	       } catch( regex_exception_no_match e ) {
		  char buf[100];
		  snprintf( buf, 100, "__unnamed__%d__", track_num );
		  track_name = buf;
	       };
   	       chrom = "__no_chromosome__";
               prev_chrom = "__invalid__"; 
               track_type = bed; // might be changed again by the next line
	       cout << "Found track '" << track_name << "'." << endl;
	       continue;
	    }

	    if( regex_match_pp( "^[[:space:]]*fixedStep", line, REG_ICASE ) ) {
               track_type = fixedStep;

	       regex_pp( "chrom[[:space:]]*=[[:space:]]*([^[:space:]]+)", line, m );
               chrom = m[1].s;

	       regex_pp( "step[[:space:]]*=[[:space:]]*([[:digit:]]+)", line, m );
	       step = from_string<int>( m[1].s );

	       regex_pp( "start[[:space:]]*=[[:space:]]*([[:digit:]]+)", line, m );
	       start = from_string<int>( m[1].s ) - step - 1;

	       if( regex_match_pp( "span[[:space:]]*=", line ) ) {
  		  regex_pp( "span[[:space:]]*=[[:space:]]*([[:digit:]]+)",
		     line, m );
		  span = from_string<int>( m[1].s );
	       } else
		  span = 1;	 

	       continue;
	    }

	    if( regex_match_pp( "^[[:space:]]*variableStep", line, REG_ICASE ) ) {
               track_type = variableStep;

	       regex_pp( "chrom[[:space:]]*=[[:space:]]*([^[:space:]]+)", line, m );
               chrom = m[1].s;

	       if( regex_match_pp( "span[[:space:]]*=", line ) ) {
  		  regex_pp( "span[[:space:]]*=[[:space:]]*([[:digit:]]+)",
		     line, m );
		  span = from_string<int>( m[1].s );
	       } else
		  span = 1;	 

	       continue;
	    }      
         }

	 switch( track_type ) {

            case bed:
	       regex_pp( "^[[:space:]]*([^[:space:]]+)"
		  "[[:space:]]*([[:digit:]]+)"
		  "[[:space:]]*([[:digit:]]+)"
		  "[[:space:]]*([[:digit:]eE\\.+\\-]+)[[:space:]]*$",
		  line, m );
               chrom = m[1].s;
	       start = from_string<int>( m[2].s );
	       stop  = from_string<int>( m[3].s );
	       value = from_string<double>( m[4].s );
	       break;

	    case variableStep:
	       regex_pp( "^[[:space:]]*([[:digit:]]+)"
		  "[[:space:]]*([[:digit:]eE\\.+\\-]+)[[:space:]]*$",
		  line, m );
	       start = from_string<int>( m[1].s ) - 1;
	       stop = start + span;
	       value = from_string<double>( m[2].s );
	       break;    

	    case fixedStep:
	       regex_pp( "^[[:space:]]*([[:digit:]eE\\.+\\-]+)[[:space:]]*$",
		  line, m );
	       start += step;
	       stop = start + span;
	       value = from_string<double>( m[1].s );
	       break;    

            case gffTrack:
	       regex_pp( "^[[:space:]]*([^[:space:]]+)"   // seqname
	          "[[:space:]]+[^[:space:]]+"             // source
	          "[[:space:]]+[^[:space:]]+"             // feature
		  "[[:space:]]*([[:digit:]]+)"            // start
		  "[[:space:]]*([[:digit:]]+)"            // end
		  "[[:space:]]*([[:digit:]eE\\.+\\-]+)",  // score
		  line, m );
	       if( m[4].s == "." )
	          continue;  // Skip entries without score
               chrom = m[1].s;
	       start = from_string<int>( m[2].s );
	       stop  = from_string<int>( m[3].s );
	       value = from_string<double>( m[4].s );
	       break;

	 }

	 if( chrom != prev_chrom ) {
            if( out_file ) {
	       write_rep( out_file, 0., seq_lengths[prev_chrom] - pos );
	       gzclose( out_file );
	       out_file = NULL;
	    }
            cout << "Found chromosome '" << chrom << "' on track " << 
	       track_name << ": ";
	    if( seq_lengths.count( chrom ) ) {
               char buf[200];
	       if( file_type == wiggle )
		  snprintf( buf, 200, "%s_%s_%s.bwc", outfile_prefix.c_str(),
		     track_name.c_str(), chrom.c_str() );
	       else
		  snprintf( buf, 200, "%s_%s.bwc", outfile_prefix.c_str(),
		     chrom.c_str() );
	       make_valid_filename( buf );
	       if( !force ) {
	          if( FILE * f = fopen( buf, "r" ) ) {
		     fclose( f );
		     cerr << "\nError: Cannot write to file name '" << buf << "' because a file of\n";
		     cerr << "  this name already exists. Use '-f' to force overwriting.\n";
		     exit( 1 );
		  }
	       }
	       out_file = gzopen( buf, "wb" );	
	       cout << "Writing to file '" << buf << "'." << endl;
	       gzputs( out_file, "BWC1\n" );
	       guint32 l = seq_lengths[chrom];
	       gzwrite( out_file, &l, sizeof( guint32 ) );
	       snprintf( buf, 200, "%s: %s\n", 
		  track_name.c_str(), chrom.c_str() );
	       gzputs( out_file, buf );
               pos = 0;      
	    } else {
	       cout << "Skipping (no length given)." << endl;
	    }
            prev_chrom = chrom;
	 }
	 if( out_file ) {
            if( start - pos < 0 ) {
	       cerr << endl << start << " " << pos << endl;
	       cerr << "Error: The entries in this file are overlapping or not ordered by coordinates.\n"
	          "   The present implementation of this tool cannot handle this. Sorry.\n"
                  "   You can, however,  use the 'makeWiggleVector' function in the R package version\n"
		  "    of HilbertCurveDisplay.\n"
		  "   (Error occured in line " << linecount << " of input file.)\n";
	       exit( 1 );
	    }
	    write_rep( out_file, 0., start - pos );
	    write_rep( out_file, value, stop - start );
	    pos = stop;
	    if( pos > seq_lengths[chrom] ) {
	       cerr << "Error: Found coordinate beyond the given length (line " << 
	          linecount << ").\n";
	       exit( 1 );
	    }
	 }
      } catch( regex_exception_no_match e ) {
         cerr << "Error: Parse error in line " << linecount << ".\n";
	 exit( 1 );
      }
   }   
   infile.close();
   if( out_file ) {
      write_rep( out_file, 0., seq_lengths[chrom] - pos );	 
      gzclose( out_file );
      out_file = NULL;
   }
}


int main( int argc, char * argv[] ) 
{
   map< string, long > seqlens;   
   char opt;
   vector< regex_match_t > m;
   string prefix;
   bool force = false;
   file_type_t file_type = ext;

   cout << "wig2bwc: Converts wiggle (.wig) files to binary wiggle \n"
   "chromosome (.bwc) files, to be used with hilbertCurveDisplay.\n"
   "(c) Simon Anders, European Bioinformatics Institute (EMBL-EBI)\n"
   "sanders@fs.tum.de, version 0.99.3 of 2008-08-20\n"
   "Released under GNU General Public License (GPL) version 3\n\n";
   cout.flush();
   

   while( (opt = getopt(argc, argv, "l:L:hp:fwg") ) != -1 )
      switch( opt ) {
         case 'l':
	    try{
	       regex_pp( "^[[:space:]]*([^=]+)=([[:digit:]]+)[[:space:]]*", optarg, m);
	    } catch( regex_exception_no_match e ) {
	       cerr << "Cannot parse chromsome length specification '" << optarg << "'.\n";
	       return 1;
	    }
	    seqlens[ m[1].s ] = from_string<int>( m[2].s );
	    break;
	 case 'L': {
	    ifstream slfile( optarg );   
	    if( !slfile ) {
	       cerr << "Error: Failed to open sequence/length file " << optarg << endl;
	       exit( 1 );
	    }
	    try{
	       string line;
	       while( getline( slfile, line ) ) {
		  regex_pp( "^[[:space:]]*([^=]+)=([[:digit:]]+)[[:space:]]*", line, m);
  		  seqlens[ m[1].s ] = from_string<int>( m[2].s );
	       }
	    } catch( regex_exception_no_match e ) {
               cerr << "Error: Failed to parse sequence/length file " << optarg << ".\n";
	       exit( 1 );
	    } 
	    slfile.close();
	    break; }
	 case 'p':
	    prefix = optarg;
	    break;
	 case 'w':
	    file_type = wiggle;
	    break;
	 case 'g':
	    file_type = gff;
	    break;
         case 'h':
	    cout << "Usage: conv2bwc [options] <wiggle or GFF file(s)>\n\n"
	       "Options:\n"
	       "  -l <seqname>=<length>   Extract the sequence (typically: the chromosome)\n"
	       "           that is denoted as <seqname> in the wiggle file(s), which has a\n"
	       "           length of <length> base pairs. Do not put spaces around the '='\n"
	       "           sign.\n"
	       "  -L <filename>   Read a list of sequences to be extracted with their names\n"
	       "           from the file <filename>. This file should contain one line per\n"
	       "           sequence, in the format: <seqname>=<length>\n"
	       "  -p <prefix>   Use <prefix> as prefix for the output filenames. By default\n"
	       "           the name of the wiggle file, stripped of the '.wig' extension, \n"
	       "           is used.\n"
	       "  -f       Allow overwriting of existing files.\n"
	       "  -g       Treat all files as GFF files. (Default: Only files with extension\n"
	       "           '.gff' are considered GFF files.\n"
	       "  -w       Treat all files as wiggle files. (Default: Only files with extension\n"
	       "           '.wig' are considered wiggle files.\n"
	       "  -h       Display this help message.\n";
	    return 0;
	 case 'f':
	    force = true;
	    break;
	 case '?':
	    return 1;
	 default:
	    cerr << "Error parsing command line.\n";
	    return 1;
      }
   if( seqlens.size() == 0 ) {
      cerr << "Error: You must specify at least on sequence length with the '-l' or '-L' option.\n";
      return 1;
   }   
   if( optind >= argc ) {
      cerr << "Error: You must specify at least one wiggle file.\n";
      return 1;
   }   
   if( prefix.length() > 0 && argc - optind > 1) {
      cerr << "Error: You must not specify more than one wiggle file when using the '-p' option.\n";
      return 1;
   }   

   for( int i = optind; i < argc; i++ ) {
      if( prefix.length() == 0 ) {
         // Remove path
         char buf[200];
	 strncpy( buf, argv[i], 200 );
         prefix = basename( buf );
	 // Remove '.wig' or '.gff' if present:
	 if( regex_match_pp( "\\.(wig|gff)$", argv[i]) )
	    prefix = prefix.substr( 0, prefix.length() - 4 );
      }
      conv2bwc( argv[i], prefix, seqlens, force, file_type );
   }
}
