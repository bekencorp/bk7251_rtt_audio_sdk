//
//��ͷ�ļ�ʹ����RTosϵͳ
//��������ͫ��������Ȩ��¼�ͷ�ҳ����㷨�����������κ������⣬
//���������ص���������������߼���sdkʹ�÷�ʵ�֡�
//

#ifndef _VTBK_H__
#define _VTBK_H__

#include "vttypes.h"

int vt_init(StatedetParam *stateParam);                  //��ʼ��
int vt_destroy();                                        //�ͷ��ڴ�
int vt_setLoginRequestFunc(loginRequest fun);            //��Ȩ��¼��������
int vt_setLicenseText(const char *text);                 //license�ļ�����
int vt_setDeviceID(const char *deviceId);                //�豸ID,Ψһ���У���MAC��SN��
int vt_setModelType(const char *model);                  //�豸ģ�����ƣ���ͫ�ṩ
int vt_login();                                          //ʹ��������������Ϣ��Ȩ��¼
DetState vt_stateDetect(ImageWrapper img);                //��һ֡ͼƬ����״̬���

#endif
