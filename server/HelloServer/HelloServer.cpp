#include <string.h>
#include <string>
#include <iostream>
#include "util/cm_config.h"
#include "util/tc_epoll_server.h"
#include "util/tc_common.h"
#include "util/ServantHandle.h"
#include "util/ServantHelper.h"
#include "HelloServer.h"
#include "TestImp.h"


void show_help()
{
	printf("Usage : stabird [-h][-v][-e][-c config_file]\n");
	printf("        -h Show help\n");
	printf("        -v Show version\n");
	printf("        -e don't start as daemon process!\n");
	printf("        -c input config file. [must]\n");
}

void show_version()
{
	printf("version: %s\n", "1.0");
	return;
}
void close_all_fd()
{
	uint32_t fdlimit = sysconf(_SC_OPEN_MAX);

	for (uint32_t fd=0;fd<fdlimit;fd++)
	{
		close(fd);
	}
}

int init_daemon()
{
	if (g_daemon)
	{
		switch (fork())
		{
		case 0:  break;
		case -1: return -1;
		default: exit(0);          
		}

		if (setsid() < 0)
		{
			return -1;
		}

		//leader exit,so the second child can't open shell
		switch (fork())
		{
		case 0:  break;
		case -1: return -1;
		default: exit(0);
		}

		sb_pid=getpid();
		//chdir("/");
		umask(0);

		close_all_fd();
	}

	return 0;

}

void set_daemon_flag(int flag)
{
	g_daemon = flag;

	return;
}

void set_cfgfile(char *filename)
{
	if (filename)
	{
		snprintf(g_config_file, sizeof(g_config_file), "%s", filename);
	}
	return;
}

int get_options(int argc, char* argv[])
{
	int optch;
	extern char *optarg;

	static char optstring[] = "hvec:";
	while((optch = getopt(argc, argv, optstring)) != -1)
	{
		switch (optch)
		{
		case 'h':
			show_help();
			return -1;
		case 'v':
			show_version();
			return -1;
		case 'e':
			set_daemon_flag(0);
			break;
		case 'c':
			set_cfgfile(optarg);
			break;
		}
	}

	return 0;
}




int bindAdapters(vector<TC_EpollServer::BindAdapterPtr>& adapters)
{
	CM_ConfigFileParser configFileParser(g_config_file);
	_configFileParser=configFileParser;
	string  endpoint=_configFileParser.GetValue("HelloObjAdapter","endpoint");
	string  maxconns=_configFileParser.GetValue("HelloObjAdapter","maxconns");
	
	string  threads=_configFileParser.GetValue("HelloObjAdapter","threads");
	string  queuecap=_configFileParser.GetValue("HelloObjAdapter","queuecap");

	string  queuetimeout=_configFileParser.GetValue("HelloObjAdapter","queuetimeout");
	string  servant=_configFileParser.GetValue("HelloObjAdapter","servant");

	string  allow=_configFileParser.GetValue("HelloObjAdapter","allow");
	string  denny=_configFileParser.GetValue("HelloObjAdapter","denny");
	string  protocol=_configFileParser.GetValue("HelloObjAdapter","protocol");

	string  netThreadNum=_configFileParser.GetValue("server","net-threads");

	OUTPUTVALUE(endpoint)
	OUTPUTVALUE(maxconns)
	OUTPUTVALUE(threads)
	OUTPUTVALUE(queuecap)
	OUTPUTVALUE(queuetimeout)
	OUTPUTVALUE(servant)
	OUTPUTVALUE(protocol)
	OUTPUTVALUE(netThreadNum)

	_epollServer = std::make_shared<TC_EpollServer>(TC_Common::strto<int>(netThreadNum));// new TC_EpollServer(TC_Common::strto<int>(netThreadNum));

	TC_EpollServer::BindAdapterPtr bindAdapter = std::make_shared<TC_EpollServer::BindAdapter>(_epollServer.get());// new TC_EpollServer::BindAdapter(_epollServer.get());

	ServantHelperManager::getInstance()->setAdapterServant("HelloObjAdapter", servant);

	bindAdapter->setName("HelloObjAdapter");

	bindAdapter->setEndpoint(endpoint);

	bindAdapter->setMaxConns(TC_Common::strto<int>(maxconns ));

	bindAdapter->setOrder(TC_EpollServer::BindAdapter::ALLOW_DENY);

	bindAdapter->setAllow(TC_Common::sepstr<string>(allow, ";,", false));

	bindAdapter->setDeny(TC_Common::sepstr<string>(denny, ";,", false));

	bindAdapter->setQueueCapacity(TC_Common::strto<int>(queuecap));

	bindAdapter->setQueueTimeout(TC_Common::strto<int>(queuetimeout));

	bindAdapter->setProtocolName(protocol);

	if (bindAdapter->isTafProtocol())
	{
		//bindAdapter->setProtocol(AppProtocol::parse);
	}

	bindAdapter->setHandleGroupName("HelloObjAdapter");

	bindAdapter->setHandleNum(TC_Common::strto<int>(threads));

	bindAdapter->setBackPacketBuffLimit(0);

	//固定第一个线程监听客户端连接,accept得到的客户端fd 分给其它线程监听
	_epollServer->bind(bindAdapter);

	//处理类
	bindAdapter->setHandle<ServantHandle>();

	cout <<__FILE__LINE__<< "|bindAdapter->use_count=" <<bindAdapter.use_count()<<endl;
	adapters.push_back(bindAdapter);
	cout <<__FILE__LINE__<< "|bindAdapter->use_count=" <<bindAdapter.use_count()<<endl;

	return 0;
}

