/*

Copyright (c) <2015> <Erich Horn and LaughingSun>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE


 */



#ifndef PROCESS_ARGUMENTS_HPP
#define PROCESS_ARGUMENTS_HPP

#ifndef QUOTE
#define _QUOTE(a) #a
#define QUOTE(a) _QUOTE(a)
#endif
//#pragma message ( "test/process-arguments.hpp" )

#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <malloc.h>
#include <exception>
#include <stdexcept>

namespace ls {

  enum argument_reserved_keys_e {
    ARGDEFS_END           = 0     // end of definitions
    
    , ARGDEFS_STDIO       = '-'   // handler for standard io argument ('-')
    
    , ARGDEFS_EXITREQ     =  3    // exit request
    
    , ARGDEFS_SWITCH      = -1    // handler for undefined switch
    , ARGDEFS_ARGUMENT    = -2    // handler for non-switch argument
    , ARGDEFS_UNDEF       = -3    // handler for undefined anything
    , ARGDEFS_ERROR       = -4    // handler for result error
    
  };

  enum argument_flags_e {
    ARGFLAGS_DEFAULT      = 0x00000 // use defaults
    
    , ARGFLAGS_IMMED      = 0x00001 // process immediately
    , ARGFLAGS_EVERY      = 0x00002 // process on every pass
    , ARGFLAGS_REQUIRE    = 0x00004 // is a required switch
    , ARGFLAGS_REQEXIT    = 0x00008 // request exit after switch is processed
    
    , ARGFLAGS_USER       = 0x10000 // user defined flags
    
  };

  struct argdef_s;
  typedef int (*argument_callback_t)( int key, char* arg
      , int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s* argdef, void* userdata_p );

  struct argdef_s {
    int key;
    const char* name;
    const char* args;
    const char* desc;
    argument_callback_t callback;
    unsigned int flags;
    
    argdef_s (
        int key_ = ARGDEFS_END
        , const char* name_ = 0
        , const char* args_ = 0
        , const char* desc_ = 0
        , argument_callback_t callback_ = 0
        , int flags_ = ARGFLAGS_DEFAULT )
    : key( key_ )
    , name( name_ )
    , args( args_ )
    , desc( desc_ )
    , callback( callback_ )
    , flags( flags_ ) {
    }
    
  };

  typedef enum verbosity {
    v_silent
    , v_failure
    , v_warn
    , v_info
    , v_verbose
    , v_debug
  } verbosity_e;

  const char* argument_applicators = "=:+-";
  const char* argument_seperators = ",;&";

  struct process_arguments_state_s {
    const char *in_name = "stdin";
    const char *out_name = "stdout";
    const char *err_name = "stderr";
    const char *nil_name = "stdnil";
    std::ifstream* in_file = 0;
    std::ofstream* out_file = 0;
    std::ofstream* err_file = 0;
    std::ofstream* nil_file = 0;
    std::istream* in = &std::cin;
    std::ostream* out = &std::cout;
    std::ostream* err = &std::cerr;
    std::ostream* nil = new std::ostream( 0 );
    int verbosity = v_failure;
    
    process_arguments_state_s ( ) 
        : in_name( "stdin" )
        , out_name( "stdout" )
        , err_name( "stderr" )
        , nil_name( "stdnil" )
        , in_file( 0 )
        , out_file( 0 )
        , err_file( 0 )
        , in( &std::cin )
        , out( &std::cout )
        , err( &std::cerr )
        , nil( new std::ostream( 0 ) )
        , verbosity( v_failure ) {
//      std::cerr << "process_arguments_state_s constructed" << std::endl;
    }
    
    ~process_arguments_state_s ( ) {
//      std::cerr << "process_arguments_state_s destructed" << std::endl;
      if ( in_file ) in_file->close( );
      if ( out_file ) in_file->close( );
      if ( err_file ) err_file->close( );
      
    }
    
  };
  
  char _build_msg_buffer[240];
  inline const char* _build_msg ( const char* file, int line, const char* format, ... ) {
    size_t p = 0;
    va_list args;
    va_start( args, format );
    p += vsnprintf( _build_msg_buffer, sizeof( _build_msg_buffer ) - p, format, args );
    p += snprintf( _build_msg_buffer + p, sizeof( _build_msg_buffer ) - p, "@%s:%d", file, line );
    va_end(args);
    
    return (const char*)_build_msg_buffer;
  }
  
  inline char process_arguments_isapplicator( char chr ) {
    const char* c_ptr = strchr( argument_applicators, chr );
    return c_ptr ? *c_ptr : 0;
  }

  inline char process_arguments_isseperators( char chr ) {
    const char* c_ptr = strchr( argument_seperators, chr );
    return c_ptr ? *c_ptr : 0;
  }

  int process_arguments_find_applicator ( const char* arg, int c_argi ) {
    const char* end = strpbrk( arg + c_argi, argument_applicators );
    return end ? (end - arg) : strlen( arg );
  }

