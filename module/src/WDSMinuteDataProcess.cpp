/*****************************************************************************/
/*                                                                           */
/*           系统名        ：        内蒙气象局后台服务                      */
/*           客户名        ：        内蒙气象局                              */
/*           机能名        ：        六要素分钟数据业务处理类                */
/*           程序名        ：        WDSMinuteDataProcess.cpp                */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                VERSION    DATE      BY      CHANGE/COMMENT                */
/*---------------------------------------------------------------------------*/
/*                V1.00      09-11-11          Create                        */
/*                                                                           */
/*****************************************************************************/

#include "WDSMinuteDataProcess.h"
#include "WDS_DBHelper.h"
#include "ObjectFactory.h"

/******************************************************************************
    处理名        ：  类的构造，类的成员变量的初始化处理
    函数名        ：  CWDSMinuteDataProcess()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
CWDSMinuteDataProcess::CWDSMinuteDataProcess(){
    m_pSendSocket          = NULL;
    m_pRecvSocket          = NULL;
    m_nFuncType            = -1;
}

/******************************************************************************
    处理名        ：  类的析构处理
    函数名        ：  ~CWDSMinuteDataProcess()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
CWDSMinuteDataProcess::~CWDSMinuteDataProcess(){
    m_pRecvSocket = NULL;
    if(m_pSendSocket != NULL) {
        if (!m_pSendSocket->IsClosed()){
            m_pSendSocket->Close();
        }
        delete m_pSendSocket;
        m_pSendSocket = NULL;
    }
}

/******************************************************************************
    处理名        ：  业务处理
    函数名        ：  Do()
    参数          ：  无
    返回值        ：  无
******************************************************************************/
void CWDSMinuteDataProcess::Do(){
    printfs(1,"<CWDSMinuteDataProcess::Do()> WDSMinuteDataProcess start!");

    int nStatus = 0;
    int nRet = 0;

    // 取得分钟气象信息的长度
    int nLength = sizeof(stElementsMinuteData) - sizeof(stHeader);
    stElementsMinuteData elementsMinuteData;
    memset(&elementsMinuteData,  0, sizeof(elementsMinuteData));

    WDS_DBHelper* pWDSHelper = NULL;
    do{
        if (m_pRecvSocket == NULL){
            printfs(0,"<CWDSMinuteDataProcess::Do()> m_pRecvSocket == NULL");
            break;
        }

        // 【业务处理】1. 取得端口传入的分钟气象数据信息数据
        nRet = m_pRecvSocket->Receive((char*)(&elementsMinuteData) + sizeof(stHeader), nLength);
        if(nRet == 0) {
            printfs(0,"<CWDSMinuteDataProcess::Do()> Receive packet time out");
            m_pRecvSocket->Close();
            break;
        }
        if(nRet == -1) {
            printfs(0, "<CWDSMinuteDataProcess::Do()> Receive packet failed");
            m_pRecvSocket->Close();
            break;
        }
        printfs(2, "<CWDSMinuteDataProcess::Do()> Receive struct info:cCurTime[%s], cStationID[%.10s], fCurTemp[%f], fRainfall[%f], cWindDirection[%.4s], fWindVelocity[%f], fCurAP[%f], fHumidity[%f]",
                             elementsMinuteData.cCurTime,
                             elementsMinuteData.cStationID,
                             elementsMinuteData.fCurTemp,
                             elementsMinuteData.fRainfall,
                             elementsMinuteData.cWindDirection,
                             elementsMinuteData.fWindVelocity,
                             elementsMinuteData.fCurAP,
                             elementsMinuteData.fHumidity);

        // 修改方向为标准方向
        nStatus = CObjectFactory::GetInstance()->ChangeDirection(elementsMinuteData.cWindDirection);
        if (!nStatus){
            printfs(0,"<CWDSMinuteDataProcess::Do()> Wind direction error! input wind direct [%s]", elementsMinuteData.cWindDirection);
            break;
        }

        MYSQL* pMysqlConnection = CObjectFactory::GetInstance()->GetMySQLPool()->GetIdleMySql();
        if (pMysqlConnection == NULL){
            printfs(0,"<CWDSMinuteDataProcess::Do()> No enough mysql connections!");
            nStatus = 0;
            break;
        }
        pWDSHelper = new WDS_DBHelper();
        if (pWDSHelper == NULL){
            printfs(0,"<CWDSMinuteDataProcess::Do()> Can not connect to DB!");
            nStatus = 0;
            break;
        }
        pWDSHelper->SetMysqlHandle(pMysqlConnection);

        // step 1.业务处理，从DB取得当前台站的分钟气象数据个数
        int nCount = 0;
        nStatus = pWDSHelper->GetElementsMinuteDataCount(elementsMinuteData.cStationID, nCount);
        if (!nStatus){
            printfs(0,"<CWDSMinuteDataProcess::Do()> GetElementsMinuteDataCount() operation failed!");
            break;
        }
        if (nCount != 0){
            // step 2.1业务处理，向DB更新当前台站的分钟气象数据
            nStatus = pWDSHelper->UpdateElementsMinuteData(&elementsMinuteData);
        }
        else{
            // step 2.2业务处理，向DB插入当前台站的分钟气象数据
            nStatus = pWDSHelper->InsertElementsMinuteData(&elementsMinuteData);
        }
        if (!nStatus){
            printfs(0,"<CWDSMinuteDataProcess::Do()> Process elements minute data failed!");
            break;
        }

    }
    while(0);
    if (pWDSHelper){
        CObjectFactory::GetInstance()->GetMySQLPool()->SetIdleMysql(pWDSHelper->GetMysqlHandle());
        delete pWDSHelper;
        pWDSHelper = NULL;
    }

    stAnswer answerInfo;
    memset(&answerInfo, 0, sizeof(answerInfo));
    answerInfo.nVerify1 = elementsMinuteData.nVerify1;
    answerInfo.nStatus = nStatus;
    answerInfo.nVerify2 = elementsMinuteData.nVerify1;

    // 应答电文送信
    nRet = m_pRecvSocket->Send((char*)(&answerInfo), sizeof(answerInfo));
    if(nRet == 0) {
        printfs(0,"<CWDSMinuteDataProcess::Do()> Send status code time out");
    }
    if(nRet == -1) {
        printfs(0, "<CWDSMinuteDataProcess::Do()> Send status code failed");
    }
    m_pRecvSocket->Close();

    printfs(1,"<CWDSMinuteDataProcess::Do()> WDSMinuteDataProcess end!");
}

/******************************************************************************
    End
******************************************************************************/
