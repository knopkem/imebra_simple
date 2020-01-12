# Options:
# -DIMEBRA_SHARED_STATIC=SHARED|STATIC (default = SHARED)
# -DJPEG2000=0|1|2 include JPEG2000 codec (1=version 1.X 2=version2.X)
# -DADDITIONAL_INCLUDE_PATH=path to additional include folders (optional)
# -DADDITIONAL_LIB_PATH=path to additional libraries (optional)
# -DIMEBRA_CHARSET_CONVERSION=ICONV|ICU|JAVA|WINDOWS (default = ICONV on posix, WINDOWS on Windows)
# -DIMEBRA_OBJC=1|0 (default = 0)
# -DIOS=PHONE|SIMULATOR (default not defined)
# -DEMBED_BITCODE=1|0 (default = 0)
# -DCMAKE_OSX_SYSROOT

cmake_minimum_required(VERSION 3.0)

project("imebra")

set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(imebra_libraries "")

# Select the proper charset conversion libraries according to the charset conversion method
#------------------------------------------------------------------------------------------
if(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    message("Using ICONV")
    add_definitions(-DIMEBRA_USE_ICONV)
    if(APPLE)
        set(imebra_libraries ${imebra_libraries} iconv)
    endif(APPLE)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES ICU)
    message("Using ICU")
    add_definitions(-DIMEBRA_USE_ICU)
    set(imebra_libraries ${imebra_libraries} icuuc)
    set(imebra_libraries ${imebra_libraries} icuio)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES JAVA)
    message("Using JAVA")
    add_definitions(-DIMEBRA_USE_JAVA)
elseif(IMEBRA_CHARSET_CONVERSION MATCHES WINDOWS)
    message("Using WINDOWS CHARSET CONVERSION")
    add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
else(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)
    if(WIN32)
        message("Using WINDOWS CHARSET CONVERSION (default)")
        add_definitions(-DIMEBRA_USE_WINDOWS_CHARSET)
    else(WIN32)
        message("Using ICONV (default)")
        add_definitions(-DIMEBRA_USE_ICONV)
        if(APPLE)
            set(imebra_libraries ${imebra_libraries} iconv)
        endif(APPLE)
    endif(WIN32)
endif(IMEBRA_CHARSET_CONVERSION MATCHES ICONV)

# Set the proper logging preprocessor macros
#-------------------------------------------
if(IMEBRA_LOG_FRAMEWORK MATCHES LOG4CXX)
    add_definitions(-DIMEBRA_LOGGING_LOG4CXX)
    set(imebra_libraries ${imebra_libraries} log4cxx) # Log4cxx needs additional libraries
elseif(IMEBRA_LOG_FRAMEWORK MATCHES SYSLOG)
    add_definitions(-DIMEBRA_LOGGING_SYSLOG)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES COUT)
    add_definitions(-DIMEBRA_LOGGING_COUT)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES APPLE)
    add_definitions(-DIMEBRA_LOGGING_APPLE)
elseif(IMEBRA_LOG_FRAMEWORK MATCHES ANDROID)
    add_definitions(-DIMEBRA_LOGGING_ANDROID)
endif(IMEBRA_LOG_FRAMEWORK MATCHES LOG4CXX)

if(IMEBRA_LOG_LEVEL)
    add_definitions(-DIMEBRA_LOG_LEVEL_${IMEBRA_LOG_LEVEL})
endif(IMEBRA_LOG_LEVEL)


# For IPhone build a static library, otherwise build a dynamic library
#---------------------------------------------------------------------
if(NOT DEFINED IMEBRA_SHARED_STATIC)
    set(IMEBRA_SHARED_STATIC "SHARED")
endif(NOT DEFINED IMEBRA_SHARED_STATIC)

