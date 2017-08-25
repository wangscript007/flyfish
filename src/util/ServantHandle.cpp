//#include "util/tc_thread_pool.h"
//#include "util/tc_timeprovider.h"
#include "util/ServantHandle.h"
//#include "servant/Application.h"
#include "util/ServantHelper.h"
//#include "servant/AppProtocol.h"
//#include "servant/BaseF.h"
//#include "nodeF/taf_nodeF.h"

namespace taf
{

/////////////////////////////////////////////////////////////////////////
//
ServantHandle::ServantHandle()
{
}

ServantHandle::~ServantHandle()
{
    map<string, ServantPtr>::iterator it = _servants.begin();

    while(it != _servants.end())
    {
        try
        {
            it->second->destroy();
        }
        catch(exception &ex)
        {
            LOGERROR("[TAF]ServantHandle::destroy error:" << ex.what() << endl) ;
        }
        catch(...)
        {
            LOGERROR("[TAF]ServantHandle::destroy unknown exception error" << endl);
        }
        ++it;
    }
}

/*
void ServantHandle::handleAsyncResponse()
{
    ReqMessagePtr resp;

    map<string, ServantPtr>::iterator it = _servants.begin();

    while (it != _servants.end())
    {
        while (it->second->getResponseQueue().pop_front(resp, 0))
        {
            try
            {
                if (resp->response.iRet == JCESERVERSUCCESS)
                {
                    it->second->doResponse(resp);
                }
                else if (resp->proxy == NULL)
                {
                    it->second->doResponseNoRequest(resp);
                }
                else
                {
                    it->second->doResponseException(resp);
                }
            }
            catch (exception& e)
            {
                LOG->error() << "[TAF][ServantHandle::doResponse ex:" << e.what() << "]" << endl;
            }
            catch (...)
            {
                LOG->error() << "[TAF][ServantHandle::doResponse ex.]" << endl;
            }
        }

        //业务处理附加的自有消息
        try
        {
            it->second->doCustomMessage(false);
            it->second->doCustomMessage();
        }
        catch (exception& e)
        {
            LOG->error() << "[TAF][ServantHandle::doCustemMessage ex:" << e.what() << "]" << endl;
        }
        catch (...)
        {
            LOG->error() << "[TAF][ServantHandle::doCustemMessage ex.]" << endl;
        }

        ++it;
    }
}
*/

void ServantHandle::handleCustomMessage(bool bExpectIdle)
{
    for (map<string, ServantPtr>::iterator it = _servants.begin(); it != _servants.end(); it++)
    {
        //业务处理附加的自有消息
        try
        {
            it->second->doCustomMessage(bExpectIdle);
            it->second->doCustomMessage();
        }
        catch (exception& e)
        {
            LOGERROR( "[TAF][ServantHandle::doCustemMessage ex:" << e.what() << "]" << endl);
        }
        catch (...)
        {
            LOGERROR( "[TAF][ServantHandle::doCustemMessage ex.]" << endl);
        }
    }
}

/*
bool ServantHandle::allFilterIsEmpty()
{
    map<string, ServantPtr>::iterator it = _servants.begin();

    while (it != _servants.end())
    {
        if (it->second->getResponseQueue().size() > 0)
        {
            return false;
        }
        ++it;
    }
    return true;
}*/

void ServantHandle::initialize()
{
    map<string, TC_EpollServer::BindAdapterPtr>::iterator adpit;

    map<string, TC_EpollServer::BindAdapterPtr>& adapters = _handleGroup->adapters;

	cout<<"adapters.size()="<<adapters.size()<<endl;
    for (adpit = adapters.begin(); adpit != adapters.end(); ++adpit)
    {
        ServantPtr servant = ServantHelperManager::getInstance()->create(adpit->first);

        if (servant)
        {
            _servants[servant->getName()] = servant;
        }
        else
        {
            LOGERROR("[TAF]ServantHandle initialize createServant ret null, for adapter `" + adpit->first + "`" << endl);
        }
    }

    map<string, ServantPtr>::iterator it = _servants.begin();

    if(it == _servants.end())
    {
        LOGERROR( "[TAF]initialize error: no servant exists." << endl);

        //TafRemoteNotify::getInstance()->report("initialize error: no servant exists.");

        exit(-1);
    }

    while(it != _servants.end())
    {
        try
        {
            it->second->setHandle(this);

            it->second->initialize();

            LOGINFO("[TAF][" << it->second->getName() << "] initialize" << endl);
        }
        catch(exception &ex)
        {
           LOGERROR( "[TAF]initialize error:" << ex.what() << endl);

           // TafRemoteNotify::getInstance()->report("initialize error:" + string(ex.what()));

            exit(-1);
        }
        catch(...)
        {
           LOGERROR( "[TAF]initialize unknown exception error" << endl);

           // TafRemoteNotify::getInstance()->report("initialize error");

            exit(-1);
        }
        ++it;
    }
}

/*
void ServantHandle::heartbeat()
{
    //10s上报给node一次
    time_t fcur = TC_TimeProvider::getInstance()->getNow();

    map<string, TC_EpollServer::BindAdapterPtr>::iterator it;

    map<string, TC_EpollServer::BindAdapterPtr>& adapters = _handleGroup->adapters;

    for (it = adapters.begin(); it != adapters.end(); ++it)
    {
        if (abs(fcur - it->second->getHeartBeatTime()) > HEART_BEAT_INTERVAL)
        {
            it->second->setHeartBeatTime(fcur);

            TAF_KEEPALIVE(it->second->getName());


            string connectRateName = it->second->getName() + ".connectRate";
            PropertyReportPtr connectPtr = Application::getCommunicator()->getStatReport()->getPropertyReport(connectRateName);
            //上报连接数 比率
            if (connectPtr)
            {
                int iRate=it->second->getNowConnection()*1000/it->second->getMaxConns();
                connectPtr->report(iRate);
            }

            //有队列, 且队列长度>0才上报
            size_t n = it->second->getRecvBufferSize();
            if (n <= 0)
            {
                continue;
            }
            string queueName = it->second->getName() + ".queue";
            PropertyReportPtr queuePtr = Application::getCommunicator()->getStatReport()->getPropertyReport(queueName);
            if (queuePtr)
            {
                queuePtr->report(n);
            }
        }
    }
}
*/

JceCurrentPtr ServantHandle::createCurrent(const TC_EpollServer::tagRecvData &stRecvData)
{
    JceCurrentPtr current = std::make_shared<JceCurrent>(this);//new JceCurrent(this);

    try
    {
        current->initialize(stRecvData);
    }
    catch (std::runtime_error &ex)
    {
        LOGERROR( "[TAF]ServantHandle::handle request protocol decode error:" << ex.what() << endl);
        close(stRecvData.uid,stRecvData.fd);
        return NULL;
    }

    //只有TAF协议才处理
    if(current->getBindAdapter()->isTafProtocol())
    {
        time_t now = TNOW;//TC_TimeProvider::getInstance()->getNow();

        //数据在队列中的时间超过了客户端等待的时间(TAF协议)
        if (current->_request.iTimeout > 0 && (now - stRecvData.recvTimeStamp -1 )*1000 > current->_request.iTimeout)
        {
            LOGERROR(  "[TAF]ServantHandle::handle queue timeout:"
                         << current->_request.sServantName << "|"
                         << current->_request.sFuncName << "|"
                         << stRecvData.recvTimeStamp << "|"
                         << stRecvData.adapter->getQueueTimeout() << "|"
                         << current->_request.iTimeout << "|"
                         << now << "|" << stRecvData.ip << ":" << stRecvData.port << endl);

            //current->sendResponse(JCESERVERQUEUETIMEOUT);

            return NULL;
        }
    }

    return current;
}

JceCurrentPtr ServantHandle::createCloseCurrent(const TC_EpollServer::tagRecvData &stRecvData)
{
	JceCurrentPtr current = std::make_shared<JceCurrent>(this);//new JceCurrent(this);
    current->initializeClose(stRecvData);
    current->setReportStat(false);

	return current;
}

void ServantHandle::handleClose(const TC_EpollServer::tagRecvData &stRecvData)
{
    LOGINFO("[TAF]ServantHandle::handleClose,adapter:" << stRecvData.adapter->getName()
                << ",peer:" << stRecvData.ip << ":" << stRecvData.port << endl);

	JceCurrentPtr current = createCloseCurrent(stRecvData);

	map<string, ServantPtr>::iterator sit = _servants.find(current->getServantName());

    if (sit == _servants.end())
    {
        LOGERROR( "[TAF]ServantHandle::handleClose,adapter:" << stRecvData.adapter->getName()
                << ",peer:" << stRecvData.ip << ":" << stRecvData.port <<", " << current->getServantName() << " not found" << endl);

        return;
    }


    try
    {
        //业务逻辑处理
        sit->second->doClose(current);
    }
    catch(exception &ex)
    {
        LOGERROR( "[TAF]ServantHandle::handleClose " << ex.what() << endl);

        return;
    }
    catch(...)
    {
        LOGERROR( "[TAF]ServantHandle::handleClose unknown error" << endl);

        return;
    }
}

void ServantHandle::handleTimeout(const TC_EpollServer::tagRecvData &stRecvData)
{
    JceCurrentPtr current = createCurrent(stRecvData);

    if (!current) return;

    LOGERROR(  "[TAF]ServantHandle::handleTimeout adapter '"
                 << stRecvData.adapter->getName()
                 << "', recvtime:" << stRecvData.recvTimeStamp << "|"
                 << ", timeout:" << stRecvData.adapter->getQueueTimeout()
                 << ", id:" << current->getRequestId() << endl);

    if (current->getBindAdapter()->isTafProtocol())
    {
        //current->sendResponse(JCESERVERQUEUETIMEOUT);
    }
}

void ServantHandle::handleOverload(const TC_EpollServer::tagRecvData &stRecvData)
{
    JceCurrentPtr current = createCurrent(stRecvData);

    if (!current) return;

    LOGERROR( "[TAF]ServantHandle::handleOverload adapter '"
                 << stRecvData.adapter->getName()
                 << "',overload:-1" //<< stRecvData.adapter->getRecvBufferSize() <<  ">"
                 << stRecvData.adapter->getQueueCapacity()
                 << ",id:" << current->getRequestId() << endl);

    if (current->getBindAdapter()->isTafProtocol())
    {
        //current->sendResponse(JCESERVEROVERLOAD);
    }
}

void ServantHandle::handle(const TC_EpollServer::tagRecvData &stRecvData)
{
    JceCurrentPtr current = createCurrent(stRecvData);

    if (!current) return;

    if (current->getBindAdapter()->isTafProtocol())
    {
        handleTafProtocol(current);
    }
    else
    {
        handleNoTafProtocol(current);
    }
}

/*
bool ServantHandle::processDye(const JceCurrentPtr &current, string& dyeingKey)
{
    //当前线程的线程数据
    ServantProxyThreadData* sptd = ServantProxyThreadData::getData();

    if (sptd)
    {
        sptd->data()->_dyeingKey = "";
    }

    //当前请求已经被染色, 需要打印染色日志
    map<string, string>::const_iterator dyeingIt = current->getRequestStatus().find(ServantProxy::STATUS_DYED_KEY);

    if (IS_MSG_TYPE(current->getMessageType(), taf::JCEMESSAGETYPEDYED))
    {
        LOGINFO("[TAF] servant got a dyeing request, message_type set" << current->getMessageType() << endl);

        if (dyeingIt != current->getRequestStatus().end())
        {
            LOGINFO("[TAF] servant got a dyeing request, dyeing key:" << dyeingIt->second << endl);

            dyeingKey = dyeingIt->second;
        }
        return true;
    }

    //servant已经被染色, 开启染色日志
    if (ServantHelperManager::getInstance()->isDyeing())
    {
        map<string, string>::const_iterator gridKeyIt = current->getRequestStatus().find(ServantProxy::STATUS_GRID_KEY);

        if (gridKeyIt != current->getRequestStatus().end() &&
            ServantHelperManager::getInstance()->isDyeingReq(gridKeyIt->second, current->getServantName(), current->getFuncName()))
        {
            LOGINFO("[TAF] dyeing servant got a dyeing req, key:" << gridKeyIt->second << endl);

            dyeingKey = gridKeyIt->second;

            return true;
        }
    }

    return false;
}

bool ServantHandle::processGrid(const JceCurrentPtr &current)
{
    int32_t clientGrid = -1;

    string clientKey = "";

    //当前服务是否是路由服务
    int32_t serverGrid = current->getBindAdapter()->getEndpoint().getGrid();

    //当前消息是否是路由消息
    bool isGridMessage = IS_MSG_TYPE(current->getMessageType(), taf::JCEMESSAGETYPEGRID);

    //如果是路由消息则取出clientGrid的值
    if (isGridMessage)
    {
        const map<string, string>& status = current->getRequestStatus();

        //灰度路由的用户号码
        map<string, string>::const_iterator keyIt = status.find(ServantProxy::STATUS_GRID_KEY);

        //灰度路由的服务端状态值
        map<string, string>::const_iterator gridIt = status.find(ServantProxy::STATUS_GRID_CODE);

        if (keyIt != status.end() && gridIt != status.end())
        {
            //客户端请求来的key以及路由状态
            clientKey = keyIt->second;

            clientGrid = TC_Common::strto<int32_t>(gridIt->second);
        }
    }

    LOGINFO("[TAF]ServantHandle::handleTafProtocol serverGrid:" << serverGrid
                << ",isGridMessage:" << isGridMessage
                << ",clientGrid:" << clientGrid
                << ",clientKey:" << clientKey << endl);

    //如果是有状态的服务,且不是ping
    if (serverGrid != 0 && current->getFuncName() != "taf_ping")
    {
        //非路由消息或者状态值不相等，则返回resetGrid命令(以服务端状态为准)
        if (!isGridMessage || clientGrid != serverGrid)
        {
            map<string, string> status;

            status[ServantProxy::STATUS_GRID_CODE] = TC_Common::tostr(serverGrid);

            current->sendResponse(JCESERVERRESETGRID, JceCurrent::TAF_BUFFER(), status);

            return true;
        }
    }

    //是路由消息且1.服务端无状态或者2.有状态且值匹配,则保存并传递状态值
    if (isGridMessage)
    {
        ServantProxyThreadData* sptd = ServantProxyThreadData::getData();

        if (sptd)
        {
            sptd->data()->_routeKey = clientKey;

            sptd->data()->_gridCode = clientGrid;
        }
    }
    return false;
}

void ServantHandle::handleTafProtocol(const JceCurrentPtr &current)
{
    LOGINFO("[TAF]ServantHandle::handleTafProtocol current:"
                << current->getIp() << "|"
                << current->getPort() << "|"
                << current->getMessageType() << "|"
                << current->getServantName() << "|"
                << current->getFuncName() << "|"
                << current->getRequestId() << "|"
                << TC_Common::tostr(current->getRequestStatus())<<endl);

    //处理灰度逻辑
    if (processGrid(current))
    {
        return;
    }

    //处理染色消息
    string dyeingKey = "";

    TafDyeingSwitch dyeSwitch;

    if (processDye(current, dyeingKey))
    {
        dyeSwitch.enableDyeing(dyeingKey);
    }

    //保存采样信息
    current->saveSampleKey();

    map<string, ServantPtr>::iterator sit = _servants.find(current->getServantName());

    if (sit == _servants.end())
    {
        current->sendResponse(JCESERVERNOSERVANTERR);

        return;
    }

    int ret = JCESERVERUNKNOWNERR;

    string sResultDesc = "";

    vector<char> buffer;

    try
    {
        //业务逻辑处理
        ret = sit->second->dispatch(current, buffer);
    }
    catch(JceDecodeException &ex)
    {
        LOG->error() << "[TAF]ServantHandle::handleTafProtocol " << ex.what() << endl;

        ret = JCESERVERDECODEERR;

        sResultDesc = ex.what();
    }
    catch(JceEncodeException &ex)
    {
        LOG->error() << "[TAF]ServantHandle::handleTafProtocol " << ex.what() << endl;

        ret = JCESERVERENCODEERR;

        sResultDesc = ex.what();
    }
    catch(exception &ex)
    {
        LOG->error() << "[TAF]ServantHandle::handleTafProtocol " << ex.what() << endl;

        ret = JCESERVERUNKNOWNERR;

        sResultDesc = ex.what();
    }
    catch(...)
    {
        LOG->error() << "[TAF]ServantHandle::handleTafProtocol unknown error" << endl;

        ret = JCESERVERUNKNOWNERR;

        sResultDesc = "handleTafProtocol unknown exception error";
    }

    //单向调用或者业务不需要同步返回
    if (current->isResponse())
    {
        current->sendResponse(ret, buffer, JceCurrent::TAF_STATUS(), sResultDesc);
    }
}
*/
void ServantHandle::handleTafProtocol(const JceCurrentPtr &current)
{}
void ServantHandle::heartbeat()
{}
void ServantHandle::handleAsyncResponse()
{}
bool ServantHandle::allFilterIsEmpty()
{
	return true;
}
void ServantHandle::handleNoTafProtocol(const JceCurrentPtr &current)
{
    LOGINFO("[TAF]ServantHandle::handleNoTafProtocol current:"
        << current->getIp() << "|"
        << current->getPort() << "|"
        << current->getServantName() << endl);

    map<string, ServantPtr>::iterator sit = _servants.find(current->getServantName());

    assert(sit != _servants.end());

    vector<char> buffer;

    try
    {
        //业务逻辑处理
        sit->second->dispatch(current, buffer);
    }
    catch(exception &ex)
    {
        LOGERROR( "[TAF]ServantHandle::handleNoTafProtocol " << ex.what() << endl);
    }
    catch(...)
    {
        LOGERROR( "[TAF]ServantHandle::handleNoTafProtocol unknown error" << endl);
    }

    if (current->isResponse())
    {
        current->sendResponse((const char*)(&buffer[0]), buffer.size());
    }
}

////////////////////////////////////////////////////////////////////////////
}
