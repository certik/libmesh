# The location of the mesh library
LIBMESH_DIR ?= ../../..

include ../../../Make.common

libmesh_INCLUDE += -I. -I../../metis/GKlib

SRC     := $(wildcard *.c)
OBJS   	:= $(patsubst %.c, %.$(obj-suffix), $(SRC))

.PHONY: clean clobber distclean



target := $(EXTERNAL_LIBDIR)/libparmetis$(static_libext)

ifeq ($(enable-shared),yes)
  target := $(EXTERNAL_LIBDIR)/libparmetis$(shared_libext)
endif


ifeq ($(enable-parmetis),no)
  all::
	@echo "<<< parmetis is disabled, not building >>>"
else
    ifeq (x$(MPI_IMPL),x)
      all::
	@echo "<<< no MPI found, not building parmetis >>>"
    else
      all:: $(target)
    endif
endif

ifeq ($(findstring darwin,$(hostos)),darwin)
$(EXTERNAL_LIBDIR)/libparmetis$(static_libext): $(OBJS)
	@echo "Linking "$@
	@$(shell mkdir -p $(EXTERNAL_LIBDIR))
	@libtool -static -o $@ $(OBJS)
else
$(EXTERNAL_LIBDIR)/libparmetis$(static_libext): $(OBJS)
	@echo "Linking "$@
	@$(shell mkdir -p $(EXTERNAL_LIBDIR))
	@$(AR) rv $@ $(OBJS)
endif

$(EXTERNAL_LIBDIR)/libparmetis$(shared_libext): $(OBJS)
	@echo "Linking "$@
	@$(shell mkdir -p $(EXTERNAL_LIBDIR))
	@$(libmesh_CC) $(libmesh_CSHAREDFLAG) -o $@ $(OBJS) $(libmesh_LDFLAGS)

clean:
	@rm -f $(OBJS) *~ .depend

clobber:
	@$(MAKE) clean
	@rm -f *.o *.g.o *.pg.o *.sy.o
	@rm -f $(EXTERNAL_LIBDIR)/libparmetis.*

distclean:
	@$(MAKE) clobber
	@rm -f ../../lib/*/libparmetis.*


# include the dependency list
include .depend


#
# Dependencies
#
.depend: $(SRC) *.h
	@$(perl) $(LIBMESH_DIR)/contrib/bin/make_dependencies.pl -I.  "-S\$$(obj-suffix)" $(SRC) > .depend

###############################################################################
