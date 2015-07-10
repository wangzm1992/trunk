/*
 *   Class to pass the configuration parameters for DAMPE reconstruction algorithms
 *   Created by:    A. Tykhonov
 *   adopted from:  https://code.google.com/p/inih/
 *   date:          04/03/2014
 */


// Read an INI file into easy-to-access name/value pairs.

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>


#include <algorithm>
#include <locale>

#include "DmpSimuConfigParser.h"

//#include <ini.h>

//#define fSimuConfigFile              "dmpSimu.cfg"
#define DAMPECONFIG_PATH_ENVVARIABLE  "DMPSWCONFIGPATH"


/* inih -- simple .INI file parser

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

http://code.google.com/p/inih/

*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

//#include "ini.h"

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char c or ';' comment in given string, or pointer to
   null at end of string if neither found. ';' must be prefixed by a whitespace
   character to register as a comment. */
static char* find_char_or_comment(const char* s, char c)
{
    int was_whitespace = 0;
    while (*s && *s != c && !(was_whitespace && *s == ';')) {
        was_whitespace = isspace((unsigned char)(*s));
        s++;
    }
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size);
    dest[size - 1] = '\0';
    return dest;
}

/* See documentation in header file. */
int ini_parse_file(FILE* file,
                   int (*handler)(void*, const char*, const char*,
                                  const char*),
                   void* user)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
