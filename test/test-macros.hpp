
#ifndef TEST_MACROS_HPP
#define TEST_MACROS_HPP

#define TEST_EQ(a,b) \
try { all_count++; \
  if ( a == b ) { pass_count++; \
    if ( verbosity >= v_info ) std::cerr << "[PASS] " << #a << " == " << #b << ": actual " << (b) << std::endl; \
  } else { fail_count++; \
    std::cerr << "[FAIL] " << #a << " == " << #b << ": expected " << (a) << ", actual " << (b) << std::endl; \
  } \
} catch ( const std::runtime_error& e ) { error_count++; \
    std::cerr << "[ERROR] " << #a << " == " << #b << ": exception " << e.what() << std::endl; \
} catch ( const std::exception& e ) { error_count++; \
    std::cerr << "[ERROR] " << #a << " == " << #b << ": exception " << e.what() << std::endl; \
}

#define TEST_EQ_ASSTR(a,b) \
try { all_count++; \
  if ( std::string(a) == std::string(b) ) { \
    pass_count++; \
    if ( verbosity >= v_info ) std::cerr << "[PASS] " << #a << " == " << #b << ": actual " << (b) << std::endl; \
  } else { fail_count++; \
    if ( verbosity >= v_failure ) std::cerr << "[FAIL] " << #a << " == " << #b << ": expected " << (a) << ", actual " << (b) << std::endl; \
  } \
} catch ( const std::runtime_error& e ) { error_count++; \
    if ( verbosity >= v_failure ) std::cerr << "[ERROR] " << #a << " == " << #b << ": exception " << e.what() << std::endl; \
} catch ( const std::exception& e ) { error_count++; \
    if ( verbosity >= v_failure ) std::cerr << "[ERROR] " << #a << " == " << #b << ": exception " << e.what() << std::endl; \
}

#define TEST_FILE_VERSION(a,b) \
  TEST_EQ(a.parts.major[0],b.parts.major[0]) \
  TEST_EQ(a.parts.major[1],b.parts.major[1]) \
  TEST_EQ(a.parts.sub[0],b.parts.sub[0]) \
  TEST_EQ(a.parts.sub[1],b.parts.sub[1]) \
  TEST_EQ(a.parts.sub[2],b.parts.sub[2]) \
  TEST_EQ(a.parts.minor[0],b.parts.minor[0]) \
  TEST_EQ(a.parts.minor[1],b.parts.minor[1]) \
  TEST_EQ(a.parts.minor[2],b.parts.minor[2]) \
  TEST_EQ(a.parts.release,b.parts.release) \
  TEST_EQ_ASSTR(a.c_str,b.c_str) \
  TEST_EQ(a.major( ),b.major( )) \
  TEST_EQ(a.sub( ),b.sub( )) \
  TEST_EQ(a.minor( ),b.minor( )) \
  TEST_EQ(a.release( ),b.release( )) \
  TEST_EQ(a.version( ),b.version( ))

#define TEST_HEADING(a) \
if ( verbosity >= v_info ) std::cerr << "[TEST] " << #a << std::endl;

#define TEST_TRAP_BEGIN(a) \
try {

#define TEST_TRAP_END(a) \
} catch ( const std::runtime_error& e ) {  all_count++; error_count++; \
    if ( verbosity >= v_failure ) std::cerr << "[ERROR] " << #a << ": exception " << e.what() << std::endl; \
} catch ( const std::exception& e ) { all_count++; error_count++; \
    if ( verbosity >= v_failure ) std::cerr << "[ERROR] " << #a << ": exception " << e.what() << std::endl; \
}

#define TEST_TRAP(a) \
TEST_TRAP_BEGIN(a) \
a; \
TEST_TRAP_END(a)

#endif  // TEST_MACROS_HPP

