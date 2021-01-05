-include $(TOPDIR)/Make.defs
ROOTDEPPATH = --dep-path .

MAINSRC = hamstrone_main.c
CSRCS = hamster_tongue.c serialize.c task.c value_store.c
COBJS = hamster_tongue.o serialize.o task.o value_store.o



# Build targets
all: libapps.a
.PHONY: dirlinks context preconfig depend clean clean_context distclean
.PRECIOUS: libapps$(LIBEXT)
 
# Compile C Files
$(COBJS): %$(OBJEXT): %.c
	$(call COMPILE, $<, $@)

# Add object files to the apps archive
libapps.a: $(COBJS)
	$(call ARCHIVE, libapps.a, $(COBJS))
 
# Create directory links
dirlinks:
 
# Setup any special pre-build context
context:
 
# Setup any special pre-configuration context
preconfig:
 
# Make the dependency file, Make.deps
depend: Makefile $(CSRCS)
	$(Q) $(MKDEP) $(ROOTDEPPATH) "$(CC)" -- $(CFLAGS) -- $(SRCS) >Make.dep
 
# Clean the results of the last build
clean:
	$(call CLEAN)
 
# Remove the build context and directory links
clean_context:
 
# Restore the directory to its original state
distclean: clean clean_context
	$(call DELFILE, Make.dep)
 
# Include dependencies
-include Make.dep

PROGNAME  = hamstrone
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = CONFIG_DEFAULT_TASK_STACKSIZE
MODULE    = $(CONFIG_HAMSTRONE)