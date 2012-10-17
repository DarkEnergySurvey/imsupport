#include <stdio.h>
#include <stdlib.h>
#include "imsupport.h"

/* ******************************************************** */
/* subroutine used to report status and qa events to STDOUT */
/* ******************************************************** */
void reportevt(flag_verbose,type,level,event)
	int flag_verbose,type,level;
	char event[];
{
	void putevent();
	if (flag_verbose>1) {
	  putevent(type,level,event);
	  fflush(stdout);
	}
	else {
	  if (level==5) printf("** %s **\n",event);
	  else  printf("  %s\n",event);
	}
}



/* ******************************************************** */
/* ****** subroutine to issue status and qa events ******** */
/* ******************************************************** */
void putevent(type,level,event)
	int type,level;
	char event[];
{

	/* issue STATUS event */
	if (type==STATUS) 
	  printf("STATUS%dBEG %s STATUS%dEND\n",level,event,level);
	/* issue QA event */
	else if (type==QA) 
	  printf("QA%dBEG %s QA%dEND\n",level,event,level);
	/* unknown even type */
	else {
	  printf("STATUS5BEG  Unknown event type %d  STATUS5END\n",type);
	  exit(0);
	}

}