#else
    char* line;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_MAX_LINE);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through file line by line */
    while (fgets(line, INI_MAX_LINE, file) != NULL) {
        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (*start == ';' || *start == '#') {
            /* Per Python ConfigParser, allow '#' comments at start of line */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-black line with leading whitespace, treat as continuation
               of previous name's value (as per Python ConfigParser). */
            if (!handler(user, section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_char_or_comment(start + 1, ']');
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start && *start != ';') {
            /* Not a comment, must be a name[=:]value pair */
            end = find_char_or_comment(start, '=');
            if (*end != '=') {
                end = find_char_or_comment(start, ':');
            }
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = lskip(end + 1);
                end = find_char_or_comment(value, '\0');
                if (*end == ';')
                    *end = '\0';
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler(user, section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

/* See documentation in header file. */
int ini_parse(const char* filename,
              int (*handler)(void*, const char*, const char*, const char*),
              void* user)
{
    FILE* file;
    int error;

    file = fopen(filename, "r");
    if (!file)
        return -1;
    error = ini_parse_file(file, handler, user);
    fclose(file);
    return error;
}


// trim from start
static inline std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}

// split
void StringExplode(std::string str, std::string separator, std::vector<std::string>* results, bool Trim = false){
  int found;
  found = str.find_first_of(separator);
  while(found != std::string::npos){
    if(found > 0){
      std::string value = str.substr(0,found);
      if(Trim) value = trim(value);
      if(value.size()) results->push_back(value);
    }
    str = str.substr(found+1);
    found = str.find_first_of(separator);
  }
  if(str.length() > 0){
    if(Trim) str = trim(str);
    if(str.size())results->push_back(str);
  }
}



using std::string;

#include "DmpLog.h"

// *
// * general parser functions
// *

DmpSimuConfigParser::DmpSimuConfigParser():
    DmpVSvc("DmpSimuConfigParser"),
    fSimuConfigFile("dmpSimu.cfg")
{
}

DmpSimuConfigParser::~DmpSimuConfigParser()
{
}

bool DmpSimuConfigParser::Initialize(){
  DmpLogInfo<<"|"<<DmpLogEndl;
  DmpLogInfo<<"|   Using configuration simulation file: "<<fSimuConfigFile<<DmpLogEndl;
  DmpLogInfo<<"|"<<DmpLogEndl;

  try{
    Parse();
  }
  catch (...) {
    DmpLogError<<"[DmpSimuConfigParser::Initialize] Can not parse configuration ==> throwing exception!" << DmpLogEndl;
    throw;
  }
  return true;
}

bool DmpSimuConfigParser::Finalize(){
  return true;
}


void DmpSimuConfigParser::Set(const std::string& arg,const std::string& val){
  if(std::string("ConfigFile") == arg){
    fSimuConfigFile = val;
    return;
  }

  DmpLogError<<"Unknown argument type: "<< arg << " ==> throwing exception!"<<DmpLogEndl;
  throw;
}

void DmpSimuConfigParser::Parse()
{
  //std::string filename("../share/Configuration/dmpReco.cfg");
  //_error = ini_parse(filename.c_str(), ValueHandler, this);


  // * Obtain location of DAMPE configuration folder
  char* configPath = NULL;
  configPath = getenv (DAMPECONFIG_PATH_ENVVARIABLE);
  if (configPath==NULL){
    std::ostringstream errorstr;
    errorstr<<"[DmpSimuConfigParser::Parse] ERROR: configuration path environment variable"<<DAMPECONFIG_PATH_ENVVARIABLE<<" is not defined!";
    std::string error(errorstr.str());
    DmpLogError<<error<<DmpLogEndl;
    throw error;
  }

  // * Parse config file
  std::ostringstream filename;
  filename<<configPath<<"/"<<fSimuConfigFile.c_str();
  _error = ini_parse(filename.str().c_str(), ValueHandler, this);


  //* error handler
  if(_error < 0) {
    DmpLogError << "[DmpSimuConfigParser::Initialize] ERROR Can't load configuration file: "<< filename.str() << DmpLogEndl;
    throw new std::string("[DmpSimuConfigParser::Initialize] ERROR can't load configuration file");;
  }

}

int DmpSimuConfigParser::ParseError()
{
    return _error;
}

string DmpSimuConfigParser::Get(string section, string name, string default_value)
{
    string key = MakeKey(section, name);
    return _values.count(key) ? _values[key] : default_value;
}

long DmpSimuConfigParser::GetInteger(string section, string name, long default_value)
{
    string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    // This parses "1234" (decimal) and also "0x4D2" (hex)
    long n = strtol(value, &end, 0);
    if (end <= value){
      std::ostringstream error;
      error<<"parameter not found in the configuration file: section = [  "<< section << " ] parameter name = ["<< name <<"]";
      DmpLogError<<error.str()<<DmpLogEndl;
      throw error.str();
    }
    return end > value ? n : default_value;
}
double DmpSimuConfigParser::GetReal(string section, string name, double default_value)
{
    string valstr = Get(section, name, "");
    const char* value = valstr.c_str();
    char* end;
    double n = strtod(value, &end);
    if (end <= value){
      std::ostringstream error;
      error<<"parameter not found in the configuration file: section = [  "<< section << " ] parameter name = ["<< name <<"]";
      DmpLogError<<error.str()<<DmpLogEndl;
      throw error.str();
    }
    return end > value ? n : default_value;
}

bool DmpSimuConfigParser::GetBoolean(string section, string name, bool default_value)
{
    string valstr = Get(section, name, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
    if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        return true;
    else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        return false;
    else
        return default_value;
}


bool DmpSimuConfigParser::Find(string section, string name)
{
	string NOT_FOUND = "NOT_FOUND_______________";
	string result = Get(section, name, NOT_FOUND);
	if(result == NOT_FOUND)
		return false;
	return true;
}


std::vector<std::string> DmpSimuConfigParser::GetStringArray(string section, string name){
  std::vector<std::string> values;
  std::string value = Get(section, name, "");
  StringExplode(value,",",&values,true);
  return values;
}

string DmpSimuConfigParser::MakeKey(string section, string name)
{
    string key = section + "." + name;
    // Convert to lower case to make section/name lookups case-insensitive
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key;
}

int DmpSimuConfigParser::ValueHandler(void* user, const char* section, const char* name,
                            const char* value)
{
  DmpSimuConfigParser* reader = (DmpSimuConfigParser*)user;
  reader->_values[MakeKey(section, name)] = value;
  return 1;
}



// *
// * STK parameters
// *

//string DmpSimuConfigParser::GetSTKMoreSteppingVolumes(){
  //return Get("STK", "MoreSteppingVolumes", "");
std::vector<std::string> DmpSimuConfigParser::GetSTKMoreSteppingVolumes(){
  return GetStringArray("STK", "MoreSteppingVolumes");
}


