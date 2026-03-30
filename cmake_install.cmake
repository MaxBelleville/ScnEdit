# Install script for directory: C:/Users/maxbe/Documents/Swat 3/skylicht-engine

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/maxbe/Documents/Swat 3/skylicht-engine/InstallLibs")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/ThirdParty/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Irrlicht/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/System/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Engine/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Imgui/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/SpineCpp/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Graph/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Components/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Collision/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Lightmapper/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Bullet3/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Physics/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Audio/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/UserInterface/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Crypto/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Network/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Projects/Skylicht/Client/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/Samples/SCNEdit/cmake_install.cmake")
  include("C:/Users/maxbe/Documents/Swat 3/skylicht-engine/UnitTest/TestApp/cmake_install.cmake")

endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/maxbe/Documents/Swat 3/skylicht-engine/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
if(CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_COMPONENT MATCHES "^[a-zA-Z0-9_.+-]+$")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
  else()
    string(MD5 CMAKE_INST_COMP_HASH "${CMAKE_INSTALL_COMPONENT}")
    set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INST_COMP_HASH}.txt")
    unset(CMAKE_INST_COMP_HASH)
  endif()
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/maxbe/Documents/Swat 3/skylicht-engine/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
