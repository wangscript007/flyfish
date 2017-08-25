#ifndef _HELLO_SERVER_H
#define _HELLO_SERVER_H

#include "util/tc_epoll_server.h"
#include "util/cm_config.h"

using namespace std;
using namespace taf;

static char g_config_file[512]={0};
static int  g_daemon = 1;
static int  sb_pid=0;

#define VNAME(name) (#name)
#define OUTPUTVALUE(value) {cout<<VNAME(value)<<":"<<value<<endl;}

shared_ptr<TC_EpollServer>  _epollServer;

//shared_ptr<TC_EpollServer::RollWrapperInterface> _tclog;
TC_EpollServer::RollWrapperInterface *_tclog;

CM_ConfigFileParser _configFileParser;
#endif