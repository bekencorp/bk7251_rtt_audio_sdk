/*
 * File: tts_player_sample.c
 * COPYRIGHT (C) 2012-2018, Shanghai Real-Thread Technology Co., Ltd
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-08-13     liu2guang    add player memops example.
 */

#include "rtthread.h"
#include "player.h" 
#include "player_init.h" 

static struct player_mem_ops mem_ops = 
{
    rt_malloc,   /* PLAYER �ڲ������ڴ� */
    rt_free,     /* PLAYER �ڲ������ڴ� */
    rt_realloc,  /* PLAYER �ڲ������ڴ� */
    rt_malloc,   /* PLAYER �ڲ������ڴ� */
    rt_free,     /* PLAYER �ڲ������ڴ� */ 
    psram_rpipe, /* PLAYER ���绺���ڴ� */ 
    psram_wpipe, /* PLAYER ���绺���ڴ� */ 
    RT_NULL,     /* PLAYER ���绺���ַ, �ⲿ������� */ 
    0            /* PLAYER ���绺���С(Bytes) */ 
}; 

static rt_size_t psram_rpipe(void *from, void *to, rt_size_t size)
{ 
    /* ��һ����Ҫ��fromָ�뿪ʼ��size��С�����ݰ��˵�toָ����ַ */ 
    rt_memcpy(from, to, size); 
    return 0;
}

static rt_size_t psram_wpipe(void *from, void *to, rt_size_t size)
{
    /* ��һ����Ҫ��fromָ�뿪ʼ��size��С�����ݰ��˵�toָ����ַ */ 
    rt_memcpy(from, to, size); 
    return 0;
}

rt_err_t memops_example(void)
{
    rt_err_t ret = RT_EOK; 

    /* ʹ�� 96KBytes �ڴ� */
    #define PIPE_SIZE (96*1024) 

    rt_uint8_t *ppipe = RT_NULL; 
	ppipe = (rt_uint8_t *)rt_malloc(PIPE_SIZE); 
    if(ppipe == RT_NULL)
    {
        rt_kprintf("ppipe malloc failed\n"); 
        return (-RT_ERROR); 
    }
    mem_ops._pipe_addr = (int)ppipe; 
    mem_ops._pipe_size = (int)PIPE_SIZE; 
    player_mem_ops_register(&mem_ops); 

    /* ��Ҫ��ע������ mem ops ���ʼ�� player */ 
    player_system_init();

    return ret; 
} 
