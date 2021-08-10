#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H

void	print_results(int details);
void	bw_quartile(uint64 bytes);
void	nano_quartile(uint64 n);
void	print_mem(char* addr, size_t size, size_t line);
void	check_mem(char* addr, size_t size);

#endif /* _LIB_DEBUG_H */
