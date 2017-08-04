cmake_minimum_required(VERSION 2.8)
add_compile_options(-g -std=c++0x -Wno-deprecated -Wall -fPIC)
message(STATUS "optional:-g -std=c++0x -Wno-deprecated -Wall -fPIC") 
   SET(MYTSS_SRCS  
      src/tss_sdk_api.cpp
      src/tss_sdk_import.c
	  src/cmq_handler.cpp
	  src/cmq_api.cpp
	  src/curl_http.cpp
      )   
INCLUDE_DIRECTORIES(include) 
INCLUDE_DIRECTORIES(/home/ubuntu/install/curl/include)
INCLUDE_DIRECTORIES(/home/hezhihua/cmq/qc_cmq_cpp_sdk_V1.0.4/inc)
LINK_DIRECTORIES(/home/ubuntu/install/curl/lib/)
SET(LIBRARY libcurl.a )
ADD_LIBRARY(mytssapi SHARED ${MYTSS_SRCS} )
ADD_LIBRARY(mytssapi_static STATIC ${MYTSS_SRCS})
SET (LIBRARY_OUTPUT_PATH  /data/deploy/wolf-backend/src/share/tencent/tsssdk)
