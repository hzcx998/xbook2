#include <dwin/client.h>
#include <dwin/hal.h>
#include <dwin/workstation.h>

/* 基于线程的客户端，接口更加简洁，封装得更简单。 */

/**
 * NOTE: layer must create and added to workstation
 */
int dwin_client_init(dwin_client_t *client, dwin_layer_t *layer)
{
    if (client == NULL)
    {
        return -EINVAL;
    }    
    client->layer = layer;
    client->state = DWIN_CLIENT_CONNECT;
    client->flags = 0;
    return 0;
}

int dwin_client_detach(dwin_client_t *client)
{
    if (client == NULL)
    {
        return -EINVAL;
    }    
    client->layer = NULL;
    client->state = DWIN_CLIENT_INIT;
    client->flags = 0;
    return 0;
}

int dwin_client_open(dwin_client_t *client)
{
    if (client == NULL)
    {
        return -EINVAL;
    }
    client->state = DWIN_CLIENT_INIT;
    client->layer = NULL;   /* no layer */
    client->flags = 0;
    return 0;
}

int dwin_client_close(dwin_client_t *client)
{
    if (client == NULL)
    {
        return -EINVAL;
    }

    if (client->layer != NULL)
    {
        if (dwin_layer_delete(client->layer) < 0)
        {
            return -EPERM;
        }
        client->layer = NULL;
    }

    return 0;
}

int dwin_client_read(dwin_client_t *client, void *buf)
{
    if (client == NULL || buf == NULL)
    {
        return -EINVAL;
    }

    if (client->state != DWIN_CLIENT_CONNECT)
    {
        return -EBUSY;
    }

    if (client->layer == NULL)
    {
        return -EFAULT;
    }
    
    return dwin_layer_recv_message(client->layer, buf, client->flags);
}

int dwin_client_write(dwin_client_t *client, void *buf)
{
    if (client == NULL || buf == NULL)
    {
        return -EINVAL;
    }

    if (client->state != DWIN_CLIENT_CONNECT)
    {
        return -EBUSY;
    }

    if (client->layer == NULL)
    {
        return -EFAULT;
    }
    
    return dwin_layer_send_message(client->layer, buf, client->flags);
}

int dwin_client_ioctl(dwin_client_t *client, int cmd, void *cmd_arg)
{
    if (client == NULL)
    {
        return -EINVAL;
    }
    dwin_client_arg_t *arg = (dwin_client_arg_t *)cmd_arg; 
    if (cmd != DWC_CREATE)
    {
        if (client->state != DWIN_CLIENT_CONNECT)
        {
            return -EBUSY;
        }

        if (client->layer == NULL)
        {
            return -EFAULT;
        }
    }

    switch (cmd)
    {
    case DWC_CREATE:
        client->layer = dwin_layer_create(arg->create.width, arg->create.height, arg->create.flags);
        if (client->layer == NULL)
        {
            return -ENOMEM;
        }
        if (dwin_workstation_add_layer(dwin_current_workstation, client->layer) < 0)
        {
            dwin_layer_destroy(client->layer);
            client->layer = NULL;
            return -EPERM;
        }
        client->state = DWIN_CLIENT_CONNECT;
        break;
    case DWC_DELETE:
        if (dwin_layer_delete(client->layer) < 0)
        {
            return -EPERM;
        }
        client->state = DWIN_CLIENT_DISCONNECT;
        break;
    case DWC_ZORDER:
        dwin_layer_zorder(client->layer, arg->zorder.z);
        break;
    case DWC_CHANGE_PRIORITY:
        dwin_layer_change_priority(client->layer, arg->priority.priority);
        break;
    case DWC_MOVE:
        if (dwin_layer_move(client->layer, arg->move.x, arg->move.y) < 0)
        {
            return -EPERM;
        }
        break;
    case DWC_RELMOVE:
        if (dwin_layer_move(client->layer, client->layer->x + arg->relmove.x, client->layer->y + arg->relmove.y) < 0)
        {
            return -EPERM;
        }
        break;
    default:
        break;
    }
    return 0;
}

static struct dwin_thread *desktop_thread;
static struct dwin_thread *client_thread_1;