if(APPLE)
    set(IMEBRA_OBJC 1)
    if(IOS)
        set(IMEBRA_SHARED_STATIC "STATIC")
        if("${IOS}" MATCHES "PHONE")
            exec_program(xcrun ARGS --sdk iphoneos --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
            set (CMAKE_OSX_ARCHITECTURES armv7 armv7s arm64)
        elseif("${IOS}" MATCHES "SIMULATOR")
            exec_program(xcrun ARGS --sdk iphonesimulator --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
            set (CMAKE_OSX_ARCHITECTURES i386 x86_64)
        else("${IOS}" MATCHES "PHONE")
            message (FATAL_ERROR "Set IOS to PHONE or SIMULATOR")
        endif("${IOS}" MATCHES "PHONE")
        message ("IOS SDK ROOT: ${CMAKE_OSX_SYSROOT}")
    endif(IOS)
endif(APPLE)


##############################################
#
# LIBRARY
#
##############################################
add_definitions(-DIMEBRA_DLL)
add_definitions(-DIMEBRA_DLL_EXPORTS)
add_definitions(-DNOMINMAX)

file(GLOB imebra_interface "${CMAKE_CURRENT_SOURCE_DIR}/library/include/imebra/*.h")
file(GLOB imebra_include "${CMAKE_CURRENT_SOURCE_DIR}/library/src/*.h")
file(GLOB imebra_src "${CMAKE_CURRENT_SOURCE_DIR}/library/src/*.cpp")
file(GLOB imebra_implementation_src "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.cpp")
file(GLOB imebra_implementation_src_objc "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.mm")
file(GLOB imebra_implementation_include "${CMAKE_CURRENT_SOURCE_DIR}/library/implementation/*.h")
set(imebra_objc_src "")
set(imebra_objc_include "")
set(imebra_objc_include_dir "")
set(imebra_objc_gnustep_dir "")

if("${IMEBRA_OBJC}" STREQUAL "1")

    file(GLOB imebra_objc_src "${CMAKE_CURRENT_SOURCE_DIR}/wrappers/objectivec/src/*.*")
    file(GLOB imebra_objc_include "${CMAKE_CURRENT_SOURCE_DIR}/wrappers/objectivec/include/imebraobjc/*.h")
    add_definitions(-DIMEBRA_OBJC)

    set(imebra_objc_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/wrappers/objectivec/include)

    set(objc_compiler_flags "")
    if(APPLE)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework Foundation -framework CoreServices")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Foundation -framework CoreServices")
        set(objc_compiler_flags "${objc_compiler_flags} -fobjc-arc")
        if(IOS)
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -framework UIKit")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework UIKit")
        else(IOS)
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -framework AppKit")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AppKit")
        endif(IOS)
    else(APPLE)
        set(objc_compiler_flags "${objc_compiler_flags} -fconstant-string-class=NSConstantString")
        set(imebra_objc_gnustep_dir "/usr/include/GNUstep")
        set(imebra_libraries ${imebra_libraries} gnustep-base objc)
    endif(APPLE)

    if(NOT "${objc_compiler_flags}" STREQUAL "")

        foreach(objcfile ${imebra_objc_src})
            set_source_files_properties(${objcfile} PROPERTIES COMPILE_FLAGS ${objc_compiler_flags})
        endforeach(objcfile ${imebra_objc_src})

    endif(NOT "${objc_compiler_flags}" STREQUAL "")

endif("${IMEBRA_OBJC}" STREQUAL "1")


if(NOT "${ADDITIONAL_LIB_PATH}" STREQUAL "")
    link_directories(${LINK_DIRECTORIES} ${ADDITIONAL_LIB_PATH})
endif(NOT "${ADDITIONAL_LIB_PATH}" STREQUAL "")

# Add jpeg2000 if specified
if("${JPEG2000}" STREQUAL "1")
    set(imebra_libraries ${imebra_libraries} openjpeg)
    add_definitions(-DJPEG2000)
    add_definitions(-DJPEG2000_V1)
    message(STATUS "Adding experimental Jpeg2000 codec (based on OpenJpeg v1.X)")
endif("${JPEG2000}" STREQUAL "1")

if("${JPEG2000}" STREQUAL "2")
    set(imebra_libraries ${imebra_libraries} openjp2)
    add_definitions(-DJPEG2000)
    add_definitions(-DJPEG2000_V2)
    message(STATUS "Adding experimental Jpeg2000 codec (based on OpenJpeg v2.X)")
endif("${JPEG2000}" STREQUAL "2")

add_library(imebra ${IMEBRA_SHARED_STATIC}
    ${imebra_implementation_src}
    ${imebra_implementation_src_objc}
    ${imebra_implementation_include}
    ${imebra_objc_src}
    ${imebra_objc_include}
    ${imebra_include}
    ${imebra_src}
    ${imebra_interface}
)

