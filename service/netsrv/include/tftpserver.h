
#ifndef __TFTPSERVER_H_
#define __TFTPSERVER_H_

#define TFTP_SERVER_ON  0

#if TFTP_SERVER_ON == 1
#include <lwip/mem.h>
#include <lwip/udp.h>


/* TFTP opcodes as specified in RFC1350   */
typedef enum {
  TFTP_RRQ = 1,
  TFTP_WRQ = 2,
  TFTP_DATA = 3,
  TFTP_ACK = 4,
  TFTP_ERROR = 5
} tftp_opcode;


/* TFTP error codes as specified in RFC1350  */
typedef enum {
  TFTP_ERR_NOTDEFINED,
  TFTP_ERR_FILE_NOT_FOUND,
  TFTP_ERR_ACCESS_VIOLATION,
  TFTP_ERR_DISKFULL,
  TFTP_ERR_ILLEGALOP,
  TFTP_ERR_UKNOWN_TRANSFER_ID,
  TFTP_ERR_FILE_ALREADY_EXISTS,
  TFTP_ERR_NO_SUCH_USER,
} tftp_errorcode;

void tftp_server_demo(void *pdata);
void tftp_server_init(void);
int tftp_process_write(struct udp_pcb *upcb2, struct ip_addr *to, int to_port, char* FileName);
int tftp_process_read(struct udp_pcb *upcb2, struct ip_addr *to, int to_port, char* FileName);
#endif
#endif

