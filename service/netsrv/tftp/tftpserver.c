/* tftpsercer.c */

#include "tftpserver.h"

#if TFTP_SERVER_ON == 1
#include "tftputils.h"
#include <stdio.h>
#include <string.h>

#define MFS_MODE_READ 0
#define MFS_MODE_WRITE 1

#define TFTP_OPCODE_LEN         2
#define TFTP_BLKNUM_LEN         2
#define TFTP_ERRCODE_LEN        2
#define TFTP_DATA_LEN_MAX       512
#define TFTP_DATA_PKT_HDR_LEN   (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_ERR_PKT_HDR_LEN    (TFTP_OPCODE_LEN + TFTP_ERRCODE_LEN)
#define TFTP_ACK_PKT_LEN        (TFTP_OPCODE_LEN + TFTP_BLKNUM_LEN)
#define TFTP_DATA_PKT_LEN_MAX   (TFTP_DATA_PKT_HDR_LEN + TFTP_DATA_LEN_MAX)
#define TFTP_MAX_RETRIES        3
#define TFTP_TIMEOUT_INTERVAL   5


typedef struct
{
  int op;    /* RRQ/WRQ */

  /* last block read */
  char data[TFTP_DATA_PKT_LEN_MAX];
  int  data_len;
  
  u16_t remote_port;

  /* next block number */
  u16_t block;

  /* total number of bytes transferred */
  u32_t tot_bytes;
}tftp_connection_args;

/* server pcb to bind to port 69  */
struct udp_pcb *tftp_server_pcb = NULL;
/* tftp_errorcode error strings */
char *tftp_errorcode_string[] = {
                                  "not defined",
                                  "file not found",
                                  "access violation",
                                  "disk full",
                                  "illegal operation",
                                  "unknown transfer id",
                                  "file already exists",
                                  "no such user",
                                };

err_t tftp_send_message(struct udp_pcb *upcb, struct ip_addr *to_ip, int to_port, char *buf, int buflen)
{

  err_t err;
  struct pbuf *pkt_buf = NULL; /* Chain of pbuf's to be sent */

  /* PBUF_TRANSPORT - specifies the transport layer */
  pkt_buf = pbuf_alloc(PBUF_TRANSPORT, buflen, PBUF_RAM);

  if (!pkt_buf)      /*if the packet pbuf == NULL exit and EndTransfertransmission */
    return ERR_MEM;

  /* Copy the original data buffer over to the packet buffer's payload */
  memcpy(pkt_buf->payload, buf, buflen);

  /* Sending packet by UDP protocol */
  err = udp_send(upcb, pkt_buf);

  /* free the buffer pbuf */
  pbuf_free(pkt_buf);
  //printf("\n\tftp_send_message OK....,buflen = %d",buflen);
  return err;
}


/* construct an error message into buf using err as the error code */
int tftp_construct_error_message(char *buf, tftp_errorcode err)
{

  int errorlen;
  /* Set the opcode in the 2 first bytes */
  tftp_set_opcode(buf, TFTP_ERROR);
  /* Set the errorcode in the 2 second bytes  */
  tftp_set_errorcode(buf, err);
  /* Set the error message in the last bytes */
  tftp_set_errormsg(buf, tftp_errorcode_string[err]);
  /* Set the length of the error message  */
  errorlen = strlen(tftp_errorcode_string[err]);

  /* return message size */
  return 4 + errorlen + 1;
}

/* construct and send an error message back to client */
int tftp_send_error_message(struct udp_pcb *upcb, struct ip_addr *to, int to_port, tftp_errorcode err)
{
  char buf[512];
  int error_len;

  /* construct error */
  error_len = tftp_construct_error_message(buf, err);
  /* sEndTransfererror  */
  return tftp_send_message(upcb, to, to_port, buf, error_len);
}

