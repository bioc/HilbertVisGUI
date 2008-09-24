// Simple, very light-weight C++ interface to regex.h

#ifndef _SIMPLE_REGEX_PP_H_
#define _SIMPLE_REGEX_PP_H_

#include <string>
#include <vector>
#include <regex.h>

// The following exceptions may be thrown:

struct regex_exception {};

struct regex_exception_compilation_failed : public regex_exception {
   std::string error_message;  // the message returned by regerror
};

struct regex_exception_no_match : public regex_exception {};

struct regex_match_t {
   int begin, length;   // position and length of matches substring
   std::string s;       // a copy of the substring
};


// Look for the _extended_ POSIX regex 'regex' in the string 's' and
// return matches in the vector 'res', which is emptied before being
// filled by the function. The flags are as defined in regex.h for
// compilation and execution. REG_EXTENDED is always set.
void regex_pp( const std::string regex, const std::string & s,  
   std::vector< regex_match_t > & res, int cflags = 0, int eflags = 0 );

// Check whether the extended POSIX regex 'regex' matches the string 's':
bool regex_match_pp( const std::string regex, const std::string & s,  
   int cflags = 0, int eflags = 0 );


#endif // _SIMPLE_REGEX_PP_H_
