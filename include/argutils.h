//
// argutils.h
// 
// Collection of utilities for simple string manipulations
//

//
// build_command_line: 
// 
// Utility to take command line arguments and build a single command 
// line string in a caller provided char array "command_line", of 
// length "limit".
//
// If everything is successful, then the return value is the length of 
// the returned string.  If the command line string overruns the maximum
// allowed length (as specified by "limit"), or if argv is malformed,
// then a negative value will be returned. If nothing is done 
// (i.e. argc = 0, or argv empty, then the return value is 0.  
//
int build_command_line(int argc,char *argv[],char *command_line,int limit);