  const struct argdef_s* process_arguments_find_named_argdef ( const char* name
      , const struct argdef_s* argdefs, int c_argi, int c_argc ) {
    
//    std::cout << "looking for " << name << std::endl;
    while ( argdefs->key && ( ! argdefs->name || strcmp( name + c_argi, argdefs->name) ) ) {
//      std::cout << "+ miss  " << argdefs->key << " : " << (argdefs->name ? argdefs->name : "(null)") << std::endl;
      argdefs++;
    }
//    std::cout << "+ found " << argdefs->key << " : " << (argdefs->name ? argdefs->name : "(null)") << std::endl;
    
    return argdefs;
  }

  const struct argdef_s* process_arguments_find_keyed_argdef ( int key
      , const struct argdef_s* argdefs ) {
//    std::cout << "looking for " << key << std::endl;
    while ( argdefs->key && key != argdefs->key ) {
//      std::cout << "+ miss  " << argdefs->key << " : " << (argdefs->name ? argdefs->name : "(null)") << std::endl;
      argdefs++;
    }
//    std::cout << "+ found " << argdefs->key << " : " << (argdefs->name ? argdefs->name : "(null)") << std::endl;
    
    return argdefs;
  }

  char* process_arguments_next_plain_argument ( 
      char* argv[], int& argi, int& argc
      , char* arg, int& c_argi, int& c_argc
      ) {
    int i = argi, chr;
    
    if ( arg && c_argc < strlen( arg ) 
        && ( process_arguments_isapplicator( chr = arg[c_argc] )
            || process_arguments_isseperators( chr ) ) ) {
      return arg + (++c_argc);
    }
    
    while ( i < argc ) {
      if ( argv[i][0] != '-' ) {
        argi = i + 1;
        return argv[i];
      }
      i++;
    }
    
    return 0;
  }

  int process_arguments_find_end ( char* argv[], int i, int c, const char* terminator = "--" ) {
    while ( i < c ) {
      if ( 0 == strcmp( terminator, argv[i] ) ) {
        c = i;
        break;
      }
      i++;
    }
    
    return c;
  }

  bool process_arguments ( char* argv[], int& argi, int& argc
      , int& c_argi, int& c_argc
      , const struct argdef_s argdefs[], void* userdata ) {
    const struct argdef_s *argdef, *def
        , *argdef_switch = process_arguments_find_keyed_argdef( ARGDEFS_SWITCH, argdefs )
        , *argdef_argument = process_arguments_find_keyed_argdef( ARGDEFS_ARGUMENT, argdefs )
        , *argdef_stdio = process_arguments_find_keyed_argdef( ARGDEFS_STDIO, argdefs )
        , *argdef_undef = process_arguments_find_keyed_argdef( ARGDEFS_UNDEF, argdefs )
        , *argdef_error = process_arguments_find_keyed_argdef( ARGDEFS_ERROR, argdefs )
        ;
    std::stringstream ss;
    char* arg = 0;
    int errnum = 0, key, i, c;
    int last_argi, last_c_argi; 
    
    last_argi = -1;
    while ( argi < argc ) {
      if ( last_argi == argi && last_c_argi == c_argi ) {
//        throw std::invalid_argument( "last_c_argi == c_argi" );
        std::cout << "last_argi == argi && last_c_argi == c_argi";
        return false;
      }
      last_argi = argi;
      
      if ( ! (arg = argv[argi++])[0] ) continue;
      if ( arg[0] == '-' ) {
        if ( arg[1] == '-' ) {
          
          // named switch
          
          c_argi = 2;
          c_argc = process_arguments_find_applicator( arg, c_argi );
          argdef = def = process_arguments_find_named_argdef( arg, argdefs, c_argi, c_argc );
          if ( ! def || ! (key = def->key) || ! def->callback ) {
            if ( ! (def = argdef_switch)->callback && ! (def = argdef_undef)->callback ) {
              throw std::invalid_argument( _build_msg( __FILE__, __LINE__, "undefined named switch and no fallback for \"%s\"", arg ) );
            }
          }
          if ( 0 > (errnum = def->callback( key, arg, argi, argc, c_argi, c_argc, argdef, userdata )) 
              && ( ! (def = argdef_error)->callback 
              || 0 > (errnum = def->callback( key, arg, argi, argc, c_argi, c_argc, argdef, userdata )) ) ) {
            throw std::invalid_argument( _build_msg( __FILE__, __LINE__, "named switch, result error for \"%s\"", arg ) );
          }
          
          continue;
        }
        
        if ( arg[1] ) {
          
          // keyed switch
          
          c_argi = 1;
          c_argc = process_arguments_find_applicator( arg, c_argi );
          while ( c_argi < c_argc ) {
            last_c_argi = c_argi;
            argdef = def = process_arguments_find_keyed_argdef( key = arg[c_argi++], argdefs );
            if ( ! def || ! def->key || ! def->callback ) {
              if ( ! (def = argdef_switch)->callback && ! (def = argdef_undef)->callback ) {
               throw std::invalid_argument(  "keyed switch@" __FILE__ ":" QUOTE(__LINE__) );
              }
            }
            if ( 0 > (errnum = def->callback( key, arg, argi, argc, c_argi, c_argc, argdef, userdata )) 
                && ( ! (def = argdef_error)->callback 
                || 0 > (errnum = def->callback( key, arg, argi, argc, c_argi, c_argc, argdef, userdata )) ) ) {
              throw std::invalid_argument(  "keyed switch@" __FILE__ ":" QUOTE(__LINE__) );
            }
            
            if ( last_c_argi == c_argi ) {
//              throw std::invalid_argument( "last_c_argi == c_argi" );
              std::cout << "last_c_argi == c_argi";
              return false;
            }
          } // while ( c_argi < c_argc )
          
          continue;
        }
        
        // stdio ('-')
        if ( (def = argdef_stdio)->callback ) {
          if ( 0 > (errnum = def->callback( ARGDEFS_STDIO, arg, argi, argc, c_argc, c_argi, argdef_stdio, userdata )) ) {
             throw std::invalid_argument( "stdio switch@" __FILE__ ":" QUOTE(__LINE__) );
          }
          continue;
        }

      }
      
      // argument 
      if ( (def = argdef_argument)->callback || (def = argdef_undef)->callback ) {
        if ( 0 > (errnum = def->callback( key, arg, argi, argc, c_argi, c_argc, argdef_argument, userdata )) ) {
          if ( ! (def = argdef_error)->callback
              || 0 > (errnum = def->callback( errnum, arg, argi, argc, c_argc, c_argi, argdef_argument, userdata )) ) {
            throw std::invalid_argument( _build_msg( __FILE__, __LINE__, "argument param \"%s\"", arg ) );
          }
        }
        continue;
      }
      
      // undefined
      return false;

    }
    
    return true;
  }
  