template<typename T> void addServant(const string &id)
{
	cout<<"id"<<id<<endl;
	ServantHelperManager::getInstance()->addServant<T>(id);
}

int init_server()
{
	vector<TC_EpollServer::BindAdapterPtr> adapters;

	bindAdapters(adapters);

	addServant<TestImp>(_configFileParser.GetValue("HelloObjAdapter","servant"));

	_tclog=TC_EpollServer::RollWrapperInterface::getInstance();
	_tclog->init("HelloServer.log");
	_epollServer->setLocalLogger(_tclog);

	//启动业务处理线程
	_epollServer->startHandle();

	_epollServer->createEpoll();

	//cout << "_tclog->use_count=" <<_tclog.use_count() << endl;
	//_tclog=std::make_shared<TC_EpollServer::RollWrapperInterface>("HelloServer.log");  //new TC_EpollServer::RollWrapperInterface("HelloServer.log");
	
	
	cout << "\n" << "[initialize server] "  << " [Done]" << endl;
	//cout << "_tclog->use_count=" <<_tclog.use_count() << endl;
	sleep(1);
	return 0;
}


void waitForQuit()
{
	unsigned int iNetThreadNum = _epollServer->getNetThreadNum();
	vector<TC_EpollServer::NetThread*> vNetThread = _epollServer->getNetThread();

	for (size_t i = 0; i < iNetThreadNum; ++i)
	{
		vNetThread[i]->start();
	}

	_epollServer->debug("server netthread num : " + TC_Common::tostr(iNetThreadNum));

	while(!_epollServer->isTerminate())
	{
		{
			TC_ThreadLock::Lock sync(*_epollServer);
			_epollServer->timedWait(5000);
		}
		long iLastCheckTime=0;
        long iNow = TNOW;

		if(iNow - iLastCheckTime > 10)
		{
			iLastCheckTime = iNow;

			size_t n = 0;
			for(size_t i = 0;i < iNetThreadNum; ++i)
			{
				n = n + vNetThread[i]->getSendRspSize();
			}

			cout<<"getSendRspSize ="<<n<<endl;
		}
	}
	_epollServer->debug("_epollServer->isTerminate : " + TC_Common::tostr(_epollServer->isTerminate()));
	if(_epollServer->isTerminate())
	{
		for(size_t i = 0; i < iNetThreadNum; ++i)
		{
			vNetThread[i]->terminate();
			vNetThread[i]->getThreadControl().join();
		}

		_epollServer->stopThread();
	}

}
int main(int argc, char* argv[])
{
	if(get_options(argc, argv)!=0)
	{
		return 1;
	}
	if (init_daemon()!=0)
	{
		return 1;
	}

	init_server();

	waitForQuit();
	return 0;
}