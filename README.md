# ls-process-arguments
command line arguments processor, similar to argp but with callbacks and easier to quickly implement.

1. #include "ls-process-arguments.hpp" im your main program file.
2. define a userdata type [optional]
3. define switch (and argument) callbacks [optional]
4. define switch (and argument) definitions
5. call process_arguments_usage from a place that access to argc and argv command line arguments

simple static example:
```c++
using namespace ls;

/* global of argv[0] and argv */
char *progname, **arg_list;

/* userdata structure (extends common usage struture) */

struct userdata_s : process_arguments_state_s {
  int pass_count = 0;
  int fail_count = 0;
  int error_count = 0;
  int all_count = 0;
  
  userdata_s ( )
      : pass_count( 0 )
      , fail_count( 0 )
      , error_count( 0 )
      , all_count( 0 ) {
    std::cerr << "userdata_s constructed" << std::endl;
  }
  
  ~userdata_s ( ) {
    std::cerr << "userdata_s destructed" << std::endl;
  }
  
};

/*
 * switch callback definitions
 */

int cb_usage ( int key, char* arg
      , int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s* argdef, void* userdata_p ) {
  process_arguments_usage( progname, argdefs );
    
  exit( 0 );
}

int cb_help ( int key, char* arg
      , int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s* argdef, void* userdata_p ) {
  process_arguments_help( progname, argdefs
      , "process-arguments-test -- a test program for process-arguments.hpp" );
    
  exit( 0 );
}

int cb_verbosity ( int key, char* arg
      , int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s* argdef, void* userdata_p ) {
  struct userdata_s &userdata = *((struct userdata_s*)userdata_p);
  char *param = process_arguments_next_plain_argument( arg_list, argi, argc, arg, c_argi, c_argc );
  if ( ! param || *param < '0' || *param > '5' ) return ARGDEFS_ERROR;
  ((struct userdata_s*)userdata_p)->verbosity = *param - '0';
  return 0;
}

int cb_show_state ( int key, char* arg
      , int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s* argdef, void* userdata_p ) {
    userdata_s& settings = *((userdata_s*)userdata_p);
  
  process_arguments_show_state( userdata_p );
  printf( "  %-12s => %d\n", "pass_count", settings.pass_count );
  printf( "  %-12s => %d\n", "fail_count", settings.fail_count );
  printf( "  %-12s => %d\n", "error_count", settings.error_count );
  printf( "  %-12s => %d\n", "all_count", settings.all_count );
  
  return 0;
}

/* array of switch (and argument) definitions */
struct argdef_s argdefs[] = {
    { '?',  0,              0,        "prints this message",          cb_help }
  , { 'h',  "help",         0,        "prints this message",          cb_help }
  , { 'u',  "usage",        0,        "prints command summary",       cb_usage }
  , { 'v',  "verbosity",    "[0-5]",  "set verbosity",                cb_verbosity }
  , { 1,    "show-state",   0,        "print current state values",   cb_show_state }
  , { ARGDEFS_END }
};

int main ( int arge, char* argv[] ) {
  struct userdata_s userdata;
  void *userdata_p = (void*)&userdata;
  std::stringstream ss;
  int argi = 1
      , argc = process_arguments_find_end( argv, argi, arge )
      , i = 0;
  int c_argi = 0, c_argc = 0, last_argi;
  char *arg;
  
  progname = process_arguments_progname( argv[0] );
  arg_list = argv;
  
  while ( argi < argc ) {
    last_argi = argi;
    std::cout << "pass#" << i++ << ": " << argv[argi] << ", " << argi << ", " 
        << argc << ", " << c_argi << ", " << c_argc << std::endl;
    if ( ! process_arguments( argv, argi, argc, c_argi, c_argc, argdefs, userdata_p ) ) break;
    if ( last_argi == argi ) break;
  }
  std::cout << "done@" << i << ": " << argv[argi] << ", " << argi << ", " 
      << argc << ", " << c_argi << ", " << c_argc << std::endl;
      
  return userdata.all_count != userdata.pass_count;
}


```
