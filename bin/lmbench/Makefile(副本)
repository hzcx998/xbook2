# $Id$

# Make targets:
#
# lmbench	[default] builds the benchmark suite for the current os/arch
# results	builds, configures run parameters, and runs the benchmark
# rerun		reruns the benchmark using the same parameters as last time
# scaling	reruns the benchmark using same parameters as last time,
#		except it asks what scaling value to use
# hardware	reruns the hardware benchmarks using the same parameters
# os		reruns the OS benchmarks using the same parameters
# clean		cleans out sources and run configuration
# clobber	clean and removes the bin directories
# shar		obsolete, use cd .. && make shar
# depend	builds make dependencies (needs gcc)
# debug		builds all the benchmarks with '-g' debugging flag
# assembler	builds the .s files for each benchmark
#
# This is largely self configuring.  Most stuff is pretty portable.  
#
# If you don't have gcc, try make CC=cc and see if that works.
#
# If you want to do cross-compilation try make OS=armv5tel-linux-gnu
# or whatever your OS string should be in the target environment.
# Since many embedded development environments also have a special
# cross-compiler, you might want to also select a particular compiler,
# so your build command would look something like:
#	make OS=armv5tel-linux-gnu CC=gcc-arm
#
# Overriding the OS and CC make parameters needs to be done as an
# argument to make, not as an environment variable.  See above comments.
#

# I finally know why Larry Wall's Makefile says "Grrrr".
SHELL=/bin/sh

CC=`../scripts/compiler`
MAKE=`../scripts/make`
AR=ar
ARCREATE=cr

# base of installation location
BASE=/usr/local
O= ../bin/unknown
D= ../doc
TRUE=/bin/true
OS=`../scripts/os`
TARGET=`../scripts/target`
BINDIR=../bin/$(OS)
CONFIG=../bin/$(OS)/`../scripts/config`
UTILS=../scripts/target ../scripts/os ../scripts/gnu-os ../scripts/compiler \
	../scripts/info ../scripts/info-template ../scripts/version \
	../scripts/config ../scripts/config-run ../scripts/results \
	../scripts/lmbench ../scripts/make ../scripts/build
INSTALL=cp
RESULTS=Results/$(OS)
SAMPLES=lmbench/Results/aix/rs6000 lmbench/Results/hpux/snake \
	lmbench/Results/irix/indigo2 lmbench/Results/linux/pentium \
	lmbench/Results/osf1/alpha lmbench/Results/solaris/ss20* 

COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS)

INCS =	bench.h lib_mem.h lib_tcp.h lib_udp.h stats.h timing.h

SRCS =  bw_file_rd.c bw_mem.c bw_mmap_rd.c bw_pipe.c bw_tcp.c bw_udp.c	\
	bw_unix.c							\
	cache.c clock.c disk.c enough.c flushdisk.c getopt.c hello.c	\
	lat_connect.c lat_ctx.c	lat_fcntl.c lat_fifo.c lat_fs.c 	\
	lat_mem_rd.c lat_mmap.c lat_ops.c lat_pagefault.c lat_pipe.c 	\
	lat_proc.c lat_rpc.c lat_select.c lat_sig.c lat_syscall.c	\
	lat_tcp.c lat_udp.c lat_unix.c lat_unix_connect.c lat_sem.c	\
	lat_usleep.c lat_pmake.c  					\
	lib_debug.c lib_mem.c lib_stats.c lib_tcp.c lib_timing.c 	\
	lib_udp.c lib_unix.c lib_sched.c				\
	line.c lmdd.c lmhttp.c par_mem.c par_ops.c loop_o.c memsize.c 	\
	mhz.c msleep.c rhttp.c seek.c timing_o.c tlb.c stream.c		\
	bench.h lib_debug.h lib_tcp.h lib_udp.h lib_unix.h names.h 	\
	stats.h timing.h version.h

