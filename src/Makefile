SHELL=/bin/sh

ifndef PREFIX
  PREFIX=../..
endif

# EUPS compliant definition
LIBIMSUPPORT = ../lib/libimsupport.a

OBJECTS=imarithsubs.o archivesubs.o catsubs.o db_utils.o eventsubs.o imreadsubs.o lut_utils.o mask_utils.o subs_fits.o subs_memory.o subs_inoutput.o wcssubs.o readstdstable.o ImageMorphology.o LegacyInterface.o ComLine.o primitive_utilities.o System.o MorphologyCX.o argutils.o

STANDALONEOBJECTS=imarithsubs.o archivesubs.o catsubs.o eventsubs.o imreadsubs.o subs_fits.o subs_memory.o subs_inoutput.o wcssubs.o readstdstable.o ImageMorphology.o LegacyInterface.o ComLine.o primitive_utilities.o System.o MorphologyCX.o argutils.o Profiler.o

CFLAGS = -I../include -I$(CFITSIO_SHARED_DIR)/include/ -I${SWARP_DIR}/include/

CPPC := 	g++
CPPFLAGS:=      -g -O3 -I. -I../include -I$(CFITSIO_SHARED_DIR)/include
CPPCOPTS :=	-c -Wall -funroll-loops -O3 -Wno-unused-variable 

#
# These are all the codes that are self contained in the system.
#
all: objects ${LIBIMSUPPORT}($(OBJECTS)) rand

standalone: standaloneobjects ${LIBIMSUPPORT}($(STANDALONEOBJECTS)) rand

standaloneobjects: $(STANDALONEOBJECTS)

rand:
	ranlib ${LIBIMSUPPORT}

objects: $(OBJECTS)

%.o : %.cc
	$(CPPC) $(CPPFLAGS) $(CPPCOPTS) -c $^ -o $@


# We build this with "-O0" to "fix" it:                                                                             
imreadsubs.o: imreadsubs.c
	$(CC) $(CFLAGS) -O0 -c $^ -o $@  -I../include -I$(CFITSIO_SHARED_DIR)/include/

clean: tidy
	rm -f ${LIBIMSUPPORT}
tidy: 
	rm -f *.o
	rm -f  *~ \#*\#

#
# Upper level makefile does the install and clean_install for this direcorty
# throw these falses in as and assertions	
install:
	false upper level makefile does install.
clean_install:
	false upper level makefile does clean_install.
