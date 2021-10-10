/*
 * Copyright 2014, NICTA
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(NICTA_BSD)
 */

#ifndef _LIB_CPIO_H_
#define _LIB_CPIO_H_

/* Magic identifiers for the "cpio" file format. */
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_ALIGNMENT 4

struct cpio_header {
    char c_magic[6];      /* Magic header '070701'. */
    char c_ino[8];        /* "i-node" number. */
    char c_mode[8];       /* Permisions. */
    char c_uid[8];        /* User ID. */
    char c_gid[8];        /* Group ID. */
    char c_nlink[8];      /* Number of hard links. */
    char c_mtime[8];      /* Modification time. */
    char c_filesize[8];   /* File size. */
    char c_devmajor[8];   /* Major dev number. */
    char c_devminor[8];   /* Minor dev number. */
    char c_rdevmajor[8];
    char c_rdevminor[8];
    char c_namesize[8];   /* Length of filename in bytes. */
    char c_check[8];      /* Checksum. */
};


/**
 * Stores information about the underlying implementation.
 */
struct cpio_info {
    /// The number of files in the CPIO archive
    unsigned int file_count;
    /// The maximum size of a file name
    unsigned int max_path_sz;
};


/**
 * Retrieve file information from a provided CPIO list index
 * @param[in] archive  The location of the CPIO archive
 * @param[in] index    The index of the CPIO entry to query
 * @param[out] name    A pointer to the file name of the entry. This name is not
 *                     NULL terminated but it will not exceed max_path_sz as
 *                     reported by cpio_info.
 * @param[out] size    The size of the file in question
 * @return             The location of the file in memory; NULL if the index
 *                     exceeds the number of files in the CPIO archive.
 */
void *cpio_get_entry(void *archive, int index, const char **name, unsigned long *size);

/**
 * Retrieve file information from a provided file name
 * @param[in] archive  The location of the CPIO archive
 * @param[in] name     The name of the file in question.
 * @param[out] size    The retrieved size of the file in question
 * @return             The location of the file in memory; NULL if the file
 *                     does not exist.
 */
void *cpio_get_file(void *archive, const char *name, unsigned long *size);

/**
 * Retrieves information about the provided CPIO archive
 * @param[in] archive  The location of the CPIO archive
 * @param[out] info    A CPIO info structure to populate
 * @return             Non-zero on error.
 */
int cpio_info(void *archive, struct cpio_info *info);

/**
 * Writes the list of file names contained within a CPIO archive into 
 * a provided buffer
 * @param[in] archive  The location of the CPIO archive
 * @param[in] buf      A memory location to store the CPIO file list to
 * @param[in] buf_len  The length of the provided buf
 */
void cpio_ls(void *archive, char **buf, unsigned long buf_len);

#endif /* _LIB_CPIO_H_ */
