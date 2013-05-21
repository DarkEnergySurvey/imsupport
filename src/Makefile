SHELL=/bin/sh

ifndef PREFIX
  ifdef DES_HOME
    PREFIX=${DES_HOME}
  else
    PREFIX=../..
  endif
endif

ifndef FITSIO_BASE
  ifdef DES_PREREQ
    FITSIO_BASE=${DES_PREREQ}
  endif
endif


OBJECTS=imarithsubs.o archivesubs.o catsubs.o  db_utils.o eventsubs.o imreadsubs.o lut_utils.o subs_fits.o subs_memory.o subs_inoutput.o wcssubs.o readstdstable.o ImageMorphology.o LegacyInterface.o ComLine.o primitive_utilities.o System.o MorphologyCX.o argutils.o

STANDALONEOBJECTS=imarithsubs.o archivesubs.o catsubs.o eventsubs.o imreadsubs.o subs_fits.o subs_memory.o subs_inoutput.o wcssubs.o readstdstable.o ImageMorphology.o LegacyInterface.o ComLine.o primitive_utilities.o System.o MorphologyCX.o argutils.o

CFLAGS +=  -I../include -I$(FITSIO_BASE)/include/ -I$(FITSIO_BASE)/include/wcs/ -I$(FITSIO_BASE)/include/psfex/ -I$(FITSIO_BASE)/include/fits/

CPPC := 	g++
CPPFLAGS:=      -g -O3 -I. -I../include -I$(FITSIO_BASE)/include
CPPCOPTS :=	-c -Wall -funroll-loops -O3 -Wno-unused-variable 

LIBIMSUPPORT=../lib/libimsupport.a
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
	$(CC) $(CFLAGS) -O0 -c $^ -o $@  -I../include -I$(FITSIO_BASE)/include/

#ImageMorphology.o: ImageMorphology.cc
#	$(CPPC) $(CPPFLAGS) -I. -I../ -c ImageMorphology.cc -o $@
#LegacyInterface.o: LegacyInterface.cc
#	$(CPPC) $(CPPFLAGS) -I. -I../ -c LegacyInterface.cc -o $@

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
