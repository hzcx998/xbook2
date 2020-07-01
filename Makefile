# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

.PHONY: all $(MAKECMDGOALS) build debuild
all $(MAKECMDGOALS):
	@$(MAKE) -s -C ./src $(MAKECMDGOALS)

# 构建环境。libary，service，user。这样，可以很方便得把内核开发和
# 其它的开发分离
build: 
	cp develop/image/raw.img develop/image/c.img
	cd libary && make
	cd service && make
	cd user && make
	
# 清理环境。libary，service，user。这样，可以很方便得把内核开发和
# 其它的开发分离
debuild: 
	cd libary && make clean
	cd service && make clean
	cd user && make clean
	