SHELL=/bin/sh

DIRS=lib include ups src

#
#  PREFIX is used solely for the install target, right?
#
# Set PREFIX to "." in undefined -- FM
ifndef PREFIX
  export PREFIX=.
endif

#
# The pattern of this makefile is to call a subordinate makefile for each of the tasks, if a Makefile is 
# is present in the subordinate directory, else to to the task. Install is an exception.
#
all: 
	for d in $(DIRS) ; do if [ -e $$d/Makefile ] ; then (cd $$d;$(MAKE) $(MAKEFLAGS) all ) fi ; done

clean: tidy
	for d in $(DIRS) ; do if [ -e $$d/Makefile ] ; then (cd $$d;$(MAKE) $(MAKEFLAGS) clean  ) fi ; done

tidy:
	for d in $(DIRS) ; do if [   -e $$d/Makefile ] ; then (cd $$d;$(MAKE) $(MAKEFLAGS) tidy  )  fi ; done
	for d in $(DIRS) ; do if [ ! -e $$d/Makefile ] ; then (cd $$d ; rm -f \#*\#  *~ )           fi ; done
	rm -rf \#*\#  *~

install: tidy
	if [ -z "$(PREFIX)" ] ; then echo install area not defined; exit 1 ; fi
	echo about to install under $(PREFIX)
	sleep 5
	for d in $(DIRS) ; do  mkdir -p $(PREFIX)/$$d ; cp $$d/* $(PREFIX)/$$d  ; done