void dwin_desktop(void *thread_arg)
{
    dwin_log("desktop thread running\n");
    dwin_layer_t *layer = dwin_workstation_get_lowest_layer(dwin_current_workstation);
    
    dwin_client_t clien;
    dwin_client_init(&clien, layer);
    
    dwin_message_t msg;
    dwin_message_zero(&msg);
    
    while (1)
    {
        if (!dwin_client_read(&clien, &msg))
        {
            dwin_log("desktop recv msg:%d\n", msg.mid);
        }
        else
        {
            break;
        }
    }
    dwin_log("desktop thread exit\n");
    dwin_client_detach(&clien);
    dwin_client_close(&clien);
    
    dwin_hal->thread->stop(desktop_thread, 0);
}

void dwin_client_1(void *thread_arg)
{
    dwin_log("client 1 thread running\n");

    dwin_client_t client;
    if (dwin_client_open(&client) != 0)
    {
        dwin_log("client open client failed!\n");
        return;    
    }

    dwin_client_arg_t arg;
    arg.create.width = 400;
    arg.create.height = 300;
    arg.create.flags = 0;
    if (dwin_client_ioctl(&client, DWC_CREATE, &arg) != 0)
    {
        dwin_log("client create layer failed!\n");
        return;    
    }

    dwin_layer_draw_rect(client.layer, 0, 0, client.layer->width, client.layer->height, 0XFFFF0000);

    arg.priority.priority = DWIN_LAYER_PRIO_WINDOW;
    if (dwin_client_ioctl(&client, DWC_CHANGE_PRIORITY, &arg) != 0)
    {
        dwin_log("client priority layer failed!\n");
        return;    
    }

    arg.zorder.z = 0;
    if (dwin_client_ioctl(&client, DWC_ZORDER, &arg) != 0)
    {
        dwin_log("client zorder layer failed!\n");
        return;
    }

    arg.move.x = 100;
    arg.move.y = 200;
    if (dwin_client_ioctl(&client, DWC_MOVE, &arg) != 0)
    {
        dwin_log("client zorder layer failed!\n");
        return;
    }

    int click_x = -1;
    int click_y = -1;

    int dis_x = -1;
    int dis_y = -1;

    dwin_message_t msg;
    dwin_message_zero(&msg);
    while (1)
    {
        if (!dwin_client_read(&client, &msg))
        {
            if (msg.lid != client.layer->id)
            {            
                dwin_log(">>> layer check failed!\n");
            }
            
            // dwin_log("client recv msg:%d\n", msg.mid);

            switch (msg.mid)
            {
            case DWM_MOUSE_MOTION:
                // dwin_log("%d,%d,%d,%d\n", msg.data[0], msg.data[1], msg.data[2], msg.data[3]); 
                if (click_x != -1 && click_y != -1) /* clicked */
                {
                    /* clicked and moved */
                    if (click_x != msg.data[2] || click_y != msg.data[3])
                    {
                        /* TODO: 锁定鼠标状态 */

                        /* move window */
                        arg.move.x = msg.data[2] - dis_x;
                        arg.move.y = msg.data[3] - dis_y;
                        dwin_client_ioctl(&client, DWC_MOVE, &arg);
                    }
                }
                break;
            case DWM_MOUSE_LBTN_DOWN:
                dis_x = msg.data[0];
                dis_y = msg.data[1];
                click_x = msg.data[2];
                click_y = msg.data[3];

                dwin_log("%d,%d,%d,%d\n", msg.data[0], msg.data[1], msg.data[2], msg.data[3]); 
                break;
            case DWM_MOUSE_LBTN_UP:
                /* TODO: 解锁鼠标状态 */
                        
                click_x = -1;
                click_y = -1;
                dwin_log("%d,%d,%d,%d\n", msg.data[0], msg.data[1], msg.data[2], msg.data[3]); 
                break;
            default:
                break;
            }

        }
        else
        {
            break;
        }
    }
    dwin_log("client exit\n");
    dwin_client_close(&client);

    dwin_hal->thread->stop(client_thread_1, 0);
}

void dwin_client_test(void)
{
    if ((desktop_thread = dwin_hal->thread->start(dwin_desktop, NULL)) == NULL)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }
    if ((client_thread_1 = dwin_hal->thread->start(dwin_client_1, NULL)) == NULL)
    {
        dwin_hal_exit();
        dwin_log(DWIN_TAG"init failed\n");
        return;
    }

}