cmake_minimum_required(VERSION 2.8)
add_compile_options(-g -std=c++0x -Wno-deprecated -Wall -fPIC)
message(STATUS "optional:-g -std=c++0x -Wno-deprecated -Wall -fPIC") 
   SET(FLYFISH_SRCS  
      src/util/cm_config.cpp
      src/util/cm_string.cpp
	  src/util/tc_ex.cpp
	  src/util/tc_thread.cpp
	  src/util/tc_thread_cond.cpp
	  src/util/tc_thread_mutex.cpp
	  src/util/tc_common.cpp
	  src/util/tc_socket.cpp
	  src/util/tc_epoller.cpp
	  src/util/tc_clientsocket.cpp
	  src/util/tc_epoll_server.cpp
	  src/util/JceCurrent.cpp
	  src/util/Servant.cpp
	  src/util/ServantHelper.cpp
	  src/util/ServantHandle.cpp
      )   
INCLUDE_DIRECTORIES(include) 

#LINK_DIRECTORIES(/home/ubuntu/install/curl/lib/)
#SET(LIBRARY libcurl.a )
ADD_LIBRARY(flyfish SHARED ${FLYFISH_SRCS} )
ADD_LIBRARY(flyfish_static STATIC ${FLYFISH_SRCS})
# 指定静态库的输出名称
set_target_properties(flyfish_static PROPERTIES OUTPUT_NAME "flyfish")
# 使动态库和静态库同时存在
set_target_properties(flyfish PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties(flyfish_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 指定动态库版本
# VERSION 动态库版本
# SOVERSION API版本
#在win7机器的共享目录中编译，无法创建软链接 ,搬到 /root/flyfish 生成成功
#set_target_properties(flyfish PROPERTIES VERSION 1.0 SOVERSION 1)

SET (LIBRARY_OUTPUT_PATH  /mnt/hgfs/vmshare/flyfish/lib)
