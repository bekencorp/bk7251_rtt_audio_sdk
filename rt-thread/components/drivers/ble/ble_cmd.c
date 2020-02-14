#include <rtthread.h>
#include <finsh.h>
#include "common.h"
#include "ble_pub.h"
#include "app_sdp.h"
#include "param_config.h"
#include "app_task.h"

#define BLE_DEBUG   1
#if BLE_DEBUG
#define BLE_DBG(...)     rt_kprintf("[BLE]"),rt_kprintf(__VA_ARGS__)
#else
#define BLE_DBG(...)
#endif

#define BUILD_UINT16(loByte, hiByte) \
          ((uint16_t)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

int _atoi(char *str)
{
  int value = 0;
  while(*str>='0' && *str<='9')
  {
    value *= 10;
    value += *str - '0';
    str++;
  }
  return value;
}

static void ble_usage(void)
{
    rt_kprintf("ble help            - Help information\n");
    rt_kprintf("ble dut             - Enter ble dut mode\n");
    rt_kprintf("ble active          - Active ble to config network\n");
    rt_kprintf("ble start_adv [channel_map=7] [interval_max=160] [interval_min=160]\n");
    rt_kprintf("                    - Start advertising as a slave device\n");
    rt_kprintf("ble stop_adv        - Stop advertising as a slave device\n");
    rt_kprintf("ble send param value\n");
    rt_kprintf("                    - Send value to master\n");
    rt_kprintf("ble start_scan [channel_map=7] [window=20] [interval=100]\n");
    rt_kprintf("                    - Start scanning as a host device\n");
    rt_kprintf("ble stop_scan       - Stop scan as a host device\n");
    rt_kprintf("ble conn_id [idx=0] - Connect with index as a host device\n");
    rt_kprintf("ble conn_addr addr  - Connect with address as a host device\n");
    rt_kprintf("ble disc_master     - Disconnect as a host device\n");
    rt_kprintf("ble write uuid value\n");
    rt_kprintf("                    - Write value to slave\n");
}

void ble_write_callback(uint16_t char_id, uint8_t *data, uint8_t len)
{
    rt_kprintf("%s char_id=0x%04x, len=%d\n", __FUNCTION__, char_id, len);
}

uint8_t ble_read_callback(uint16_t char_id, uint8_t *data, uint8_t len)
{
    rt_kprintf("%s char_id=0x%04x, len=%d\n", __FUNCTION__, char_id, len);
    data[0] = 'a';
    data[1] = 'b';
    return 2;
}

void ble_event_callback(ble_event_t event, void *param)
{
    switch(event)
    {
        case BLE_STACK_OK:
            rt_kprintf("STACK INIT OK\r\n");
        break;
        case BLE_STACK_FAIL:
            rt_kprintf("STACK INIT FAIL\r\n");
        break;
        case BLE_CONNECT:
            rt_kprintf("BLE CONNECT\r\n");
        break;
        case BLE_DISCONNECT:
        {
            rt_kprintf("BLE DISCONNECT\r\n");
        }
        break;
        case BLE_MTU_CHANGE:
            rt_kprintf("BLE_MTU_CHANGE:%d\r\n", *(uint16_t *)param);
        break;
        case BLE_CFG_NOTIFY:
            rt_kprintf("BLE_CFG_NOTIFY:%d\r\n", *(uint16_t *)param);
        break;
        case BLE_CFG_INDICATE:
            rt_kprintf("BLE_CFG_INDICATE:%d\r\n", *(uint16_t *)param);
        break;
        case BLE_TX_DONE:
            rt_kprintf("BLE_TX_DONE\r\n");
        break;
        case BLE_GEN_DH_KEY:
        {
            rt_kprintf("BLE_GEN_DH_KEY\r\n");
            rt_kprintf("key_len:%d\r\n", ((struct ble_gen_dh_key_ind *)param)->len);
            for(int i = 0; i < ((struct ble_gen_dh_key_ind *)param)->len; i++)
            {
                rt_kprintf("%02x ", ((struct ble_gen_dh_key_ind *)param)->result[i]);
            }
            rt_kprintf("\r\n");
        }    
        break;
        case BLE_GET_KEY:
        {
            rt_kprintf("BLE_GET_KEY\r\n");
            rt_kprintf("pri_len:%d\r\n", ((struct ble_get_key_ind *)param)->pri_len);
            for(int i = 0; i < ((struct ble_get_key_ind *)param)->pri_len; i++)
            {
                rt_kprintf("%02x ", ((struct ble_get_key_ind *)param)->pri_key[i]);
            }
            rt_kprintf("\r\n");
        }    
        break;
        default:
            rt_kprintf("UNKNOW EVENT\r\n");
        break;
    }
}

void ble_recv_adv_callback(uint8_t *buf, uint8_t len)
{
#if (BLE_APP_CLIENT)
    uint8_t find = 0;

    find = appm_adv_data_decode(len, buf, NULL, 0);

    if(find)
    {
        extern int *scan_check_result;
        if(scan_check_result)
        {
            *scan_check_result = 2;
            bk_printf("scan_check_result\r\n");
        }
    }
#endif
}

void ble_adv(ble_adv_param_t *adv_param)
{
    uint8_t mac[6];
    char ble_name[20];
    uint8_t adv_idx, adv_name_len;

    wifi_get_mac_address((char *)mac, CONFIG_ROLE_STA);
    adv_name_len = rt_snprintf(ble_name, sizeof(ble_name), "bk-%02x%02x", mac[4], mac[5]);

    memset(&adv_info, 0x00, sizeof(adv_info));

    if(adv_param)
    {
        adv_info.channel_map = adv_param->channel_map;
        adv_info.interval_min = adv_param->interval_min;
        adv_info.interval_max = adv_param->interval_max;
    }

    adv_idx = 0;
    adv_info.advData[adv_idx] = 0x02; adv_idx++;
    adv_info.advData[adv_idx] = 0x01; adv_idx++;
    adv_info.advData[adv_idx] = 0x06; adv_idx++;

    adv_info.advData[adv_idx] = adv_name_len + 1; adv_idx +=1;
    adv_info.advData[adv_idx] = 0x09; adv_idx +=1; //name
    memcpy(&adv_info.advData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;

    adv_info.advDataLen = adv_idx;

    adv_idx = 0;

    adv_info.respData[adv_idx] = adv_name_len + 1; adv_idx +=1;
    adv_info.respData[adv_idx] = 0x08; adv_idx +=1; //name
    memcpy(&adv_info.respData[adv_idx], ble_name, adv_name_len); adv_idx +=adv_name_len;
    adv_info.respDataLen = adv_idx;

    if (ERR_SUCCESS != appm_start_advertising())
    {
        rt_kprintf("ERROR\r\n");
    }
}

int ble(int argc, char **argv)
{
    uint8_t mac[6];
    char ble_name[20];
    ble_adv_param_t adv_param;
    
    if ((argc < 2) || (strcmp(argv[1], "help") == 0))
    {
        ble_usage();
        return 0;
    }

    if (strcmp(argv[1], "active") == 0)
    {
        ble_activate(NULL);
        ble_set_write_cb(ble_write_callback);
        ble_set_read_cb(ble_read_callback);
        ble_set_event_cb(ble_event_callback);
        ble_set_recv_adv_cb(ble_recv_adv_callback);
    }
    else if(strcmp(argv[1], "start_adv") == 0)
    {
        /* ble start_adv channel interval_max interval_min */
        if (argc > 4)
        {
            adv_param.interval_min = _atoi(argv[4]);
        }
        else
        {
            adv_param.interval_min = 160;
        }
        if (argc > 3)
        {
            adv_param.interval_max = _atoi(argv[3]);
        }
        else
        {
            adv_param.interval_max = 160;
        }
        if (argc > 2)
        {
            adv_param.channel_map = _atoi(argv[2]);
        }
        else
        {
            adv_param.channel_map = 7;
        }
        rt_kprintf("channel_map=%d,interval=[%d,%d]\n", adv_param.channel_map, adv_param.interval_min,adv_param.interval_max);

        ble_adv(&adv_param);
    }
    else if(strcmp(argv[1], "stop_adv") == 0)
    {
        if(ERR_SUCCESS != appm_stop_advertising())
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "send") == 0)
    {
        uint8_t len;
        uint8_t write_buffer[20];
        if(argc != 4)
        {
            ble_usage();
            return 0;
        }

        len = strlen(argv[3]);
        if(len % 2 != 0)
        {
            rt_kprintf("ERROR\r\n");
            return 0;
        }
        hexstr2bin(argv[3], write_buffer, len/2);

        if(strcmp(argv[2], "ayla_statu") == 0)
        {
#if (BLE_APP_AYLA_WIFI)
            ayla_wifi_send_statu_ntf_value(len/2, write_buffer, 0xff);
#else
            rt_kprintf("unvalid param\r\n");
#endif
        }
        else if(strcmp(argv[2], "ayla_scre") == 0)
        {
#if (BLE_APP_AYLA_WIFI)
            ayla_wifi_send_scre_ntf_value(len/2, write_buffer, 0xff);
#else
            rt_kprintf("unvalid param\r\n");
#endif
        }
        else
        {
            rt_kprintf("unvalid param\r\n");
        }
    }
#if (BLE_APP_CLIENT)
    else if(strcmp(argv[1], "start_scan") == 0)
    {
        /* ble start_scan channel window interval */
        if (argc > 4)
        {
            scan_info.interval = _atoi(argv[4]);
        }
        else
        {
            scan_info.interval = 100;
        }
        if (argc > 3)
        {
            scan_info.window = _atoi(argv[3]);
        }
        else
        {
            scan_info.window = 20;
        }
        if (argc > 2)
        {
            scan_info.channel_map = _atoi(argv[2]);
        }
        else
        {
            scan_info.channel_map = 7;
        }
        rt_kprintf("channel_map=%d,window=%d,interval=%d\n", scan_info.channel_map, scan_info.window,scan_info.interval);
        if(ERR_SUCCESS != appm_start_scanning())
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "stop_scan") == 0)
    {
        if(ERR_SUCCESS != appm_stop_scanning())
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "disc_master") == 0)
    {
        if(ERR_SUCCESS != appm_disconnect_link())
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "disc_slave") == 0)
    {
        appm_disconnect(0x13);
    }
    else if (strcmp(argv[1], "conn_id") == 0)
    {
        int idx = 0;
        //char to int
        if (argc > 2)
        {
            idx = _atoi(argv[2]);
        }
        rt_kprintf("start connect, idx = %d\r\n", idx);
        if(ERR_SUCCESS != appm_start_connenct_by_id(idx))
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "conn_addr") == 0)
    {
        uint8_t mac_addr[6];
        if (argc < 3)
        {
            ble_usage();
            return 0;
        }
        hexstr2bin(argv[2], mac_addr, 6);
        rt_kprintf("start connect\r\n");
        if(ERR_SUCCESS != appm_start_connenct_by_addr(mac_addr))
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "stop_conn") == 0)
    {
        if(ERR_SUCCESS != appm_stop_connencting())
        {
            rt_kprintf("ERROR\r\n");
        }
    }
    else if(strcmp(argv[1], "write") == 0)
    {
        uint8_t len;
        uint8_t hex_uuid[2];
        uint8_t write_buffer[20];
        uint16_t uuid;

        hexstr2bin(argv[2], hex_uuid, 2);
        uuid = BUILD_UINT16(hex_uuid[1], hex_uuid[0]);
        len = strlen(argv[3]);
        if(len % 2 != 0)
        {
            rt_kprintf("ERROR\r\n");
            return 0;
        }
        hexstr2bin(argv[3], write_buffer, len/2);
        rt_kprintf("write uuid = 0x%x\r\n", uuid);
        rt_kprintf("write len = %d\r\n", len/2);
        if(!appm_write_data_by_uuid(uuid, len/2, write_buffer))
        {
            rt_kprintf("ERROR\r\n");
        }

    }
#endif

    else if (strcmp(argv[1], "get_key") == 0)
    {
        appm_get_key();
    }
    else if (strcmp(argv[1], "calc_key") == 0)
    {
        uint8_t pub_peer_key[64] = {0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0,1,2,3};
        appm_calc_psk(pub_peer_key);
    }

    return 0;
}

MSH_CMD_EXPORT(ble, ble command);

