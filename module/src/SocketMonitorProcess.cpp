/*****************************************************************************/
/*                                                                           */
/*           系统名        ：        内蒙气象局后台服务                      */
/*           客户名        ：        内蒙气象局                              */
/*           机能名        ：        端口监控处理类                          */
/*           程序名        ：        SocketMonitorProcess.cpp                */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                VERSION    DATE      BY      CHANGE/COMMENT                */
/*---------------------------------------------------------------------------*/
/*                V1.00      09-11-11          Create                        */
/*                                                                           */
/*****************************************************************************/

#include "SocketMonitorProcess.h"
#include "ObjectFactory.h"
#include "PacketAnalysisProcess.h"
#include "ThreadPool.h"

#define POOL_MAX 25

/******************************************************************************
    处理名        ：  类的构造，类的成员变量的初始化处理
    函数名        ：  CSocketMonitorProcess()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
CSocketMonitorProcess::CSocketMonitorProcess(){
    m_pRecvSocket             = NULL;
    m_bBreakFlag              = false;
    m_nSocketPort             = -1;
    m_nFuncType               = -1;
}

/******************************************************************************
    处理名        ：  类的析构处理
    函数名        ：  ~CSocketMonitorProcess()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
CSocketMonitorProcess::~CSocketMonitorProcess(){
    m_pRecvSocket = NULL;

}

//下面是测试代码  
void *clientprocess(const void *m_pTCPSocket)
{
    try{
	IProcess* m_pProcess = NULL;
    HTCPSocket *tcpp = NULL;

    tcpp = (HTCPSocket*)m_pTCPSocket;
	m_pProcess = (IProcess*)(new CPacketAnalysisProcess(tcpp));
	printfs(2, "clientprocess pid:%u m_pProcess:0x%04x m_pTCPSocket:0x%04x m_nsocket_tcp:%u",
	    pthread_self(),m_pProcess,tcpp,tcpp->get_m_nsocket_tcp());

	m_pProcess->Do();

	delete m_pProcess;
	m_pProcess = NULL;
    }catch(exception e){
    	printfs(1, "myprocess exception");
    }
    return NULL;
}

/******************************************************************************
    处理名        ：  业务处理
    函数名        ：  Do()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
void CSocketMonitorProcess::Do(){
    if (m_nSocketPort == -1){
        printfs(0, "<CSocketMonitorProcess>Channel port error!");
        return;
    }

    printfs(1, "Socket monitor process started! Port.[%d]", m_nSocketPort);

    try{
        // 1.开始Socket监控
        if(m_monitorSocket.CreateReceiveSocket( m_nSocketPort ) == 0){
            printfs(0, "Socket monitor create failed Port.[%d]", m_nSocketPort);
            return;
        }

        int             nRet       = 0;
        char            chName[20] = "\0";
	    pool_init(POOL_MAX);//初始化线程池

        while(1){
            if(pool_free()==0){
                printfs(0, "[%s] 线程池无可用线程!","CSocketMonitorProcess::Do()");
                sleep(1);
                continue;
            }

            //有新的Socket连接到达
            m_pRecvSocket = new HTCPSocket();

            nRet = m_pRecvSocket->Accept(&m_monitorSocket);
            printfs(1, "<CSocketMonitorProcess::Do()> 有新的Socket连接到达!");

            if( nRet != 1 ) {
                printfs(0, "[%s] [Accept failed 新的Socket申请失败!]", "CSocketMonitorProcess::Do()");
                if(m_pRecvSocket!=NULL) {
                    delete m_pRecvSocket;
                    m_pRecvSocket = NULL;
                }
                sleep(3);
                continue;
            }

            printfs(2, "<CSocketMonitorProcess::Do()> 新的Socket申请成功!");
            //新连接的任务加入线程池并启动
            nRet = pool_add_worker(clientprocess, m_pRecvSocket);
            //线程池满
            if(nRet==0){
                if(m_pRecvSocket!=NULL) {
                    delete m_pRecvSocket;
                    m_pRecvSocket = NULL;
                }
                printfs(0, "[%s]服务器停止接受客户端3s!","CSocketMonitorProcess::Do()");
                sleep(3);
                continue;
            }
            printfs(2, "<CSocketMonitorProcess::Do()> 轮询下一个连接!");
        }
    } catch(...){
        printfs(0, "[CSocketMonitorProcess::Do()] [Exception occured]");
    }
	//等待所有任务完成
	sleep(1);
	//销毁线程池
	pool_destroy();  

	if (!m_monitorSocket.IsClosed()){
		m_monitorSocket.Close();
	}
}

/******************************************************************************
    处理名        ：  终止当前线程的端口监控操作及子线程操作
    函数名        ：  TerminateProcess()
    参数          ：  无
    返回值        ：  = 0 : 成功
                      < 0 : 失败
******************************************************************************/
int CSocketMonitorProcess::TerminateProcess(){

    return 0;
}

/******************************************************************************
    End
******************************************************************************/
