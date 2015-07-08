/*****************************************************************************/
/*                                                                           */
/*           ϵͳ��        ��        ��������ֺ�̨����                      */
/*           �ͻ���        ��        ���������                              */
/*           ������        ��        �����̻���                              */
/*           ������        ��        Weather_Data_service.cpp                */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                VERSION    DATE      BY      CHANGE/COMMENT                */
/*---------------------------------------------------------------------------*/
/*                V1.00      09-11-11          Create                        */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "ProcessFactory.h"
#include "ChannelItem.h"
#include "ProcessThread.h"
#include "SystemConfigInfo.h"

/******************************************************************************
    ������        ��  Main����
    ������        ��  main(int argc, char* argv[])
    ����          ��  (I)    ��argc - �����в�������
                      (I)    ��argv - �����в����б�
    ����ֵ        ��  =  0   ����������
                      <> 0   ���쳣����
******************************************************************************/
int main(int argc, char* argv[])
{

    CProcessFactory*    pFactoryInstance = NULL; /* ����ģ��ʵ������         */
    CObjectFactory*     pObjectInstance  = NULL; /* ����ʵ������             */

    //CLog* //pSysLogger = NULL;
    /* ȡ����־����ģ��                                                      */
    pObjectInstance  = CObjectFactory::GetInstance();
    CChannelItem* pChannelItem = NULL;
    /* �����в�������                                                        */
    int c = 0;
    int nCount = 0;
    while ((c = getopt(argc, argv, "c:s:h")) != EOF)
    {
        switch(c)
        {
            case 'c':
                pObjectInstance->SetChannelConfFile(optarg);
                nCount++;
                break;
            case 's':
                pObjectInstance->SetSystemConfFile(optarg);
                nCount++;
                break;
            case 'h':
                printf("\nUsage : Weather_Data_service [OPTION]\n%s%s%s",
                "\n-c\twds_channel_config.ini file path",
                "\n-s\twds_system_config.ini file path",
                "\n-h\tprint this helper\n");
                break;
            default:
                break;
        }
    }
    if (nCount == 0){
        if ( pObjectInstance != NULL ){
            delete pObjectInstance;
            pObjectInstance = NULL;
        }
        printf("\nUsage : Weather_Data_service [OPTION]\n%s%s%s",
                "\n-c\twds_channel_config.ini file path",
                "\n-s\twds_system_config.ini file path",
                "\n-h\tprint this helper\n");
        return 0;
    }

    CSystemConfigInfo* pConfigInfo = pObjectInstance->GetSystemConfigInfo();
    //pSysLogger = pObjectInstance->GetSysLogger();
    printfs(1, "Start Weather_Data_service Application!");

    /* ȡ�������д���ģ��                                                    */
    pFactoryInstance = CProcessFactory::GetInstance();
    pChannelItem = new CChannelItem(pConfigInfo->GetRecvPort());
    /* ���������д����߳�                                                    */
    try{
        printfs(1, "Start pChannelItem->Create()!");
        pChannelItem->Start();
        printfs(1, "Start pChannelItem->Join()!");
        pChannelItem->Join();
        printfs(1, "End pChannelItem->Join()!");
    }
    catch(...){
        pChannelItem->Stop();
        printfs(1, "Exception occured!!");
    }

    if ( pChannelItem != NULL ){
        delete pChannelItem;
        pChannelItem = NULL;
    }

    sleep(10);
    
    char s[1024];
    fgets(s,1024,stdin);
    while(strcmp(s,"exit\n")!=0){
        fgets(s,1024,stdin);
        //printf("%s",s);
    }

    //pthread_exit(NULL);

    printfs(1, "Exit Weather_Data_service Application!");
    pObjectInstance = CObjectFactory::GetInstance();
    if ( pObjectInstance != NULL ){
        delete pObjectInstance;
        pObjectInstance = NULL;
    }

    if ( pFactoryInstance != NULL ){
        delete pFactoryInstance;
        pFactoryInstance = NULL;
    }

    return 0;
}
/******************************************************************************
    End
******************************************************************************/