if(NOT "${ADDITIONAL_INCLUDE_PATH}" STREQUAL "")
    target_include_directories(imebra PRIVATE ${ADDITIONAL_INCLUDE_PATH})
endif(NOT "${ADDITIONAL_INCLUDE_PATH}" STREQUAL "")

if("${IMEBRA_OBJC}" STREQUAL "1")
    set(HIDDEN_FLAGS "")
else("${IMEBRA_OBJC}" STREQUAL "1")
    set(HIDDEN_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
endif("${IMEBRA_OBJC}" STREQUAL "1")


# Add flags specific to the compiler
#-----------------------------------
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")

    message(STATUS "MSVC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /Wp64")
    set(imebra_libraries ${imebra_libraries} Kernel32)

else()

    message(STATUS "GNU or Clang detected, adding compile flags")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wconversion -Wfloat-equal ${HIDDEN_FLAGS}")
    set(imebra_libraries ${imebra_libraries} pthread)
    if("${EMBED_BITCODE}" STREQUAL "1")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")
    endif("${EMBED_BITCODE}" STREQUAL "1")

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(CMAKE_BUILD_TYPE MATCHES DEBUG)
            message("Debug mode, enabling coverage")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
            set(imebra_libraries ${imebra_libraries} gcov)
        endif()
    endif()

endif()

target_include_directories(imebra PRIVATE ${imebra_objc_gnustep_dir})

# Define dependency libraries
#----------------------------
target_link_libraries(imebra ${imebra_libraries})

message (STATUS "OBJC INCLUDE:${imebra_objc_include_dir}")
message (STATUS "GNU INCLUDE:${imebra_objc_gnustep_dir}")

# Define Imebra public folder
#----------------------------
target_include_directories(imebra PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/library/include> $<INSTALL_INTERFACE:include>)
target_include_directories(imebra PUBLIC $<BUILD_INTERFACE:${imebra_objc_include_dir}> $<INSTALL_INTERFACE:include>)

# Set the library version from IMEBRA_MAJOR_VERSION, IMEBRA_MINOR_VERSION,
#  & IMEBRA_REVISION
#---------------------------------------------------------------------------------
if ("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "5")
else("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")
    set(IMEBRA_MAJOR_VERSION "$ENV{IMEBRA_MAJOR_VERSION}")
endif("$ENV{IMEBRA_MAJOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "0")
else("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")
    set(IMEBRA_MINOR_VERSION "$ENV{IMEBRA_MINOR_VERSION}")
endif("$ENV{IMEBRA_MINOR_VERSION}" STREQUAL "")

if ("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "0")
else("$ENV{IMEBRA_REVISION}" STREQUAL "")
    set(IMEBRA_REVISION "$ENV{IMEBRA_REVISION}")
endif("$ENV{IMEBRA_REVISION}" STREQUAL "")

set(IMEBRA_VERSION "${IMEBRA_MAJOR_VERSION}.${IMEBRA_MINOR_VERSION}.${IMEBRA_REVISION}")

set_target_properties(imebra PROPERTIES VERSION ${IMEBRA_VERSION}
                                          SOVERSION ${IMEBRA_MAJOR_VERSION})


    ##############################################
    #
    # MOVESCU example
    #
    ##############################################
    file(GLOB movescu_sources "${CMAKE_CURRENT_SOURCE_DIR}/examples/c++/movescu/*.cpp")

    # Add the source files to the project
    #------------------------------------
    add_executable(movescu ${movescu_sources})

    target_link_libraries(movescu imebra)

    ##############################################
    #
    # STORESCU example
    #
    ##############################################
    file(GLOB storescu_sources "${CMAKE_CURRENT_SOURCE_DIR}/examples/c++/storescu/*.cpp")

    # Add the source files to the project
    #------------------------------------
    add_executable(storescu ${storescu_sources})

    target_link_libraries(storescu imebra)

    ##############################################
    #
    # STORESCP example
    #
    ##############################################
    file(GLOB storescp_sources "${CMAKE_CURRENT_SOURCE_DIR}/examples/c++/storescp/*.cpp")

    # Add the source files to the project
    #------------------------------------
    add_executable(storescp ${storescp_sources})

    target_link_libraries(storescp imebra)


