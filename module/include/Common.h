/*****************************************************************************/
/*                                                                           */
/*           ϵͳ��        ��        ��������ֺ�̨����                      */
/*           �ͻ���        ��        ���������                              */
/*           ������        ��        ��ͨ����                                */
/*           ������        ��        Common.h                                */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*                VERSION    DATE      BY      CHANGE/COMMENT                */
/*---------------------------------------------------------------------------*/
/*                V1.00      09-11-11          Create                        */
/*                                                                           */
/*****************************************************************************/

#ifndef TDM_COMMON_H_
#define TDM_COMMON_H_

int CheckRealtimeStatus(char cStatusSrc, char cCompareDest);

int IsAlertChanged(char cStatusSrc, char cCompareDest, int nOldAlert);

bool CreateFolder(char* pFolder);

#endif /* TDM_COMMON_H_ */ 