ASMS =  $O/bw_file_rd.s $O/bw_mem.s $O/bw_mmap_rd.s $O/bw_pipe.s 	\
	$O/bw_tcp.s $O/bw_udp.s $O/bw_unix.s $O/clock.s			\
	$O/disk.s $O/enough.s $O/flushdisk.s $O/getopt.s $O/hello.s	\
	$O/lat_connect.s $O/lat_ctx.s lat_fcntl.s $O/lat_fifo.s		\
	$O/lat_fs.s $O/lat_mem_rd.s $O/lat_mmap.s $O/lat_ops.s		\
	$O/lat_pagefault.s $O/lat_pipe.s $O/lat_proc.s $O/lat_rpc.s	\
	$O/lat_select.s $O/lat_sig.s $O/lat_syscall.s $O/lat_tcp.s	\
	$O/lat_udp.s $O/lat_unix.s $O/lat_unix_connect.s $O/lat_sem.s	\
	$O/lib_debug.s $O/lib_mem.s	\
	$O/lib_stats.s $O/lib_tcp.s $O/lib_timing.s $O/lib_udp.s	\
	$O/lib_unix.s $O/lib_sched.s					\
	$O/line.s $O/lmdd.s $O/lmhttp.s $O/par_mem.s	\
	$O/par_ops.s $O/loop_o.s $O/memsize.s $O/mhz.s $O/msleep.s	\
	$O/rhttp.s $O/timing_o.s $O/tlb.s $O/stream.s			\
	$O/cache.s $O/lat_dram_page.s $O/lat_pmake.s $O/lat_rand.s	\
	$O/lat_usleep.s $O/lat_cmd.s
EXES    = $O/lmbench_all
EXEOBJ =	$O/bw_file_rd $O/bw_mem $O/bw_mmap_rd $O/bw_pipe $O/bw_tcp 	\
	$O/bw_unix $O/hello						\
	$O/lat_select $O/lat_pipe $O/lat_rpc $O/lat_syscall $O/lat_tcp	\
	$O/lat_udp $O/lat_mmap $O/mhz $O/lat_proc $O/lat_pagefault	\
	$O/lat_connect $O/lat_fs $O/lat_sig $O/lat_mem_rd $O/lat_ctx	\
	$O/lat_sem 							\
	$O/memsize $O/lat_unix $O/lmdd $O/timing_o $O/enough		\
	$O/msleep $O/loop_o $O/lat_fifo $O/lmhttp $O/lat_http		\
	$O/lat_fcntl $O/disk $O/lat_unix_connect $O/flushdisk		\
	$O/lat_ops $O/line $O/tlb $O/par_mem $O/par_ops 		\
	$O/stream
EXEOBJS = $(addsuffix .o, $(EXEOBJ))
O_OPT_EXES=$O/cache $O/lat_dram_page $O/lat_pmake $O/lat_rand 		\
	$O/lat_usleep $O/lat_cmd
OPT_EXES = 
EXEOBJS += $(addsuffix .o, $(O_OPT_EXES))
LIBOBJS= $O/lib_tcp.o $O/lib_udp.o $O/lib_unix.o $O/lib_timing.o 	\
	$O/lib_mem.o $O/lib_stats.o $O/lib_debug.o $O/getopt.o		\
	$O/lib_sched.o


lmbench: $(UTILS)
	@env CFLAGS=-O MAKE="$(MAKE)" MAKEFLAGS="$(MAKEFLAGS)" CC="$(CC)" OS="$(OS)" ../scripts/build all
	-@env CFLAGS=-O MAKE="$(MAKE)" MAKEFLAGS="-k $(MAKEFLAGS)" CC="$(CC)" OS="$(OS)" ../scripts/build opt

oscomp: lmbench
	env OS="${OS}" ../scripts/oscomp.sh

results: lmbench
	@env OS="${OS}" ../scripts/config-run
	@env OS="${OS}" ../scripts/results

rerun: lmbench
	@if [ ! -f $(CONFIG) ]; then env OS="${OS}" ../scripts/config-run; fi
	@env OS="${OS}" ../scripts/results

scaling: lmbench
	@if [ ! -f $(CONFIG) ]; then env OS="${OS}" ../scripts/config-run; \
	 else ../scripts/config-scaling $(CONFIG); fi
	@env OS="${OS}" ../scripts/results

