# The location of the mesh library
LIBMESH_DIR ?= ../../..

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
include $(LIBMESH_DIR)/Make.common


###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= $(wildcard *.C) ../../transient/transient_ex1/exact_solution.C

#
# object files
objects		:= $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
###############################################################################



.PHONY: clean clobber distclean

###############################################################################
# Target:
#
target 	      := ./adaptivity_ex2-$(METHOD)

# Specify the number of timesteps to do when we call "make run"
n_timesteps   := 25

# Specify the amount of initial uniform refinement to do
n_refinements := 5

# Specify how often to write output files
output_freq   := 10

all:: $(target)

# Production rules:  how to make the target - depends on library configuration
$(target): $(objects)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)

# Useful rules.
clean:
	@rm -f $(objects) *~ .depend

clobber:
	@$(MAKE) clean
	@rm -f $(target) out_*.gmv

distclean:
	@$(MAKE) clobber
	@rm -f *.o *.g.o *.pg.o .depend


run: $(target)
	@echo "***************************************************************"
	@echo "* Running Example " $(LIBMESH_RUN) $(target) \[-read_solution\] -n_timesteps $(n_timesteps) -n_refinements $(n_refinements) -init_timestep \[0\|$(n_timesteps)\] $(LIBMESH_OPTIONS)
	@echo "***************************************************************"
	@echo " "
	@$(LIBMESH_RUN) $(target) -n_timesteps $(n_timesteps) -n_refinements $(n_refinements) \
                   -output_freq $(output_freq) -init_timestep 0 $(LIBMESH_OPTIONS)
	@echo " "
	@echo "***** Finished first" $(n_timesteps) "steps, now read in" \
	       "saved solution and continue *****"
	@echo " "
	@$(LIBMESH_RUN) $(target) -read_solution -n_timesteps $(n_timesteps) \
                   -output_freq $(output_freq) -init_timestep $(n_timesteps) $(LIBMESH_OPTIONS)
	@echo " "
	@echo "***************************************************************"
	@echo "* Done Running Example " $(LIBMESH_RUN) $(target) \[-read_solution\] -n_timesteps $(n_timesteps) -init_timestep \[0\|$(n_timesteps)\] $(LIBMESH_OPTIONS)
	@echo "***************************************************************"


restart: $(target)
	@echo "***************************************************************"
	@echo "* Restarting Example " $(target)
	@echo "***************************************************************"
	@echo " "
	@echo "***** Read in saved solution and continue *****"
	@echo " "
	@$(LIBMESH_RUN) $(target) -read_solution -n_timesteps $(n_timesteps) \
                   -output_freq $(output_freq) -init_timestep $(n_timesteps) $(LIBMESH_OPTIONS)
	@echo " "
	@echo "***************************************************************"
	@echo "* Done Restarting Example " $(target)
	@echo "***************************************************************"


# include the dependency list
include .depend


#
# Dependencies
#
.depend: $(srcfiles) $(LIBMESH_DIR)/include/*/*.h
	@$(perl) $(LIBMESH_DIR)/contrib/bin/make_dependencies.pl -I. $(foreach i, $(wildcard $(LIBMESH_DIR)/include/*), -I$(i)) "-S\$$(obj-suffix)" $(srcfiles) > .depend

###############################################################################
