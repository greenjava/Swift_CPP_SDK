cmake_minimum_required(VERSION 2.9)
project(SwiftCpp)

set(POCO_DIR CACHE PATH "Poco libraries location")

find_library(Poco_Foundation_LIB NAMES PocoFoundationd PocoFoundation PATHS ${POCO_DIR}/lib)
find_library(Poco_Net_LIB NAMES PocoNetd PocoNet PATHS ${POCO_DIR}/lib)
find_path(Poco_INCLUDE_DIR Poco/Poco.h PATHS ${POCO_DIR}/include)

set(SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Account.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Container.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Endpoint.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ErrorNo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Header.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPIO.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsoncpp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Object.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Service.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tenant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Token.cpp
)
set(HEADER_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Account.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Authentication.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigKey.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Endpoint.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ErrorNo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Header.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HTTPIO.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json-forwards.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Object.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Service.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SwiftResult.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Tenant.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Token.h
)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Source Files" FILES ${SOURCE_FILES})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(BUILD_SHARED_LIBS "Build shared library." ON)
add_library(SwiftCpp ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(SwiftCpp ${Poco_Foundation_LIB} ${Poco_Net_LIB})
target_include_directories(SwiftCpp SYSTEM PRIVATE ${Poco_INCLUDE_DIR})

if(MSVC)
  # Disable the warning C4251
  add_definitions("/wd4251")
endif()

include(GenerateExportHeader)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
generate_export_header(SwiftCpp)

install(TARGETS SwiftCpp
    RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/swiftcpp)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/swiftcpp_export.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

