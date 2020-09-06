/*
 * flushbuf.c - flush a buffer
 */
/* $Header: flushbuf.c,v 1.6 91/06/10 17:07:10 ceriel Exp $ */

#include	<stdio.h>
#include	<stdlib.h>
#include	"loc_incl.h"

#include	<sys/types.h>
#include	<unistd.h>

//int _isatty(int d);
extern void (*_clean)(void);

static int
_isatty(int d)
{
    return 1;
}

static int
dowrite(int d, char *buf, int nbytes)
{
	int c;

	/* POSIX actually allows write() to return a positive value less
	   than nbytes, so loop ...
	*/
	while ((c = write(d, buf, nbytes)) > 0 && c < nbytes) {
		nbytes -= c;
		buf += c;
	}
	return c > 0;
}

int
__flushbuf(int c, FILE * stream)
{
	_clean = __cleanup;
	if (fileno(stream) < 0) return EOF;
	if (!io_testflag(stream, _IOWRITE)) return EOF;
	if (io_testflag(stream, _IOREADING) && !feof(stream)) return EOF;

	stream->_flags &= ~_IOREADING;
	stream->_flags |= _IOWRITING;
	if (!io_testflag(stream, _IONBF)) {
		if (!stream->_buf) {
			if (stream == stdout && _isatty(fileno(stdout))) {
				if (!(stream->_buf =
					    (unsigned char *) malloc(BUFSIZ))) {
					stream->_flags |= _IONBF;
				} else {
					stream->_flags |= _IOLBF|_IOMYBUF;
					stream->_bufsiz = BUFSIZ;
					stream->_count = -1;
				}
			} else {
				if (!(stream->_buf =
					    (unsigned char *) malloc(BUFSIZ))) {
					stream->_flags |= _IONBF;
				} else {
					stream->_flags |= _IOMYBUF;
					stream->_bufsiz = BUFSIZ;
					if (!io_testflag(stream, _IOLBF))
						stream->_count = BUFSIZ - 1;
					else	stream->_count = -1;
				}
			}
			stream->_ptr = stream->_buf;
		}
	}

	if (io_testflag(stream, _IONBF)) {
		char c1 = c;

		stream->_count = 0;
		if (io_testflag(stream, _IOAPPEND)) {
			if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
				stream->_flags |= _IOERR;
				return EOF;
			}
		}
		if (write(fileno(stream), &c1, 1) != 1) {
			stream->_flags |= _IOERR;
			return EOF;
		}
		return c;
	} else if (io_testflag(stream, _IOLBF)) {
		*stream->_ptr++ = c;
		if (c == '\n' || stream->_count == -stream->_bufsiz) {
			if (io_testflag(stream, _IOAPPEND)) {
				if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
					stream->_flags |= _IOERR;
					return EOF;
				}
			}
			if (! dowrite(fileno(stream), (char *)stream->_buf,
					-stream->_count)) {
				stream->_flags |= _IOERR;
				return EOF;
			} else {
				stream->_ptr  = stream->_buf;
				stream->_count = 0;
			}
		}
	} else {
		int count = stream->_ptr - stream->_buf;

		stream->_count = stream->_bufsiz - 1;
		stream->_ptr = stream->_buf + 1;

		if (count > 0) {
			if (io_testflag(stream, _IOAPPEND)) {
				if (lseek(fileno(stream), 0L, SEEK_END) == -1) {
					stream->_flags |= _IOERR;
					return EOF;
				}
			}
			if (! dowrite(fileno(stream), (char *)stream->_buf, count)) {
				*(stream->_buf) = c;
				stream->_flags |= _IOERR;
				return EOF;
			}
		}
		*(stream->_buf) = c;
	}
	return c;
}
