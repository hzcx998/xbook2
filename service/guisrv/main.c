#include <string.h>
#include <stdio.h>
#include <stdio.h>
#include <sys/res.h>
#include <sys/ioctl.h>
#include <sys/vmm.h>

#include "guisrv.h"

int main(int argc, char *argv[])
{
    printf("%s: started.\n", SRV_NAME);
    
    int resid = res_open("video", RES_DEV, 0);
    if (resid < 0) {
        printf("%s: open video res failed! stop srvice.\n", SRV_NAME);    
    }
    printf("%s: open video res %d.\n", SRV_NAME, resid);
 
    video_info_t video_info;
    res_ioctl(resid, VIDEOIO_GETINFO, (unsigned long) &video_info);
    printf("%s: video: bits per pixel: %d\n", SRV_NAME, video_info.bits_per_pixel);
    printf("%s: video: x resolution: %d\n", SRV_NAME, video_info.x_resolution);
    printf("%s: video: y resolution: %d\n", SRV_NAME, video_info.y_resolution);
    printf("%s: video: bytes per scan line: %d\n", SRV_NAME, video_info.bytes_per_scan_line);
    int memsz = video_info.bytes_per_scan_line * video_info.y_resolution;
    void *mapaddr = res_mmap(resid, memsz, 0);
    if (mapaddr == NULL) {
        printf("%s: video mapped failed!\n", SRV_NAME);
        return -1;
    }

    printf("%s: mapped addr %x\n", SRV_NAME, mapaddr);
    memset(mapaddr, 0xff, memsz);

    munmap(mapaddr, memsz);

    int i = 0;
	while(1)
	{
        i += 5;
        memset(mapaddr, i, memsz);
	}
    return 0;
}
