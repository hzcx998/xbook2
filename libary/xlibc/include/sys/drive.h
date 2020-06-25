#ifndef _SYS_DRIVE_H
#define _SYS_DRIVE_H

#define DRIVE_MIN   'a'
#define DRIVE_MAX   'z'

#define ISBAD_DRIVE(drive)  ((drive) < DRIVE_MIN || (drive) > DRIVE_MAX)

#endif   /* _SYS_DRIVE_H */