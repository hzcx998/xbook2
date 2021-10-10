/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#include <cpio.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

/* Align 'n' up to the value 'align', which must be a power of two. */
static unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

/* Parse an ASCII hex string into an integer. */
static unsigned long parse_hex_str(char *s, unsigned int max_len)
{
    unsigned long r = 0;
    unsigned long i;

    for (i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
        continue;
    }
    return r;
}

/*
 * Compare up to 'n' characters in a string.
 *
 * We re-implement the wheel to avoid dependencies on 'libc', required for
 * certain environments that are particularly impoverished.
 */
static int cpio_strncmp(const char *a, const char *b, unsigned long n)
{
    unsigned long i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
        if (a[i] == 0) {
            return 0;
        }
    }
    return 0;
}

/**
 * This is an implementation of string copy because, cpi doesn't want to
 * use string.h.
 */
static char* cpio_strcpy(char *to, const char *from) {
    char *save = to;
    while (*from != 0) {
        *to = *from;
        to++;
        from++;
    }
    return save;
}


static unsigned int cpio_strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s) {}
    return (s - str);
}



/*
 * Parse the header of the given CPIO entry.
 *
 * Return -1 if the header is not valid, 1 if it is EOF.
 */
int cpio_parse_header(struct cpio_header *archive,
        const char **filename, unsigned long *_filesize, void **data,
        struct cpio_header **next)
{
    unsigned long filesize;
    /* Ensure magic header exists. */
    if (cpio_strncmp(archive->c_magic, CPIO_HEADER_MAGIC,
                sizeof(archive->c_magic)) != 0)
        return -1;

    /* Get filename and file size. */
    filesize = parse_hex_str(archive->c_filesize, sizeof(archive->c_filesize));
    *filename = ((char *)archive) + sizeof(struct cpio_header);

    /* Ensure filename is not the trailer indicating EOF. */
    if (cpio_strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
        return 1;

    /* Find offset to data. */
    unsigned long filename_length = parse_hex_str(archive->c_namesize,
            sizeof(archive->c_namesize));
    *data = (void *)align_up(((unsigned long)archive)
            + sizeof(struct cpio_header) + filename_length, CPIO_ALIGNMENT);
    *next = (struct cpio_header *)align_up(((unsigned long)*data) + filesize, CPIO_ALIGNMENT);
    if(_filesize){
        *_filesize = filesize;
    }
    return 0;
}

/*
 * Get the location of the data in the n'th entry in the given archive file.
 *
 * We also return a pointer to the name of the file (not NUL terminated).
 *
 * Return NULL if the n'th entry doesn't exist.
 *
 * Runs in O(n) time.
 */
void *cpio_get_entry(void *archive, int n, const char **name, unsigned long *size)
{
    int i;
    struct cpio_header *header = archive;
    void *result = NULL;

    /* Find n'th entry. */
    for (i = 0; i <= n; i++) {
        struct cpio_header *next;
        int error = cpio_parse_header(header, name, size, &result, &next);
        if (error)
            return NULL;
        header = next;
    }

    return result;
}

/*
 * Find the location and size of the file named "name" in the given 'cpio'
 * archive.
 *
 * Return NULL if the entry doesn't exist.
 *
 * Runs in O(n) time.
 */
void *cpio_get_file(void *archive, const char *name, unsigned long *size)
{
    struct cpio_header *header = archive;

    /* Find n'th entry. */
    while (1) {
        struct cpio_header *next;
        void *result;
        const char *current_filename;

        int error = cpio_parse_header(header, &current_filename,
                size, &result, &next);
        if (error)
            return NULL;
        if (cpio_strncmp(current_filename, name, -1) == 0)
            return result;
        header = next;
    }
}

int cpio_info(void *archive, struct cpio_info *info) {
    struct cpio_header *header, *next;
    const char *current_filename;
    void *result;
    int error;
    unsigned long size, current_path_sz;

    if (info == NULL) return 1;
    info->file_count = 0;
    info->max_path_sz = 0;

    header = archive;
    while (1) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        if (error == -1) {
            return error;
        } else if (error == 1) {
            /* EOF */
            return 0;
        }
        info->file_count++;
        header = next;

        // Check if this is the maximum file path size.
        current_path_sz = cpio_strlen(current_filename);
        if (current_path_sz > info->max_path_sz) {
            info->max_path_sz = current_path_sz;    
        }
    }

    return 0;
}


void cpio_ls(void *archive, char **buf, unsigned long buf_len) {
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long i, size;

    header = archive;
    for (i = 0; i < buf_len; i++) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;
        cpio_strcpy(buf[i],  current_filename);
        header = next;
    }
}
