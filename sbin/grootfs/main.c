#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define USING_GZ        0

#if USING_GZ
#include <zlib.h>
#endif
#include <cpio.h>

#define CDROM_DEV       "/dev/cdrom"
#define ROOTFS_IMG      "XBOOK/ROOTFS.IMG"
#define ROOTFS_GZ       "/rootfs.img.gz"

#define KB              (1024)
#define MB              (1024 * KB)
#define ROOTFS_GZ_SIZE  (32 * MB)

#define ROOT_PATH       "/"
#define ROOT_PATH_LEN   (sizeof(ROOT_PATH))

#define PROCESS_STR     "/-\\|"

static int cdrom_dev;
static uint32_t sector_size = 0;
static uint8_t iso9660_data[2048] = {0};

static uint8_t iso9660_read(char* path, char *buffer);

static inline void print_process()
{
    static unsigned char i = 0;

    if (i != 0)
    {
        printf("\b");
    }

    if (i >= sizeof(PROCESS_STR) - 1)
    {
        i = 0;
    }

    printf("%c", PROCESS_STR[i++]);
}

int main(void)
{
    int status = 0;
    int i;
    int extract_fd;
    char *path;
    char *filename;
    size_t file_sz;
    uint8_t* file_buf;
    uint8_t *rootfs_gz_buff = NULL;
    struct cpio_info info;

#if USING_GZ
    int rootfs_fd;
    gzFile rootfs_gz;
#endif

    if ((cdrom_dev = open(CDROM_DEV, O_RDWR)) < 0)
    {
        status = -1;
        printf("cdrom device not found!\n");
        goto end;
    }

    if (ioctl(cdrom_dev, DISKIO_GETSECSIZE, (void *)&sector_size) < 0)
    {
        status = -1;
        printf("cdrom device get sector size fail!\n");
        goto end;
    }

    if ((rootfs_gz_buff = malloc(ROOTFS_GZ_SIZE)) == NULL)
    {
        status = -1;
        printf("no memory (%dMB) enough\n", ROOTFS_GZ_SIZE / MB);
        goto end;
    }

    memset(rootfs_gz_buff, 0, ROOTFS_GZ_SIZE);

    if (!iso9660_read(ROOTFS_IMG, (char *)rootfs_gz_buff))
    {
        status = -1;
        printf("get `" ROOTFS_IMG "' fail!\n");
        goto end;
    }

    printf("\b\nunpacking `" ROOTFS_IMG "'...");

#if USING_GZ
    if ((rootfs_fd = open(ROOTFS_GZ, O_CREAT | O_RDWR)) < 0)
    {
        status = -1;
        printf("can't create `" ROOTFS_GZ "'\n");
        goto end;
    }

    write(rootfs_fd, rootfs_gz_buff, ROOTFS_GZ_SIZE);
    close(rootfs_fd);
    memset(rootfs_gz_buff, 0, ROOTFS_GZ_SIZE);

    rootfs_gz = gzopen(ROOTFS_GZ, "r");
    gzread(rootfs_gz, rootfs_gz_buff, ROOTFS_GZ_SIZE);
#endif /* USING_GZ */

    cpio_info(rootfs_gz_buff, &info);
    if ((path = (char *)malloc(info.max_path_sz + ROOT_PATH_LEN)) == NULL)
    {
        status = -1;
        printf("no memory (%dKB) enough\n", (info.max_path_sz + ROOT_PATH_LEN) / KB);
        goto end;
    }

    for (i = 0; i < info.file_count; ++i)
    {
        file_buf = cpio_get_entry(rootfs_gz_buff, i, (const char**)&filename, &file_sz);
        strcpy(path, ROOT_PATH);
        strcpy(path + ROOT_PATH_LEN - 1, filename);
        
        print_process();

        if (file_sz == 0)
        {
            mkdir(path, 0777);
        }
        else
        {
            extract_fd = open(filename, O_CREAT | O_RDWR);
            write(extract_fd, file_buf, file_sz);
            close(extract_fd);
        }
    }

    printf("\b\n");

    free(path);

end:
    if (cdrom_dev >= 0)
    {
        close(cdrom_dev);
    }

    if (rootfs_gz_buff != NULL)
    {
        free(rootfs_gz_buff);
    }

#if USING_GZ
    remove(ROOTFS_GZ);
#endif

    return status;
}

static inline unsigned long charstoint(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    return ((a << 24) | (b << 16) | (c << 8) | d);
}

static inline void cdrom_read(uint8_t *data, unsigned long target)
{
    ioctl(cdrom_dev, DISKIO_SETOFF, &target);
    int rd = read(cdrom_dev, data, sector_size);
    if (rd <= 0)
    {
        printf("cdrom read on %d failed!\n", target);
    }
}