/* construct and send a data packet */
int tftp_send_data_packet(struct udp_pcb *upcb, struct ip_addr *to, int to_port, int block,
                          char *buf, int buflen)
{
  //char packet[TFTP_DATA_PKT_LEN_MAX]; /* (512+4) bytes */
  //memset(packet, 'a', TFTP_DATA_PKT_LEN_MAX);
  /* Set the opcode 3 in the 2 first bytes */
  tftp_set_opcode(buf, TFTP_DATA);
  /* Set the block numero in the 2 second bytes */
  tftp_set_block(buf, block);
  /* Set the data message in the n last bytes */
  //@@@@tftp_set_data_message(packet, buf, buflen);
  //tftp_set_data_message(packet, packet, buflen);
  /* SEndTransferthe DATA packet */
  return tftp_send_message(upcb, to, to_port, buf, buflen + 4);
}

int tftp_send_ack_packet(struct udp_pcb *upcb, struct ip_addr *to, int to_port, int block)
{

  /* create the maximum possible size packet that a TFTP ACK packet can be */
  char packet[TFTP_ACK_PKT_LEN];

  /* define the first two bytes of the packet */
  tftp_set_opcode(packet, TFTP_ACK);

  /* Specify the block number being ACK'd.
   * If we are ACK'ing a DATA pkt then the block number echoes that of the DATA pkt being ACK'd (duh)
   * If we are ACK'ing a WRQ pkt then the block number is always 0
   * RRQ packets are never sent ACK pkts by the server, instead the server sends DATA pkts to the
   * host which are, obviously, used as the "acknowledgement".  This saves from having to sEndTransferboth
   * an ACK packet and a DATA packet for RRQs - see RFC1350 for more info.  */
  tftp_set_block(packet, block);

  return tftp_send_message(upcb, to, to_port, packet, TFTP_ACK_PKT_LEN);
}

/* close the file sent, disconnect and close the connection */
void tftp_cleanup_rd(struct udp_pcb *upcb, tftp_connection_args *args)
{
  /* Free the tftp_connection_args structure reserverd for */
  mem_free(args);

  /* Disconnect the udp_pcb*/
  udp_disconnect(upcb);

  /* close the connection */
  udp_remove(upcb);

}

/* close the file writen, disconnect and close the connection */
void tftp_cleanup_wr(struct udp_pcb *upcb, tftp_connection_args *args)
{
  /* Free the tftp_connection_args structure reserverd for */
  mem_free(args);

  /* Disconnect the udp_pcb*/
  udp_disconnect(upcb);

  /* close the connection */
  udp_remove(upcb);
}

void tftp_send_next_block(struct udp_pcb *upcb, tftp_connection_args *args,
                          struct ip_addr *to_ip, u16_t to_port)
{
  /* Function to read 512 bytes from the file to sEndTransfer(file_SD), put them
   * in "args->data" and return the number of bytes read */
  //@@@@args->data_len = file_read(&file_SD, TFTP_DATA_LEN_MAX, (euint8*)args->data);
  int total_block = args->tot_bytes/TFTP_DATA_LEN_MAX;
  total_block +=1;
  //if(args->tot_bytes%TFTP_DATA_LEN_MAX != 0)
  //{
//		total_block += 1;
  //}

  if(total_block < 1 || args->block > total_block )
  {
       return;
  }

  args->data_len = TFTP_DATA_LEN_MAX;
  if(total_block == args->block)
  {
	   if(args->tot_bytes%TFTP_DATA_LEN_MAX == 0)
	   {
	       args->data_len = 0;
	   }else
	   {
	       args->data_len = args->tot_bytes - (total_block - 1)*TFTP_DATA_LEN_MAX;
	   }
  }
  
  memset(args->data + TFTP_DATA_PKT_HDR_LEN, ('a'-1) + args->block%26 , args->data_len);
  /*   NOTE: We need to sEndTransferanother data packet even if args->data_len = 0
     The reason for this is as follows:
     1) This function is only ever called if the previous packet payload was
        512 bytes.
     2) If args->data_len = 0 then that means the file being sent is an exact
         multiple of 512 bytes.
     3) RFC1350 specifically states that only a payload of <= 511 can EndTransfera
        transfer.
     4) Therefore, we must sEndTransferanother data message of length 0 to complete
        the transfer.                */


