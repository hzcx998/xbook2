# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu
all:

TOP_CMD	:= build debuild rom
PHONY	:= all $(MAKECMDGOALS) $(TOP_CMD)

all $(filter-out $(TOP_CMD),$(MAKECMDGOALS)):
	@$(MAKE) -s -C ./src $(MAKECMDGOALS)

# 构建应用开发环境
build: 
	cp develop/image/raw.img develop/image/c.img
	$(MAKE) -s -C libary
	$(MAKE) -s -C service
	$(MAKE) -s -C user

rom:
	tools/fatfs/fatfs develop/image/d.img develop/rom/ 10

# 清理应用开发环境
debuild: 
	$(MAKE) -s -C libary clean
	$(MAKE) -s -C service clean
	$(MAKE) -s -C user clean

.PHONY: $(PHONY)