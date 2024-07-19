#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "btstack/a2dp_media_codec.h"
#include "app_config.h"
#include "app_msg.h"
#include "a2dp_player.h"
#include "bt_tws.h"
#include "earphone.h"


void bt_dual_phone_call_msg_handler(int *msg)
{
    struct bt_event *event = (struct bt_event *)msg;

    if (tws_api_get_role_async() == TWS_ROLE_SLAVE) {
        return;
    }

    /*
     * device_a为当前发生事件的设备, device_b为另外一个设备
     */
    void *device_a, *device_b;
    bt_get_btstack_device(event->args, &device_a, &device_b);

    u8 *addr_a = event->args;
    u8 *addr_b = device_b ? btstack_get_device_mac_addr(device_b) : NULL;
    if (!device_b) {
        return;
    }
    int status = btstack_bt_get_call_status(device_b);

    switch (event->event) {
    case BT_STATUS_PHONE_INCOME:
        printf("BT_STATUS_PHONE_INCOME: status = %d\n", status);
        if (status  == BT_CALL_INCOMING) {
            // 先来电的优先级高于后来电
            puts("disconn_sco-a1\n");
            btstack_device_control(device_a, USER_CTRL_DISCONN_SCO);
        } else if (status == BT_CALL_OUTGOING) {
            // 去电的优先级高于来电
            puts("disconn_sco-a2\n");
            btstack_device_control(device_a, USER_CTRL_DISCONN_SCO);
        } else if (status == BT_CALL_ACTIVE &&
                   btstack_get_call_esco_status(device_b) == BT_ESCO_STATUS_OPEN) {
            // 通话的优先级高于来电
            puts("disconn_sco-a3\n");
            btstack_device_control(device_a, USER_CTRL_DISCONN_SCO);
        }
        break;
    case BT_STATUS_PHONE_OUT:
        printf("BT_STATUS_PHONE_OUT: status = %d\n", status);
        if (status == BT_CALL_ACTIVE &&
            btstack_get_call_esco_status(device_b) == BT_ESCO_STATUS_OPEN) {
            puts("disconn_sco-a\n");
            btstack_device_control(device_a, USER_CTRL_DISCONN_SCO);
            break;
        }
        if (status == BT_CALL_OUTGOING) {
            puts("disconn_sco-b1\n");
            btstack_device_control(device_b, USER_CTRL_DISCONN_SCO);
        } else if (status == BT_CALL_INCOMING) {
            puts("disconn_sco-b2\n");
            btstack_device_control(device_b, USER_CTRL_DISCONN_SCO);
        }
        break;
    case BT_STATUS_SCO_CONNECTION_REQ:
        printf("BT_STATUS_SCO_CONNECTION_REQ: status = %d\n", status);
        if (btstack_bt_get_call_status(device_a) == BT_CALL_ACTIVE) {
            // 设备A切声卡, 抢占设备B的去电
            if (status == BT_CALL_OUTGOING &&
                btstack_get_call_esco_status(device_b) == BT_ESCO_STATUS_OPEN) {
                puts("disconn_sco-b1\n");
                btstack_device_control(device_b, USER_CTRL_DISCONN_SCO);
                break;
            }
            if (status == BT_CALL_ACTIVE &&
                btstack_get_call_esco_status(device_b) == BT_ESCO_STATUS_OPEN) {
                puts("disconn_sco-b2\n");
#if 1
                // 设备A切声卡, 不抢占设备B的通话
                btstack_device_control(device_a, USER_CTRL_DISCONN_SCO);
#else
                // 设备A切声卡, 抢占设备B的通话
                btstack_device_control(device_b, USER_CTRL_DISCONN_SCO);
#endif
                break;
            }

        }
        break;
    }

}

