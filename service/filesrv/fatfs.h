#ifndef _FATFS_H
#define _FATFS_H

FRESULT fatfs_scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
);

#endif  /* _FATFS_H */