  /* sEndTransferthe data */
  tftp_send_data_packet(upcb, to_ip, to_port, args->block, args->data, args->data_len);

}

void rrq_recv_callback(void *_args, struct udp_pcb *upcb, struct pbuf *p,
                       struct ip_addr *addr, u16_t port)
{
  /* Get our connection state  */
  tftp_connection_args *args = (tftp_connection_args *)_args;
  if(port != args->remote_port)
  {
    /* Clean the connection*/
    tftp_cleanup_rd(upcb, args);

    pbuf_free(p);
	return;
  }
  //printf("PUT rrq_recv_callback\n");
  if (tftp_is_correct_ack(p->payload, args->block))
  {
    /* increment block # */
    args->block++;
	//printf("rrq_recv_callback ACK OK\n");
  }
  else
  {
    /* we did not receive the expected ACK, so
       do not update block #. This causes the current block to be resent. */
    //printf("rrq_recv_callback ACK UNOK\n");
  }

  /* if the last read returned less than the requested number of bytes
   * (i.e. TFTP_DATA_LEN_MAX), then we've sent the whole file and we can quit
   */
  if (args->data_len < TFTP_DATA_LEN_MAX)
  {
    /* Clean the connection*/
    tftp_cleanup_rd(upcb, args);

    pbuf_free(p);
	printf("rrq_recv_callback send over\n");
	return;
  }

  /* if the whole file has not yet been sent then continue  */
  tftp_send_next_block(upcb, args, addr, port);

  pbuf_free(p);
  //printf("rrq_recv_callback send next block\n");

}

int tftp_process_read(struct udp_pcb *upcb, struct ip_addr *to, int to_port, char* FileName)
{
  tftp_connection_args *args = NULL;

  /* If Could not open the file which will be transmitted  */
  //@@@@if (file_fopen(&file_SD, &efs1.myFs, FileName, 'r') != 0)
  if(0)
  {
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_FILE_NOT_FOUND);

    tftp_cleanup_rd(upcb, args);

    return 0;
  }

  /* This function is called from a callback,
   * therefore, interrupts are disabled,
   * therefore, we can use regular malloc. */

  args = mem_malloc(sizeof(tftp_connection_args));
  /* If we aren't able to allocate memory for a "tftp_connection_args" */
  if (!args)
  {
    /* unable to allocate memory for tftp args  */
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_NOTDEFINED);

    /* no need to use tftp_cleanup_rd because no 
            "tftp_connection_args" struct has been malloc'd   */
    tftp_cleanup_rd(upcb, args);

    return 0;
  }

  /* initialize connection structure  */
  args->op = TFTP_RRQ;
  //args->to_ip.addr = to->addr;
  args->remote_port = to_port;
  args->block = 1; /* block number starts at 1 (not 0) according to RFC1350  */
  args->tot_bytes = 1024;


  /* set callback for receives on this UDP PCB (Protocol Control Block) */
  udp_recv(upcb, rrq_recv_callback, args);

  /* initiate the transaction by sending the first block of data
   * further blocks will be sent when ACKs are received
   *   - the receive callbacks need to get the proper state    */

  tftp_send_next_block(upcb, args, to, to_port);

  return 1;
}

