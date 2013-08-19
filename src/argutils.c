// Utilities for simple string manipulations

#include <stdio.h>
#include <string.h>

int build_command_line(int argc,char *argv[],char *command_line,int limit)
{
  int retval = 0;
  int argcc = 0;
  int limit_reached = 0;
  while((argcc < argc) && (!limit_reached)){
    retval = strlen(command_line);
    if(argv[argcc]){
      int curlen = strlen(argv[argcc]);
      if(retval+curlen <= limit)
	strcat(command_line,argv[argcc++]);
      else 
	limit_reached++;
      retval = strlen(command_line);
      if((argcc < argc) && retval < limit)
	strcat(command_line," ");
      else if (retval >= limit)
	limit_reached++;
    }
    else
      break;
  }
  if(argcc != argc)
    retval *= -1;
  return(retval);
}


