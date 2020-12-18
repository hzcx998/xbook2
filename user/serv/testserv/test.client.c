#include <sys/lpc.h>
#include <test.client.h>

int hello(char *str) 
{
    #if 0
    lpc_parcel_t parcel = lpc_parcel_get();
    lpc_parcel_write_string(parcel, str);
    lpc_parcel_write_string(parcel, str);
    char seqbuf[32];
    int i; for (i = 0; i < 32; i++) {
        seqbuf[i] = 'a' + i;
    }
    lpc_parcel_write_sequence(parcel, seqbuf, 32);
    lpc_parcel_write_int(parcel, 10);
    lpc_parcel_write_int(parcel, 20);
    lpc_parcel_dump_args(parcel);
    uint32_t num;
    lpc_parcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    lpc_parcel_read_int(parcel, &num);
    printf("read num:%d\n", num);
    char *sbuf;
    lpc_parcel_read_string(parcel, &sbuf);
    printf("read string:%s\n", sbuf);
    lpc_parcel_read_string(parcel, &sbuf);
    printf("read string:%s\n", sbuf);
    size_t len;
    lpc_parcel_read_sequence(parcel, &sbuf, &len);
    printf("read seq:%s len: %d\n", sbuf + 1, len);
    lpc_parcel_dump_args(parcel);
    #else
    lpc_parcel_t parcel = lpc_parcel_get();
    if (!parcel) {
        return -1;
    }
    lpc_parcel_write_string(parcel, str);
    if (lpc_call(LPC_ID_TEST, TESTSERV_hello, parcel, parcel) < 0) {
        lpc_parcel_put(parcel);
        return -1;
    }
    int num; lpc_parcel_read_int(parcel, (uint32_t *)&num);
    lpc_parcel_put(parcel);
    return num;
    #endif
    return 0;
}
