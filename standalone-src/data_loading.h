#ifndef __DATA_LOADING_H__
#define __DATA_LOADING_H__

#include<set>
#include<string>
#include "step_vector.h"

using namespace std;

set< string > get_gff_toc( const string & filename );
set< string > get_maqmap_toc( const string & filename );
set< string > get_maqmap_old_toc( const string & filename );

step_vector<double> * load_gff_data( const string & filename, const string & seqname );
step_vector<double> * load_maqmap_data( const string & filename, const string & seqname, int minqual=0 );
step_vector<double> * load_maqmap_old_data( const string & filename, const string & seqname, int minqual=0 );

class data_loading_exception : public std::string {
  public:
   data_loading_exception( std::string what = "Error while loading data file")
      : string(what) {};
};

class conversion_failed_exception {};

template <class T> T from_string( const string & s );


#endif //__DATA_LOADING_H__
