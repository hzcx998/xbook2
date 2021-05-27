#include "test.h"

//#define MNTPOINT "./mnt"
#if 0
static char mntpoint[64] = "./mnt";
static char device[64] = "/dev/vda2";
static const char *fs_type = "vfat";
#else
static char mntpoint[64] = "./mnt";
static char device[64] = "/dev/sda";
static const char *fs_type = "vfat";
#endif
void test_mount() {
	TEST_START(__func__);

	printf("Mounting dev:%s to %s\n", device, mntpoint);
	int ret = mount(device, mntpoint, fs_type, 0, NULL);
	printf("mount return: %d\n", ret);
	assert(ret == 0);

	if (ret == 0) {
		printf("mount successfully\n");
		ret = umount(mntpoint);
		printf("umount return: %d\n", ret);
	}

	TEST_END(__func__);
}

int mount_test(int argc, char* argv[])
{
    if(argc >= 2){
		strcpy(device, argv[1]);
	}

	if(argc >= 3){
		strcpy(mntpoint, argv[2]);
	}

	test_mount();
    return 0;
}
