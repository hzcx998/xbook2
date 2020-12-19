#include <netsocket.h>
#include <net.client.h>
#include <sys/lpc.h>

int net_socket(int domain, int type, int protocol)
{
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_int(parcel, domain);
    lpc_parcel_write_int(parcel, type);
    lpc_parcel_write_int(parcel, protocol);
    if (lpc_call(LPC_ID_NET, NETCALL_socket, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int num = -1;
    lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
}