void wrq_recv_callback(void *_args, struct udp_pcb *upcb, struct pbuf *pkt_buf, struct ip_addr *addr, u16_t port)
{
  tftp_connection_args *args = (tftp_connection_args *)_args;
  int n = 0;
  u16_t next_block = 0;
  
  if (port != args->remote_port || pkt_buf->len != pkt_buf->tot_len)
  {
    tftp_cleanup_wr(upcb, args);
    pbuf_free(pkt_buf);
    return;
  }

  next_block = args->block + 1;
  /* Does this packet have any valid data to write? */
  if ((pkt_buf->len > TFTP_DATA_PKT_HDR_LEN) &&
      (tftp_extract_block(pkt_buf->payload) == next_block))
  {
    /* write the received data to the file */
    //@@@@n = file_write(&file_CR,
    //@@@@               pkt_buf->len - TFTP_DATA_PKT_HDR_LEN,
    //@@@@               (euint8*)pkt_buf->payload + TFTP_DATA_PKT_HDR_LEN);

    //@@@@if (n <= 0)
    if(0)
    {
      tftp_send_error_message(upcb, addr, port, TFTP_ERR_FILE_NOT_FOUND);
      /* close the connection */
      tftp_cleanup_wr(upcb, args); /* close the connection */
    }

    /* update our block number to match the block number just received */
    args->block++;
    /* update total bytes  */
    (args->tot_bytes) += (pkt_buf->len - TFTP_DATA_PKT_HDR_LEN);

    printf("data: %s\n", (char *)pkt_buf->payload + 4);

    /* This is a valid pkt but it has no data.  This would occur if the file being
       written is an exact multiple of 512 bytes.  In this case, the args->block
       value must still be updated, but we can skip everything else.    */
  }
  else if (tftp_extract_block(pkt_buf->payload) == next_block)
  {
    /* update our block number to match the block number just received  */
    args->block++;
  }
  else
  {
	printf("ZZZSL ERROR = %d\n" ,(args->block + 1));
  }

  /* SEndTransferthe appropriate ACK pkt (the block number sent in the ACK pkt echoes
   * the block number of the DATA pkt we just received - see RFC1350)
   * NOTE!: If the DATA pkt we received did not have the appropriate block
   * number, then the args->block (our block number) is never updated and
   * we simply sEndTransfera "duplicate ACK" which has the same block number as the
   * last ACK pkt we sent.  This lets the host know that we are still waiting
   * on block number args->block+1. */
  tftp_send_ack_packet(upcb, addr, port, args->block);

  /* If the last write returned less than the maximum TFTP data pkt length,
   * then we've received the whole file and so we can quit (this is how TFTP
   * signals the EndTransferof a transfer!)
   */
  if (pkt_buf->len < TFTP_DATA_PKT_LEN_MAX)
  {
    tftp_cleanup_wr(upcb, args);
    pbuf_free(pkt_buf);
  }
  else
  {
    pbuf_free(pkt_buf);
    return;
  }

}

int tftp_process_write(struct udp_pcb *upcb, struct ip_addr *to, int to_port, char *FileName)
{
  tftp_connection_args *args = NULL;

  /* If Could not open the file which will be transmitted  */
  //@@@@if (file_fopen(&file_CR, &efs2.myFs, FileName, 'w') != 0)
  if(0)
  {
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_FILE_ALREADY_EXISTS);

    tftp_cleanup_wr(upcb, args);

    return 0;
  }

  /* This function is called from a callback,
   * therefore interrupts are disabled,
   * therefore we can use regular malloc   */
  args = mem_malloc(sizeof(tftp_connection_args));
  if (!args)
  {
    tftp_send_error_message(upcb, to, to_port, TFTP_ERR_NOTDEFINED);

    tftp_cleanup_wr(upcb, args);

    return 0;
  }

  args->op = TFTP_WRQ;
  //args->to_ip.addr = to->addr;
  args->remote_port = to_port;
  /* the block # used as a positive response to a WRQ is _always_ 0!!! (see RFC1350)  */
  args->block = 0;
  args->tot_bytes = 0;

  /* set callback for receives on this UDP PCB (Protocol Control Block) */
  udp_recv(upcb, wrq_recv_callback, args);

  /* initiate the write transaction by sending the first ack */
  tftp_send_ack_packet(upcb, to, to_port, args->block);

  return 0;
}

/* for each new request (data in p->payload) from addr:port,
 * create a new port to serve the response, and start the response
 * process
 */
