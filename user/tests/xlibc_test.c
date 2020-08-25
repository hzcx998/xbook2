#include"test.h"

int xlibc_test(int argc,char *argv[])
{
	puts("-----------xlibc_test------------");

	puts("time");
	printf("%lu\n",time(NULL));

	puts("stdlib");
	srand(time(NULL));
	for(int count=0;count<10;count++)
	{
		printf("%d ",rand());
	}
	putchar('\n');

	for(int count=0;count<10;count++)
	{
		int num=rand()+rand();
		printf("%d:%d ",count,abs(num));
	}
	putchar('\n');

	for(int count=0;count<10;count++)
	{
		/* Get a 32-bit integer */
		long int num=((rand()+rand())<<16)+rand()+rand();
		printf("%ld:%ld ",num,labs(num));
	}
	putchar('\n');
	
	return 0;
}
