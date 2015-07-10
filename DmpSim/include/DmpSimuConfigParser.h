/*
 *   Class to pass the configuration parameters for DAMPE reconstruction algorithms
 *   Created by:    A. Tykhonov
 *   adopted from:  https://code.google.com/p/inih/
 *   date:          04/03/2014
 */

#ifndef DMPSIMUCONFIGPARSER_H
#define DMPSIMUCONFIGPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>
#include <ctype.h>




/*
 *   Auxiliary functions
 */


/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

http://code.google.com/p/inih/

*/

#ifndef __INI_H__
#define __INI_H__

/* Make this header file easier to include in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Parse given INI-style file. May have [section]s, name=value pairs
   (whitespace stripped), and comments starting with ';' (semicolon). Section
   is "" if name=value pair parsed before any section heading. name:value
   pairs are also supported as a concession to Python's ConfigParser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). Handler should return nonzero on success, zero on error.

   Returns 0 on success, line number of first error on parse error (doesn't
   stop on first error), -1 on file open error, or -2 on memory allocation
   error (only when INI_USE_STACK is zero).
*/
int ini_parse(const char* filename,
              int (*handler)(void* user, const char* section,
                             const char* name, const char* value),
              void* user);

/* Same as ini_parse(), but takes a FILE* instead of filename. This doesn't
   close the file when it's finished -- the caller must do that. */
int ini_parse_file(FILE* file,
                   int (*handler)(void* user, const char* section,
                                  const char* name, const char* value),
                   void* user);

/* Nonzero to allow multi-line value parsing, in the style of Python's
   ConfigParser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See http://code.google.com/p/inih/issues/detail?id=21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Nonzero to use stack, zero to use heap (malloc/free). */
#ifndef INI_USE_STACK
#define INI_USE_STACK 1
#endif

/* Stop parsing on first error (default is to keep parsing). */
#ifndef INI_STOP_ON_FIRST_ERROR
#define INI_STOP_ON_FIRST_ERROR 0
#endif

/* Maximum line length for any line in INI file. */
#ifndef INI_MAX_LINE
#define INI_MAX_LINE 200
#endif

#ifdef __cplusplus
}
#endif

#endif /* __INI_H__ */



// *******************************************************************
// *
// *   Config parser class
// *
// *******************************************************************



static inline std::string &ltrim(std::string &s);
static inline std::string &rtrim(std::string &s);
static inline std::string &trim(std::string &s);
void StringExplode(std::string str, std::string separator, std::vector<std::string>* results, bool Trim);


#include "DmpVSvc.h"

using namespace std;

class DmpSimuConfigParser: public DmpVSvc{

public:
  DmpSimuConfigParser();
  ~DmpSimuConfigParser();
  bool Initialize();
  bool Finalize();
  void Set(const std::string&,const std::string&);



  // *
  // * Genereal parsing fucntions
  // *

  // Return the result of ini_parse(), i.e., 0 on success, line number of
  // first error on parse error, or -1 on file open error.
  int ParseError();

  // Get a string value from INI file, returning default_value if not found.
  std::string Get(std::string section, std::string name, std::string default_value);

  // Get an integer (long) value from INI file, returning default_value if
  // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
  long GetInteger(std::string section, std::string name, long default_value);

  // Get a real (floating point double) value from INI file, returning
  // default_value if not found or not a valid floating point value
  // according to strtod().
  double GetReal(std::string section, std::string name, double default_value);

  // Get a boolean value from INI file, returning default_value if not found or if
  // not a valid true/false value. Valid true values are "true", "yes", "on", "1",
  // and valid false values are "false", "no", "off", "0" (not case sensitive).
  bool GetBoolean(std::string section, std::string name, bool default_value);

  // Return true if parameter is defined
  bool Find(string section, string name);

  // Get an a array of strings from INI file, returning default_value if not found or if
  // Values should be delimited with ","
  // If none, empty array is returned
  std::vector<std::string> GetStringArray(std::string section, std::string name);

  // *
  // * DAMPE related functions
  // *
  void   Parse();

  //* STK:
  //std::string GetSTKMoreSteppingVolumes();
  std::vector<std::string> GetSTKMoreSteppingVolumes();





private:
  int _error;
  std::map<std::string, std::string> _values;
  static std::string MakeKey(std::string section, std::string name);
  static int ValueHandler(void* user, const char* section, const char* name, const char* value);
  std::string fSimuConfigFile;

};

#endif //DMPSIMUCONFIGPARSER_H







