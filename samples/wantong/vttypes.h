#ifndef __VTTYPES_H__
#define __VTTYPES_H__
#include "wantong_httpclient.h"


typedef struct _detState {
    char *error;    //Ĭ����Ϣnull
    char ret;       //������Ƿ�ɹ�
    int state;      //���״̬��1�����������������0�����Ե�ǰ״̬��-1��������-2��license����
} DetState;

typedef struct ImageWrapper {
    unsigned char *data;    // ͼ�������
    int width;              // ���
    int height;             // �߶�
    int channels;           // ͨ����
    unsigned int size;      // ͼ�����ݵ��ֽ���
} ImageWrapper;

typedef struct StatedetParam {
    int x, y, w, h;
    int ds_factor;          //һ�㸳ֵΪ4
    int in_type;            //1: �Ҷ�ͼ; 2: NV21��YUVͼƬ
} StatedetParam;

//��Ȩ��¼������ص�����, ��һ��������url���ڶ����������ڴ�ŷ��صĽ���������ǿ�������ָ��ռ��������ͷţ�
typedef void (*loginRequest)(char *, LoginResult *);

#endif
