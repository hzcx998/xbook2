#ifndef _X86_VBE_H
#define _X86_VBE_H

#include <stdint.h>

/* VBE信息的内存地址 */
#define VBE_BASE_INFO_ADDR 0x00001100
#define VBE_BASE_MODE_ADDR 0x00001300

/* VBE信息块结构体 */
struct vbe_info_block {
    uint8_t vbeSignature[4];        /* VEB Signature: 'VESA' */
    uint16_t vbeVeision;            /* VEB Version:0300h */
    uint32_t oemStringPtr;          /* VbeFarPtr to OEM string */
    uint8_t capabilities[4];        /* Capabilities of graphics controller */
    uint32_t videoModePtr;          /* VbeFarPtr to VideoModeList */
    uint16_t totalMemory;           /* Number of 64kb memory blocks added for VEB2.0+ */
    uint16_t oemSoftwareRev;        /* VEB implementation Software revision */
    uint32_t oemVendorNamePtr;      /* VbeFarPtr to Vendor Name String */
    uint32_t oemProductNamePtr;     /* VbeFarPtr to Product Name String */
    uint32_t oemProductRevPtr;      /* VbeFarPtr to Product Revision String */
    uint8_t reserved[222];          /* Reserved for VBE implementation scratch area */
    uint8_t oemData[256];           /* Data Area for OEM String */
} __attribute__ ((packed));

struct vbe_mode_info_block {
    /* Mandatory information for all VBE revisions */
    uint16_t modeAttributes;        /* mode attributes */
    uint8_t winAAttributes;         /* window A attributes */
    uint8_t winBAttributes;         /* window B attributes */
    uint16_t winGranulaity;         /* window granulaity */
    uint16_t winSize;               /* window size */
    uint16_t winASegment;           /* window A start segment */
    uint16_t winBSegment;           /* window B start segment */
    uint32_t winFuncPtr;            /* real mode pointer to window function */
    uint16_t bytesPerScanLine;      /* bytes per scan line */
    /* Mandatory information for VBE1.2 and above */
    uint16_t xResolution;           /* horizontal resolution in pixels or characters */
    uint16_t yResolution;           /* vertical resolution in pixels or characters */
    uint8_t xCharSize;              /* character cell width in pixels */
    uint8_t yCharSize;              /* character cell height in pixels */
    uint8_t numberOfPlanes;         /* number of banks */
    uint8_t bitsPerPixel;           /* bits per pixel */
    uint8_t numberOfBanks;          /* number of banks */
    uint8_t memoryModel;            /* memory model type */
    uint8_t bankSize;               /* bank size in KB */
    uint8_t numberOfImagePages;     /* number of images */
    uint8_t reserved0;              /* reserved for page function: 1 */
    uint8_t redMaskSize;            /* size of direct color red mask in bits */
    uint8_t redFieldPosition;       /* bit position of lsb of red mask */
    uint8_t greenMaskSize;          /* size of direct color green mask in bits */
    uint8_t greenFieldPosition;     /* bit position of lsb of green mask */
    uint8_t blueMaskSize;           /* size of direct color blue mask in bits */
    uint8_t blueFieldPosition;      /* bit position of lsb of blue mask */
    uint8_t rsvdMaskSize;           /* size of direct color reserved mask in bits */
    uint8_t rsvdFieldPosition;      /* bit position of lsb of reserved mask */
    uint8_t directColorModeInfo;    /* direct color mode attributes */

    /* Mandatory information for VBE2.0 and above */
    uint32_t phyBasePtr;            /* physical address for flat memory frame buffer */
    uint32_t reserved1;             /* reserved-always set to 0 */
    uint16_t reserved2;             /* reserved-always set to 0 */
    /* Mandatory information for VBE3.0 and above */
    uint16_t linebytesPerScanLine;  /* bytes per scan line for linear modes */
    uint8_t bnkNumberOfImagePages;  /* number of images for banked modes */
    uint8_t linNumberOfImagePages;  /* number of images for linear modes */
    uint8_t linRedMaskSize;         /* size of direct color red mask(linear modes) */
    uint8_t linRedFieldPosition;    /* bit position of lsb of red mask(linear modes) */
    uint8_t linGreenMaskSize;       /* size of direct color green mask(linear modes) */
    uint8_t linGreenFieldPosition;  /* bit position of lsb of green mask(linear modes) */
    uint8_t linBlueMaskSize;        /* size of direct color blue mask(linear modes) */
    uint8_t linBlueFieldPosition;   /* bit position of lsb of blue mask(linear modes) */
    uint8_t linRsvdMaskSize;        /* size of direct color reserved mask(linear modes) */
    uint8_t linRsvdFieldPosition;   /* bit position of lsb of reserved mask(linear modes) */
    uint32_t maxPixelClock;         /* maximum pixel clock (in HZ) for graphics mode */
    uint8_t reserved3[189];         /* remainder of ModeInfoBlock */
}  __attribute__ ((packed));

#endif /* _X86_VBE_H */
