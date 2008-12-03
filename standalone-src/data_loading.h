#ifndef __DATA_LOADING_H__
#define __DATA_LOADING_H__

#include<set>
#include<string>
#include "step_vector.h"

using namespace std;

set< string > get_gff_toc( const string & filename );

step_vector<double> * load_gff_data( const string & filename, const string & seqname );


class conversion_failed_exception {};

template <class T> T from_string( const string & s );


#endif //__DATA_LOADING_H__