static unsigned long iso9660_target(char* path)
{
    int i;
    int index = strlen(path);
    int paths = 1;
    int depth = 1;
    int path_chunk_i = 0;
    char path_chunk[20];
    char is_bestand = 0;

    unsigned long ret;

    for (i = 0; i < index; ++i)
    {
        if (path[i] == '/')
        {
            paths++;
        }
        if (path[i] == '.')
        {
            is_bestand = 1;
        }
    }

    printf("there are %x pathelements and the path is a %s\n", paths, is_bestand ? "file" : "directory");

    for (i = 0; i < 10; ++i)
    {
        cdrom_read(iso9660_data, 0x10 + i);
        if (!strncmp((const char *)iso9660_data, "\1CD001", 6))
        {
            goto read_primaire_sector;
        }
    }

    printf("primairy sector not found!\n");

    return -1;

read_primaire_sector:
    cdrom_read(iso9660_data, charstoint(iso9660_data[148], iso9660_data[149], iso9660_data[150], iso9660_data[151]));

    ret = charstoint(iso9660_data[2], iso9660_data[3], iso9660_data[4], iso9660_data[5]);

    if (path[0] == 0)
    {
        return ret;
    }

    memset(path_chunk, 20, 0);

    for (index = i = 0; i < (paths - (is_bestand ? 1 : 0)); ++i)
    {
        uint8_t entry_text_len = 0;
        uint8_t entry_total_len = 0;
        uint8_t entry_tree = 0;
        int entry_index = 0;
        int depth_off = 0;
        int loop_times = 0;

        memset(path_chunk, 20, 0);
        path_chunk_i = 0;
cpoy_fragment:
        if (!(path[index] == '\0' || path[index] == '/'))
        {
            path_chunk[path_chunk_i++] = path[index++];
            goto cpoy_fragment;
        }
        ++index;
        path_chunk[path_chunk_i] = '\0';
        printf("looking for chunk `%s'...", path_chunk);

loop:
        ++loop_times;
        if (loop_times > 20)
        {
            printf("unable to find requested resource\n");
            return 0;
        }

        entry_text_len = iso9660_data[entry_index + 0];
        entry_total_len = iso9660_data[entry_index + 1];
        entry_tree = iso9660_data[entry_index + 7];

        if (entry_tree == depth && entry_text_len == path_chunk_i)
        {
            int j;
            for (j = 0; j < entry_text_len; j++)
            {
                if (iso9660_data[entry_index + 8 + j] != path_chunk[j])
                {
                    goto data_no_found;
                }
            }
            depth = depth_off + 1;
            ret = charstoint(iso9660_data[entry_index + 2], iso9660_data[entry_index + 3], iso9660_data[entry_index + 4], iso9660_data[entry_index + 5]);
            if ((paths - (is_bestand ? 1 : 0)) == (i + 1))
            {
                return ret;
            }
data_no_found:
            continue;
        }

        if ((entry_text_len + entry_total_len + 8) % 2 != 0)
        {
            ++entry_index;
        }
        entry_index += entry_text_len + entry_total_len + 8;

        depth_off++;
        goto loop;
    }

    return ret;
}

static uint8_t iso9660_read(char* path, char *buffer)
{
    unsigned long target = iso9660_target(path);

    if (target != 0 && target != -1)
    {
        int i = 0;
        char *filename = path;

        cdrom_read(iso9660_data, target);

        for (i = strlen(path) - 1; i >= 0; --i)
        {
            if (path[i] == '/')
            {
                filename = (char *)(path + i + 1);
                break;
            }
        }

        for (i = 0; i < 1000; i++)
        {
            print_process();
            if (iso9660_data[i] == ';' && iso9660_data[i + 1] == '1')
            {
                int j, k = 2;
                for (j = 1; j < 30; j++)
                {
                    if (iso9660_data[i - j] == k)
                    {
                        int l = 0;
                        for (j = 2; j < k; j++)
                        {
                            if (filename[l++] != iso9660_data[(i - k) + j])
                            {
                                goto data_no_found;
                            }
                        }
                        int off1 = i - k - 25;
                        int off2 = off1 + 8;
                        unsigned long lba = charstoint(iso9660_data[off1], iso9660_data[off1 + 1], iso9660_data[off1 + 2], iso9660_data[off1 + 3]);
                        unsigned long cnt = (charstoint(iso9660_data[off2], iso9660_data[off2 + 1], iso9660_data[off2 + 2], iso9660_data[off2 + 3]) / sector_size) + 1;

                        for (l = 0; l < cnt; l++)
                        {
                            if (l % 100 == 0)
                            {
                                print_process();
                            }
                            cdrom_read((uint8_t *)(buffer + (sector_size * l)), lba + l);
                        }
                        return 1;
data_no_found:
                        break;
                    }
                    k++;
                }
            }
        }
        *buffer++ = '\0';
    }
    else
    {
        *buffer = '\0';
    }
    return 0;
}
