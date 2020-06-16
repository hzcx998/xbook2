# 文件路径
BOSH = ./bosh
LOGIN = ./login
TEST = ./test
INFONES = ./infones
TERMINAL = ./terminal

.PHONY: all clean

all: 
	cd $(LOGIN) && make
	cd $(BOSH) && make
	cd $(TEST) && make
	cd $(INFONES) && make
	cd $(TERMINAL) && make
	
clean:
	cd $(LOGIN) && make clean
	cd $(BOSH) && make clean
	cd $(TEST) && make clean
	cd $(INFONES) && make clean
	cd $(TERMINAL) && make clean
	