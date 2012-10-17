// Utilities for simple string manipulations

#include <stdio.h>
#include <string.h>

int build_command_line(int argc,char *argv[],char *command_line,int limit)
{
  int retval = 0;
  int argcc = 0;
  while((argcc < argc)&& ( retval <= limit)){
    retval = strlen(command_line);
    if(argv[argcc]){
      int curlen = strlen(argv[argcc]);
      if(retval+curlen <= limit)
	strcat(command_line,argv[argcc++]);
      retval = strlen(command_line);
      if((argcc < argc) && retval < limit)
	strcat(command_line," ");
    }
    else
      break;
  }
  if(argc != 0)
    retval *= -1;
  return(retval);
}


