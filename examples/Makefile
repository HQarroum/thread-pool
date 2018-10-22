DIRS       = $(wildcard */.)
BUILDDIRS  = $(DIRS:%=build-%)
RUNDIRS    = $(DIRS:%=run-%)
TESTDIRS   = $(DIRS:%=test-%)
CLEANDIRS  = $(DIRS:%=clean-%)
FCLEANDIRS = $(DIRS:%=fclean-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(MAKE) -C $(@:build-%=%)

run: $(RUNDIRS)
$(RUNDIRS):
	$(MAKE) -C $(@:run-%=%) run

test: $(TESTDIRS)
$(TESTDIRS):
	$(MAKE) -C $(@:test-%=%) test

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

fclean: $(FCLEANDIRS)
$(FCLEANDIRS):
	$(MAKE) -C $(@:fclean-%=%) fclean

re: fclean all

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(RUNDIRS)
.PHONY: subdirs $(TESTDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: subdirs $(FCLEANDIRS)
.PHONY: all fclean run
