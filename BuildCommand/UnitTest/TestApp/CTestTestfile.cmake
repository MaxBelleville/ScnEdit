# CMake generated Testfile for 
# Source directory: C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp
# Build directory: C:/Users/maxbe/Documents/Swat3/skylicht-engine/BuildCommand/UnitTest/TestApp
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(TestApp "C:/Users/maxbe/Documents/Swat3/skylicht-engine/Bin/Debug/TestApp.exe")
  set_tests_properties(TestApp PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;71;add_test;C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(TestApp "C:/Users/maxbe/Documents/Swat3/skylicht-engine/Bin/Release/TestApp.exe")
  set_tests_properties(TestApp PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;71;add_test;C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(TestApp "C:/Users/maxbe/Documents/Swat3/skylicht-engine/Bin/MinSizeRel/TestApp.exe")
  set_tests_properties(TestApp PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;71;add_test;C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(TestApp "C:/Users/maxbe/Documents/Swat3/skylicht-engine/Bin/RelWithDebInfo/TestApp.exe")
  set_tests_properties(TestApp PROPERTIES  _BACKTRACE_TRIPLES "C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;71;add_test;C:/Users/maxbe/Documents/Swat3/skylicht-engine/UnitTest/TestApp/CMakeLists.txt;0;")
else()
  add_test(TestApp NOT_AVAILABLE)
endif()
