volatile int i;

busy_main()
{

	//nice(10);
	for (;;) getppid();
	//for (;;) i++;
	exit(i);
}
