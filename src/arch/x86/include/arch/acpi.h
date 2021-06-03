#ifndef _X86_ACPI_H
#define _X86_ACPI_H

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

extern dword *SMI_CMD;
extern byte ACPI_ENABLE;
extern byte ACPI_DISABLE;
extern dword *PM1a_CNT;
extern dword *PM1b_CNT;
extern word SLP_TYPa;
extern word SLP_TYPb;
extern word SLP_EN;
extern word SCI_EN;
extern byte PM1_CNT_LEN;

struct RSDPtr {
    byte Signature[8];
    byte CheckSum;
    byte OemID[6];
    byte Revision;
    dword *RsdtAddress;
};

struct FACP {
    byte Signature[4];
    dword Length;
    byte unneded1[40 - 8];
    dword *DSDT;
    byte unneded2[48 - 44];
    dword *SMI_CMD;
    byte ACPI_ENABLE;
    byte ACPI_DISABLE;
    byte unneded3[64 - 54];
    dword *PM1a_CNT_BLK;
    dword *PM1b_CNT_BLK;
    byte unneded4[89 - 72];
    byte PM1_CNT_LEN;
};

unsigned int *acpi_check_RSDPtr(unsigned int *ptr);
unsigned int *acpi_get_RSDPtr(void);
int acpi_checkHeader(unsigned int *ptr, char *sig);
int acpi_enable(void);
int acpi_init(void);

#endif /* _X86_ACPI_H */