  char* process_arguments_progname ( char* argv_0 ) {
#ifdef _WIN32
    char* c_ptr = strrchr( argv_0, '\\' );
#else
    char* c_ptr = strrchr( argv_0, '/' );
#endif
    return c_ptr ? c_ptr + 1 : argv_0;
  }
  
  void process_arguments_usage ( const char* progname
      , const struct argdef_s* argdefs ) {
    const struct argdef_s* def;
    int i, j, l;
    
    // short version
    std::cout << "usage: " << progname;
    l = 0;
    while ( (def = argdefs++)->key ) {
      if ( isprint( def->key ) ) {
        if ( ! l ) std::cout << " [-";
        l = true;
        std::cout << (char)def->key;
        continue;
      }
      
      if ( l ) std::cout << "]";
      l = false;
      
      if ( def->name ) {
        std::cout << " [--" << def->name << "]";
      }
    }
    if ( l ) std::cout << "]";
    std::cout << std::endl;
  }
  
  void process_arguments_max_widths ( const struct argdef_s* defs,
      int& name_len, int& args_len, int& desc_len ) {
    int l;
    name_len = args_len = desc_len = 0;
    while ( defs->key ) {
      if ( defs->name && name_len < (l = strlen( defs->name )) ) name_len = l;
      if ( defs->args && args_len < (l = strlen( defs->args )) ) args_len = l;
      if ( defs->desc && desc_len < (l = strlen( defs->desc )) ) desc_len = l;
      defs++;
    }
  }
  
  void process_arguments_help ( const char* progname
      , const struct argdef_s* argdefs
      , const char* title
      , const char* footer = 0 ) {
    const struct argdef_s* def;
    int name_len, args_len, desc_len;
    
    // long version
    
    process_arguments_max_widths ( argdefs, name_len, args_len, desc_len );
    
    std::cout << title << std::endl;
    process_arguments_usage( progname, argdefs );
    while ( (def = argdefs++)->key ) {
      if ( isprint( def->key ) ) std::cout << "  -" << (char)def->key;
      else                       std::cout << "    ";
      
      if ( def->name ) printf( "  --%-*s", name_len, def->name );
      else             printf( "    %-*s", name_len, "" );
      
      printf( "  %-*s  %s\n"
          , args_len, def->args ? def->args : ""
          , def->desc ? def->desc : ""
      );
    }
    if ( footer ) std::cout << footer;
    std::cout << std::endl;
  }
  
  void process_arguments_show_state ( void* settings_p ) {
    process_arguments_state_s& settings = *((process_arguments_state_s*)settings_p);
    std::cout << "state:" << std::endl;
    printf( "  %-12s => %s [%s]\n", "in ", settings.in_name, settings.in_file ? "FILE" : "STREAM" );
    printf( "  %-12s => %s [%s]\n", "out", settings.out_name, settings.out_file ? "FILE" : "STREAM" );
    printf( "  %-12s => %s [%s]\n", "err", settings.err_name, settings.err_file ? "FILE" : "STREAM" );
    printf( "  %-12s => %s [%s]\n", "nil", settings.nil_name, settings.nil_file ? "FILE" : "STREAM" );
    printf( "  %-12s => %d\n", "verbosity", settings.verbosity );
    
  }
  
};  // namespace ls

#endif  // PROCESS_ARGUMENTS_HPP