void process_tftp_request(struct pbuf *pkt_buf, struct ip_addr *addr, u16_t port)
{
  tftp_opcode op = tftp_decode_op(pkt_buf->payload);
  char FileName[50] = {0};
  struct udp_pcb *upcb = NULL;
  err_t err;
  u32_t IPaddress;

  
  IPaddress = addr->addr;
  printf("\n\rTFTP RRQ from: [%d.%d.%d.%d]:[%d]\n\r", (u8_t)(IPaddress), \
        (u8_t)(IPaddress >> 8),(u8_t)(IPaddress >> 16),(u8_t)(IPaddress >> 24), port);
  
  /* create new UDP PCB structure */
  upcb = udp_new();
  if (!upcb)
  {     /* Error creating PCB. Out of Memory  */
    printf("alloc upcb error\n");
    return;
  }
  
  /* connect to remote ip:port, and a random local port will be set too */
  /* NOTE:  This is how TFTP works.  There is a UDP PCB for the standard port
   * 69 which all transactions begin communication on, however, _all_ subsequent
   * transactions for a given "stream" occur on another port!  */
  err = udp_connect(upcb, addr, port);
  if (err != ERR_OK)
  {    /* Unable to bind to port   */
    printf("connect upcb error\n");
    return;
  }
  
  tftp_extract_filename(FileName, pkt_buf->payload);

  switch (op)
  {

    case TFTP_RRQ:    /* TFTP RRQ (read request)  */
      /* Read the name of the file asked by the client 
                            to be sent from the SD card */
      //tftp_extract_filename(FileName, pkt_buf->payload);

      //printf("\n\rTFTP RRQ (read request)");
      //printf("\n\rONLY EFS filesystem(NTFS in WinXp) is support");
      
      /* If Could not open filesystem */
      //@@@@if (efs_init(&efs1, 0) != 0)
      //@@@@{
      //@@@@printf("\n\rIf Could not open filesystem");
      //@@@@return;
      //@@@@}
      
      /* If Could not open the selected directory */
      //@@@@if (ls_openDir(&list1, &(efs1.myFs), "/") != 0)
      //@@@@{
      //@@@@  printf("\n\rIf Could not open the selected directory");
      //@@@@  return;
      //@@@@}
      /* Start the TFTP read mode*/
      printf("\n\rTFTP client start to read file..[%s]..", FileName);
      tftp_process_read(upcb, addr, port, FileName);
      break;

    case TFTP_WRQ:    /* TFTP WRQ (write request)   */
      /* Read the name of the file asked by the client 
                to received and writen in the SD card */
      //tftp_extract_filename(FileName, pkt_buf->payload);

      /* If Could not open filesystem */
      //@@@@if (efs_init(&efs2, 0) != 0)
      if(0)
      {
        return;
      }
      /* If Could not open the selected directory */
      //@@@@if (ls_openDir(&list2, &(efs2.myFs), "/") != 0)
      if(0)
      {
        return;
      }
	  printf("\n\rTFTP client start to write file..[%s]..", FileName);

      /* Start the TFTP write mode*/
      tftp_process_write(upcb, addr, port, FileName);
      break;

    default:
      /* sEndTransfera generic access violation message */
      tftp_send_error_message(upcb, addr, port, TFTP_ERR_ACCESS_VIOLATION);
      /* TFTP unknown request op */
      /* no need to use tftp_cleanup_wr because no 
            "tftp_connection_args" struct has been malloc'd   */
      udp_remove(upcb);

      break;
  }
}

/* the recv_callback function is called when there is a packet received
 * on the main tftp server port (69)
 */
void server_recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                        struct ip_addr *addr, u16_t port)
{
  /* process new connection request */
  
  process_tftp_request(p, addr, port);

  pbuf_free(p);
}

#define TFTP_PORT 69
void tftp_server_init(void)
{
  err_t err;

  /* create a new UDP PCB structure  */
  tftp_server_pcb = udp_new();
  if (NULL == tftp_server_pcb)
  {  /* Error creating PCB. Out of Memory  */
    return;
  }

  /* Bind this PCB to port 69  */
  err = udp_bind(tftp_server_pcb, IP_ADDR_ANY, TFTP_PORT);
  if (err != ERR_OK)
  {    /* Unable to bind to port  */
    return;
  }

  /* TFTP server start  */
  udp_recv(tftp_server_pcb, server_recv_callback, NULL);
}
#endif