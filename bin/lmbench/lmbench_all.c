#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int busy_main(int, char**);
int bw_file_rd_main(int, char**);
int bw_mem_main(int, char**);
int bw_mmap_rd_main(int, char**);
int bw_pipe_main(int, char**);
int bw_tcp_main(int, char**);
int bw_unix_main(int, char**);
int cache_main(int, char**);
int clock_main(int, char**);
int disk_main(int, char**);
int enough_main(int, char**);
int flushdisk_main(int, char**);
int getopt_main(int, char**);
int hello_main(int, char**);
int lat_cmd_main(int, char**);
int lat_connect_main(int, char**);
int lat_ctx_main(int, char**);
int lat_dram_page_main(int, char**);
int lat_fcntl_main(int, char**);
int lat_fifo_main(int, char**);
int lat_fs_main(int, char**);
int lat_http_main(int, char**);
int lat_mem_rd_main(int, char**);
int lat_mmap_main(int, char**);
int lat_ops_main(int, char**);
int lat_pagefault_main(int, char**);
int lat_pipe_main(int, char**);
int lat_pmake_main(int, char**);
int lat_proc_main(int, char**);
int lat_rand_main(int, char**);
int lat_rpc_main(int, char**);
int lat_select_main(int, char**);
int lat_sem_main(int, char**);
int lat_sig_main(int, char**);
int lat_syscall_main(int, char**);
int lat_tcp_main(int, char**);
int lat_udp_main(int, char**);
int lat_unix_main(int, char**);
int lat_unix_connect_main(int, char**);
int lat_usleep_main(int, char**);
int line_main(int, char**);
int lmdd_main(int, char**);
int lmhttp_main(int, char**);
int loop_o_main(int, char**);
int memsize_main(int, char**);
int mhz_main(int, char**);
int msleep_main(int, char**);
int par_mem_main(int, char**);
int par_ops_main(int, char**);
int rhttp_main(int, char**);
int seek_main(int, char**);
int stream_main(int, char**);
int timing_o_main(int, char**);
int tlb_main(int, char**);

struct FuncStru{
	const char *name;
	int (*func)(int, char **);
} main_table[] = {
	// {"busy", busy_main}, 
	{"bw_file_rd", bw_file_rd_main}, 
	{"bw_mem", bw_mem_main}, 
	{"bw_mmap_rd", bw_mmap_rd_main}, 
	{"bw_pipe", bw_pipe_main}, 
	/*{"bw_tcp", bw_tcp_main},*/ 
	/*{"bw_unix", bw_unix_main},*/ 
	{"cache", cache_main}, 
	// {"clock", clock_main}, 
	{"disk", disk_main}, 
	{"enough", enough_main}, 
	// {"flushdisk", flushdisk_main}, 
	// {"getopt", getopt_main}, 
	{"hello", hello_main}, 
	{"lat_cmd", lat_cmd_main}, 
	/*{"lat_connect", lat_connect_main},*/ 
	{"lat_ctx", lat_ctx_main}, 
	{"lat_dram_page", lat_dram_page_main}, 
	{"lat_fcntl", lat_fcntl_main}, 
	/*{"lat_fifo", lat_fifo_main},*/ 
	{"lat_fs", lat_fs_main}, 
	/*{"lat_http", lat_http_main},*/ 
	{"lat_mem_rd", lat_mem_rd_main}, 
	{"lat_mmap", lat_mmap_main}, 
	{"lat_ops", lat_ops_main}, 
	{"lat_pagefault", lat_pagefault_main}, 
	{"lat_pipe", lat_pipe_main}, 
	{"lat_pmake", lat_pmake_main}, 
	{"lat_proc", lat_proc_main}, 
	{"lat_rand", lat_rand_main}, 
	/*{"lat_rpc", lat_rpc_main},*/ 
	/*{"lat_select", lat_select_main},*/ 
	/*{"lat_sem", lat_sem_main}, */
	{"lat_sig", lat_sig_main}, 
	{"lat_syscall", lat_syscall_main}, 
	/*{"lat_tcp", lat_tcp_main}, */
	/*{"lat_udp", lat_udp_main},*/ 
	/*{"lat_unix", lat_unix_main},*/ 
	{"lat_unix_connect", lat_unix_connect_main}, 
	{"lat_usleep", lat_usleep_main}, 
	{"line", line_main}, 
	{"lmdd", lmdd_main}, 
	/*{"lmhttp", lmhttp_main},*/ 
	{"loop_o", loop_o_main}, 
	{"memsize", memsize_main}, 
	{"mhz", mhz_main}, 
	{"msleep", msleep_main}, 
	{"par_mem", par_mem_main}, 
	{"par_ops", par_ops_main}, 
	// {"rhttp", rhttp_main}, 
	// {"seek", seek_main}, 
	{"stream", stream_main}, 
	{"timing_o", timing_o_main}, 
	{"tlb", tlb_main},
	{NULL, NULL}
};

int main(int ar, char **av) {
	if (ar < 2) {
		printf("must have at least one argument\n");
		return 1;
	}
	struct FuncStru* index = main_table;
	while(index->name != NULL) {
		if (strcmp(index->name, av[1]) == 0) {
			return index->func(ar - 1, &av[1]);
		}
		index++;
	}
	printf("no match func %s\n", av[1]);
	return 2;
}