hardware: lmbench
	@if [ ! -f $(CONFIG) ]; then env OS="${OS}" ../scripts/config-run; fi
	@env OS="${OS}" BENCHMARK_HARDWARE=YES BENCHMARK_OS=NO  ../scripts/results

os: lmbench
	@if [ ! -f $(CONFIG) ]; then env OS="${OS}" ../scripts/config-run; fi
	@env OS="${OS}" BENCHMARK_HARDWARE=NO  BENCHMARK_OS=YES ../scripts/results

install: lmbench
	@env CFLAGS=-O MAKE="$(MAKE)"  MAKEFLAGS="$(MAKEFLAGS)" CC="${CC}" OS="${OS}" ../scripts/build install-target

install-target:
	if [ ! -d $(BASE) ]; then mkdir $(BASE); fi
	if [ ! -d $(BASE)/bin ]; then mkdir $(BASE)/bin; fi
	if [ ! -d $(BASE)/include ]; then mkdir $(BASE)/include; fi
	if [ ! -d $(BASE)/lib ]; then mkdir $(BASE)/lib; fi
	cp $(EXES) $(BASE)/bin
	cp $(INCS) $(BASE)/include
	cp $O/lmbench.a $(BASE)/lib/libmbench.a
	cd ../doc; env MAKEFLAGS="$(MAKEFLAGS)" make CC="${CC}" OS="${OS}" BASE="$(BASE)" install


# No special handling for all these
all: $(EXES) $O/lmbench
opt: $(OPT_EXES)
asm: $(ASMS)
$(ASMS):
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ `basename $@ .s`.c

Wall:
	@env CFLAGS="-Wall -ansi" MAKE="$(MAKE)" MAKEFLAGS="$(MAKEFLAGS)" CC="${CC}" OS="${OS}" ../scripts/build all opt

debug:
	@env CFLAGS="-g -O" MAKE="$(MAKE)" MAKEFLAGS="$(MAKEFLAGS)" CC="${CC}" OS="${OS}" ../scripts/build all opt

assembler:
	@env CFLAGS=-O MAKE="$(MAKE)" MAKEFLAGS="$(MAKEFLAGS)" CC="${CC}" OS="${OS}" ../scripts/build asm

bk.ver: ../SCCS/s.ChangeSet
	rm -f bk.ver
	-echo `bk prs -hr+ -d'$$if(:SYMBOL:){:SYMBOL: }:UTC:' ../ChangeSet;` > bk.ver
	touch bk.ver

dist: bk.ver
	@if [ "X`cd ..; bk sfiles -c`" != "X" ]; then \
		echo "modified files!"; \
		false; \
	 fi
	@if [ "X`cd ..; bk pending`" != "X" ]; then \
		echo "pending changes!"; \
		false; \
	 fi
	cd ..; \
		SRCDIR=`pwd`; \
		DIR=`basename $${SRCDIR}`; \
		VERSION=`cat src/bk.ver| awk '{print $$1;}' | sed -e 's/Version-//g'`; \
		cd ..; \
		bk clone $${DIR} /tmp/lmbench-$${VERSION}; \
		cd /tmp/lmbench-$${VERSION}; \
		bk sfiles | xargs touch; \
		sleep 5; \
		bk get -s; \
		for d in doc results scripts src; do \
			cd $$d; bk get -s; cd ..; \
		done; \
		bk sfiles -U -g | xargs touch; \
		cd src; \
		make bk.ver; \
		cd /tmp; \
		tar czf $${SRCDIR}/../lmbench-$${VERSION}.tgz \
			lmbench-$${VERSION}; \
		rm -rf /tmp/lmbench-$${VERSION};

get $(SRCS):
	-get -s $(SRCS)

edit get-e:
	get -e -s $(SRCS)

clean:
	/bin/rm -f ../bin/*/CONFIG ../bin/*/*.[oas]
	/bin/rm -f *.[oas]
	-bk clean

clobber:
	/bin/rm -rf ../bin* SHAR
	-bk clean

