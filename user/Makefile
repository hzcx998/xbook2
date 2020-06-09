# 文件路径
BOSH = ./bosh
LOGIN = ./login
TEST = ./test

.PHONY: all clean

all: 
	cd $(LOGIN) && make
	cd $(BOSH) && make
	cd $(TEST) && make
	
clean:
	cd $(LOGIN) && make clean
	cd $(BOSH) && make clean
	cd $(TEST) && make clean
	