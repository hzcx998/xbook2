
#ifndef __TFTP_UTILS_H_
#define __TFTP_UTILS_H_

#include <tftpserver.h>

#if TFTP_SERVER_ON == 1
tftp_opcode tftp_decode_op(char *buf);
void tftp_extract_filename(char *fname, char *buf);
u16_t tftp_extract_block(char *buf);
void tftp_set_opcode(char *buffer, tftp_opcode opcode);
void tftp_set_errorcode(char *buffer, tftp_errorcode errCode);
void tftp_set_errormsg(char * buffer, char* errormsg);
u32_t tftp_is_correct_ack(char *buf, int block);
void tftp_set_data_message(char* packet, char* buf, int buflen);
void tftp_set_block(char* packet, u16_t block);
#endif
#endif