shar:
	cd ../.. && shar lmbench/Results/Makefile $(SAMPLES) lmbench/scripts/* lmbench/src/Makefile lmbench/src/*.[ch] > lmbench/SHAR

depend: ../scripts/depend
	../scripts/depend

testmake: $(SRCS) $(UTILS) # used by scripts/make to test gmake
	@true

.PHONY: lmbench results rerun hardware os install all Wall debug \
	install install-target dist get edit get-e clean clobber \
	share depend testmake

$O/lmbench : ../scripts/lmbench bk.ver
	rm -f $O/lmbench
	sed -e "s/<version>/`cat bk.ver`/g" < ../scripts/lmbench > $O/lmbench
	chmod +x $O/lmbench

$O/lmbench.a: $(LIBOBJS)
	/bin/rm -f $O/lmbench.a
	$(AR) $(ARCREATE) $O/lmbench.a $(LIBOBJS)
	-ranlib $O/lmbench.a

$O/lib_timing.o : lib_timing.c $(INCS)
	$(COMPILE) -c lib_timing.c -o $O/lib_timing.o
$O/lib_mem.o : lib_mem.c $(INCS)
	$(COMPILE) -c lib_mem.c -o $O/lib_mem.o
$O/lib_tcp.o : lib_tcp.c $(INCS)
	$(COMPILE) -c lib_tcp.c -o $O/lib_tcp.o
$O/lib_udp.o : lib_udp.c $(INCS)
	$(COMPILE) -c lib_udp.c -o $O/lib_udp.o
$O/lib_unix.o : lib_unix.c $(INCS)
	$(COMPILE) -c lib_unix.c -o $O/lib_unix.o
$O/lib_debug.o : lib_debug.c $(INCS)
	$(COMPILE) -c lib_debug.c -o $O/lib_debug.o
$O/lib_stats.o : lib_stats.c $(INCS)
	$(COMPILE) -c lib_stats.c -o $O/lib_stats.o
$O/lib_sched.o : lib_sched.c $(INCS)
	$(COMPILE) -c lib_sched.c -o $O/lib_sched.o
$O/getopt.o : getopt.c $(INCS)
	$(COMPILE) -c getopt.c -o $O/getopt.o

$(UTILS) :
	-cd ../scripts; make get

$O/lmbench_all: $(EXEOBJS) $O/lmbench.a lmbench_all.c
	$(COMPILE) -o $O/lmbench_all $^ -lm
	cat lmexes.txt | xargs -n 1 -I {} ../scripts/mklink.sh {} $(abspath $O)


$(EXEOBJS): 
	$(COMPILE) -c -o $@ $(basename $(notdir $@)).c 

# Do not remove the next line, $(MAKE) depend needs it
# MAKEDEPEND follows
$O/rhttp.s:rhttp.c timing.h stats.h bench.h 
$O/rhttp:  rhttp.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/rhttp rhttp.c $O/lmbench.a $(LDLIBS)

$O/http.s:http.c timing.h stats.h bench.h 
$O/http:  http.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/http http.c $O/lmbench.a $(LDLIBS)

$O/flushdisk.s:flushdisk.c 
$O/flushdisk:  flushdisk.c 
	$(COMPILE) -DMAIN -o $O/flushdisk flushdisk.c

$O/mhz.s: mhz.c timing.h stats.h bench.h
$O/mhz: mhz.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/mhz mhz.c $O/lmbench.a $(LDLIBS) -lm

$O/lat_ctx.s: lat_ctx.c timing.h stats.h bench.h
$O/lat_ctx: lat_ctx.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_ctx lat_ctx.c $O/lmbench.a $(LDLIBS)

$O/lmhttp.s:lmhttp.c timing.h stats.h bench.h
$O/lmhttp:  lmhttp.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lmhttp lmhttp.c $O/lmbench.a $(LDLIBS)

$O/lat_http.s:lat_http.c timing.h stats.h bench.h
$O/lat_http:  lat_http.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_http lat_http.c $O/lmbench.a $(LDLIBS)

$O/bw_file_rd.s:bw_file_rd.c timing.h stats.h bench.h
$O/bw_file_rd:  bw_file_rd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/bw_file_rd bw_file_rd.c $O/lmbench.a $(LDLIBS)

$O/bw_mem.s:bw_mem.c timing.h stats.h bench.h
$O/bw_mem:  bw_mem.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/bw_mem bw_mem.c $O/lmbench.a $(LDLIBS)

$O/bw_mmap_rd.s:bw_mmap_rd.c timing.h stats.h bench.h
$O/bw_mmap_rd:  bw_mmap_rd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/bw_mmap_rd bw_mmap_rd.c $O/lmbench.a $(LDLIBS)

$O/bw_pipe.s:bw_pipe.c timing.h stats.h bench.h
$O/bw_pipe:  bw_pipe.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/bw_pipe bw_pipe.c $O/lmbench.a $(LDLIBS)

$O/bw_tcp.s:bw_tcp.c bench.h timing.h stats.h lib_tcp.h
$O/bw_tcp:  bw_tcp.c bench.h timing.h stats.h lib_tcp.h $O/lmbench.a
	$(COMPILE) -o $O/bw_tcp bw_tcp.c $O/lmbench.a $(LDLIBS)

$O/bw_udp.s:bw_udp.c bench.h timing.h stats.h lib_udp.h
$O/bw_udp:  bw_udp.c bench.h timing.h stats.h lib_udp.h $O/lmbench.a
	$(COMPILE) -o $O/bw_udp bw_udp.c $O/lmbench.a $(LDLIBS)

$O/bw_unix.s:bw_unix.c timing.h stats.h bench.h
$O/bw_unix:  bw_unix.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/bw_unix bw_unix.c $O/lmbench.a $(LDLIBS)

$O/disk.s:disk.c flushdisk.c bench.h timing.h stats.h lib_tcp.h
$O/disk:  disk.c flushdisk.c bench.h timing.h stats.h lib_tcp.h $O/lmbench.a
	$(COMPILE) -o $O/disk disk.c $O/lmbench.a $(LDLIBS)

$O/clock.s:clock.c timing.h stats.h bench.h
$O/clock:  clock.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/clock clock.c $O/lmbench.a $(LDLIBS)

$O/hello.s:hello.c
$O/hello:  hello.c $O/lmbench.a
	$(COMPILE) -o $O/hello hello.c $O/lmbench.a $(LDLIBS)

$O/lat_alarm.s:lat_alarm.c timing.h stats.h bench.h
$O/lat_alarm:  lat_alarm.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_alarm lat_alarm.c $O/lmbench.a $(LDLIBS)

$O/lat_connect.s:lat_connect.c lib_tcp.c bench.h lib_tcp.h timing.h stats.h
$O/lat_connect:  lat_connect.c lib_tcp.c bench.h lib_tcp.h timing.h stats.h $O/lmbench.a
	$(COMPILE) -o $O/lat_connect lat_connect.c $O/lmbench.a $(LDLIBS)

$O/lat_unix_connect.s:lat_unix_connect.c lib_tcp.c bench.h lib_tcp.h timing.h stats.h
$O/lat_unix_connect:  lat_unix_connect.c lib_tcp.c bench.h lib_tcp.h timing.h stats.h $O/lmbench.a
	$(COMPILE) -o $O/lat_unix_connect lat_unix_connect.c $O/lmbench.a $(LDLIBS)

$O/lat_fs.s:lat_fs.c timing.h stats.h bench.h
$O/lat_fs:  lat_fs.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_fs lat_fs.c $O/lmbench.a $(LDLIBS)

$O/lat_fcntl.s:lat_fcntl.c timing.h stats.h bench.h
$O/lat_fcntl:  lat_fcntl.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_fcntl lat_fcntl.c $O/lmbench.a $(LDLIBS)

$O/lat_mem_rd.s:lat_mem_rd.c timing.h stats.h bench.h
$O/lat_mem_rd:  lat_mem_rd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mem_rd lat_mem_rd.c $O/lmbench.a $(LDLIBS)

$O/lat_mem_rd2.s:lat_mem_rd2.c timing.h stats.h bench.h
$O/lat_mem_rd2:  lat_mem_rd2.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mem_rd2 lat_mem_rd2.c $O/lmbench.a $(LDLIBS)

$O/lat_mem_wr.s:lat_mem_wr.c timing.h stats.h bench.h
$O/lat_mem_wr:  lat_mem_wr.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mem_wr lat_mem_wr.c $O/lmbench.a $(LDLIBS)

$O/lat_mem_wr2.s:lat_mem_wr2.c timing.h stats.h bench.h
$O/lat_mem_wr2:  lat_mem_wr2.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mem_wr2 lat_mem_wr2.c $O/lmbench.a $(LDLIBS)

$O/lat_mmap.s:lat_mmap.c timing.h stats.h bench.h
$O/lat_mmap:  lat_mmap.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mmap lat_mmap.c $O/lmbench.a $(LDLIBS)

$O/lat_mmaprd.s:lat_mmaprd.c timing.h stats.h bench.h
$O/lat_mmaprd:  lat_mmaprd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_mmaprd lat_mmaprd.c $O/lmbench.a $(LDLIBS)

$O/lat_ops.s:lat_ops.c timing.h stats.h bench.h
$O/lat_ops:  lat_ops.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_ops lat_ops.c $O/lmbench.a $(LDLIBS)

$O/lat_pagefault.s:lat_pagefault.c timing.h stats.h bench.h
$O/lat_pagefault:  lat_pagefault.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_pagefault lat_pagefault.c $O/lmbench.a $(LDLIBS)

$O/lat_pipe.s:lat_pipe.c timing.h stats.h bench.h
$O/lat_pipe:  lat_pipe.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_pipe lat_pipe.c $O/lmbench.a $(LDLIBS)

$O/lat_fifo.s:lat_fifo.c timing.h stats.h bench.h
$O/lat_fifo:  lat_fifo.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_fifo lat_fifo.c $O/lmbench.a $(LDLIBS)

$O/lat_proc.s:lat_proc.c timing.h stats.h bench.h
$O/lat_proc:  lat_proc.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_proc lat_proc.c $O/lmbench.a $(LDLIBS)

$O/lat_rpc.s:lat_rpc.c timing.h stats.h bench.h
$O/lat_rpc:  lat_rpc.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_rpc lat_rpc.c $O/lmbench.a $(LDLIBS)

$O/lat_sig.s:lat_sig.c timing.h stats.h bench.h
$O/lat_sig:  lat_sig.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_sig lat_sig.c $O/lmbench.a $(LDLIBS)

$O/lat_syscall.s:lat_syscall.c timing.h stats.h bench.h
$O/lat_syscall:  lat_syscall.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_syscall lat_syscall.c $O/lmbench.a $(LDLIBS)

$O/lat_select.s:  lat_select.c timing.h stats.h bench.h
$O/lat_select:  lat_select.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_select lat_select.c $O/lmbench.a $(LDLIBS)

$O/lat_tcp.s:lat_tcp.c timing.h stats.h bench.h lib_tcp.h
$O/lat_tcp:  lat_tcp.c timing.h stats.h bench.h lib_tcp.h $O/lmbench.a
	$(COMPILE) -o $O/lat_tcp lat_tcp.c $O/lmbench.a $(LDLIBS)

$O/lat_udp.s:lat_udp.c timing.h stats.h bench.h lib_udp.h
$O/lat_udp:  lat_udp.c timing.h stats.h bench.h lib_udp.h $O/lmbench.a
	$(COMPILE) -o $O/lat_udp lat_udp.c $O/lmbench.a $(LDLIBS)

$O/lat_unix.s:lat_unix.c timing.h stats.h bench.h
$O/lat_unix:  lat_unix.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_unix lat_unix.c $O/lmbench.a $(LDLIBS)

$O/lib_tcp.s:lib_tcp.c bench.h lib_tcp.h
$O/lib_tcp:  lib_tcp.c bench.h lib_tcp.h $O/lmbench.a
	$(COMPILE) -o $O/lib_tcp lib_tcp.c $O/lmbench.a $(LDLIBS)

$O/lib_udp.s:lib_udp.c bench.h lib_udp.h
$O/lib_udp:  lib_udp.c bench.h lib_udp.h $O/lmbench.a
	$(COMPILE) -o $O/lib_udp lib_udp.c $O/lmbench.a $(LDLIBS)

$O/lmdd.s:lmdd.c timing.h stats.h bench.h
$O/lmdd:  lmdd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lmdd lmdd.c $O/lmbench.a $(LDLIBS)

$O/enough.s:enough.c timing.h stats.h bench.h
$O/enough:  enough.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/enough enough.c $O/lmbench.a $(LDLIBS)

$O/loop_o.s:loop_o.c timing.h stats.h bench.h
$O/loop_o:  loop_o.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/loop_o loop_o.c $O/lmbench.a $(LDLIBS)

$O/timing_o.s:timing_o.c timing.h stats.h bench.h
$O/timing_o:  timing_o.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/timing_o timing_o.c $O/lmbench.a $(LDLIBS)

$O/memsize.s:memsize.c timing.h stats.h bench.h
$O/memsize:  memsize.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/memsize memsize.c $O/lmbench.a $(LDLIBS)

$O/msleep.s:msleep.c timing.h stats.h bench.h
$O/msleep:  msleep.c timing.h stats.h bench.h 
	$(COMPILE) -o $O/msleep msleep.c

$O/line.s:  line.c timing.h stats.h bench.h
$O/line:  line.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/line line.c $O/lmbench.a $(LDLIBS)

$O/tlb.s:tlb.c timing.h stats.h bench.h
$O/tlb:  tlb.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/tlb tlb.c $O/lmbench.a $(LDLIBS)

$O/cache.s:cache.c timing.h stats.h bench.h
$O/cache:  cache.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/cache cache.c $O/lmbench.a $(LDLIBS)

$O/par_mem.s:par_mem.c timing.h stats.h bench.h
$O/par_mem:  par_mem.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/par_mem par_mem.c $O/lmbench.a $(LDLIBS)

$O/par_ops.s:par_ops.c timing.h stats.h bench.h
$O/par_ops:  par_ops.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/par_ops par_ops.c $O/lmbench.a $(LDLIBS)

$O/stream.s:stream.c timing.h stats.h bench.h
$O/stream:  stream.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/stream stream.c $O/lmbench.a $(LDLIBS)

$O/lat_sem.s:lat_sem.c timing.h stats.h bench.h
$O/lat_sem:  lat_sem.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_sem lat_sem.c $O/lmbench.a $(LDLIBS)

$O/par_list.s:par_list.c timing.h stats.h bench.h
$O/par_list:  par_list.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/par_list par_list.c $O/lmbench.a $(LDLIBS)

$O/lat_dram_page.s:lat_dram_page.c timing.h stats.h bench.h
$O/lat_dram_page:  lat_dram_page.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_dram_page lat_dram_page.c $O/lmbench.a $(LDLIBS)

$O/lat_usleep.s:lat_usleep.c timing.h stats.h bench.h
$O/lat_usleep:  lat_usleep.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_usleep lat_usleep.c $O/lmbench.a $(LDLIBS)

$O/lat_pmake.s:lat_pmake.c timing.h stats.h bench.h
$O/lat_pmake:  lat_pmake.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_pmake lat_pmake.c $O/lmbench.a $(LDLIBS)

$O/lat_rand.s:lat_rand.c timing.h stats.h bench.h
$O/lat_rand:  lat_rand.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_rand lat_rand.c $O/lmbench.a $(LDLIBS)

$O/lat_cmd.s:lat_cmd.c timing.h stats.h bench.h
$O/lat_cmd:  lat_cmd.c timing.h stats.h bench.h $O/lmbench.a
	$(COMPILE) -o $O/lat_cmd lat_cmd.c $O/lmbench.a $(LDLIBS)

