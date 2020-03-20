# MIT License
# Copyright (c) 2020 Jason Hu, Zhu Yu

.PHONY: all $(MAKECMDGOALS)
all $(MAKECMDGOALS):
	@$(MAKE) -s -C ./src $(MAKECMDGOALS)
