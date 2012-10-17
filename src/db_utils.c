#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "imsupport.h"

/* pull db user/passwd@platform/name from protected file */

#define DB_USER "DB_USER"
#define DB_PASSWD "DB_PASSWD"
#define DB_SERVER_RW "DB_SERVER"
#define DB_NAME_RW "DB_NAME"
#define DB_SERVER_RO "DB_SERVER_STANDBY"
#define DB_NAME_RO "DB_NAME_STANDBY"
#define DB_SERVER_FN "DB_SERVER_FNAL"
#define DB_NAME_FN "DB_NAME_FNAL"

void select_dblogin(dblogin,mode)
	char *dblogin;
	int mode;
{
	FILE	*inp;
	char	line[1000],filename[1000],db_user[100],db_passwd[100],
	  db_server[1000],db_name[100],trash[1000],keyword[100],
	  value[1000];

	/* define hidden file name  */
	sprintf(filename,"%s/.desdm",getenv("HOME"));
	inp=fopen(filename,"r");
	if (inp==NULL) {
	  printf("** DESDM configuration file %s not found.\n",
	    filename);
	  exit(0);
	}	
	while (fgets(line,1000,inp)!=NULL) {
	  sscanf(line,"%s %s",keyword,value);
	  if (!strcmp(keyword,DB_USER)) sprintf(db_user,"%s",value);
	  if (!strcmp(keyword,DB_PASSWD)) sprintf(db_passwd,"%s",value);


	  if (mode==DB_READWRITE) {

	    if (!strcmp(keyword,DB_SERVER_RW)) sprintf(db_server,"%s",value);
	    if (!strcmp(keyword,DB_NAME_RW)) sprintf(db_name,"%s",value);

	  }
	  else if (mode==DB_READONLY) {

	    if (!strcmp(keyword,DB_SERVER_RO)) sprintf(db_server,"%s",value);
	    if (!strcmp(keyword,DB_NAME_RO)) sprintf(db_name,"%s",value);

   	  }	  
	  else if (mode==DB_FNAL) {
	    if (!strcmp(keyword,DB_SERVER_FN)) sprintf(db_server,"%s",value);
            if (!strcmp(keyword,DB_NAME_FN)) sprintf(db_name,"%s",value);
	  }
	  else {
	    printf("  Unknown DB access mode: %d\n",mode);
	    exit(0);

	  }
	}
	fclose(inp);

	/* create the db call */
	sprintf(dblogin,"%s/%s@%s/%s",db_user,db_passwd,
	  db_server,db_name);
}

#define MAXNODES 20

/* retrieve the archive node information from the db */
void select_archivenode(dblogin,arnode,arroot,arsites)
	char *dblogin,*arnode,*arroot,*arsites;
{
	FILE	*out,*pip;
	char	filename[100],arhost[100],trash[1000],sqlcall[5000];
	int	arid,n,i,num_nodes;

	/* first produce query */
        sprintf(filename,"select_archivenode.sqlquery");
        out=fopen(filename,"w");
        fprintf(out,"SET NEWP 0 SPA 1 PAGES 0 FEED OFF COLSEP ' |'");
        fprintf(out,"HEAD OFF TRIMS ON LINESIZE 1000;\n");
        /* first find out how many solutions to expect */
        fprintf(out,"SELECT LOCATION_NAME||'|'||LOCATION_ID||' |'||ARCHIVE_HOST||' |'||ARCHIVE_ROOT FROM ARCHIVE_SITES;\n");
        fprintf(out,"exit;\n");
        fclose(out); 
	/* execute query */
	sprintf(sqlcall,"${ORACLE_HOME}/bin/sqlplus -S %s < %s",
	  dblogin,filename);
	pip=popen(sqlcall,"r");
	printf("  Looking for %s\n    Known archive sites: |",arnode);
	arid=0;
	while (fgets(trash,1000,pip)!=NULL) {
	  i=n=0;
	  /* find first pipe */
	  while (strncmp("|",trash+i,1)) i++;
	  /* truncate string at first pipe */
	  trash[i]=0;
	  //if (!strncmp(trash,arnode,i-1)) {
	  /* compare two archive node names */
	  if (!strcmp(trash,arnode)) {
	    sscanf(trash+(++i),"%d",&arid);
	    while (strncmp(trash+i,"|",1)) i++;
	    sscanf(trash+(++i),"%s",arhost);
	    while (strncmp(trash+i,"|",1)) i++;
	    sscanf(trash+(++i),"%s",arroot);
	  }
	  else { /* if no match then just check the number of nodes */
	    sscanf(trash+(++i),"%d",&n);
	    if (n>num_nodes) num_nodes=n;
	  }
	  /* print archive site */
	  printf("%s",trash);
	  printf("|");
	}
	pclose(pip);
	if (arid==0) { 
	  printf("\n\n**  Archive node %s unknown\n",arnode);
	  exit(0);
	}

	arsites[0]=0;
	if (arid>num_nodes) num_nodes=arid;
	if (num_nodes>MAXNODES) num_nodes=MAXNODES;
	for (i=0;i<num_nodes;i++) 
	  if (i==arid-1) sprintf(arsites,"%sY",arsites);
	  else sprintf(arsites,"%sN",arsites);

	printf("\n    %s is site %d of %d with root directory %s and archive_site %s\n",
	  arnode,arid,num_nodes,arroot,arsites); 

	/* choong's addition 12/05/2007 */
	system("rm select_archivenode.sqlquery");
	/* end choong's addition 12/05/2007 */
}

/* subroutine that returns the number of active DB jobs with program like 'name' */
int numdbjobs(name,root,dblogin)
        char name[],root[],dblogin[];
{
        FILE    *out,*pip;
        int     numjobs;
        char    queryfile[500],query[500];

        sprintf(queryfile,"%s_numdbjobs.sql",root);
        out=fopen(queryfile,"w");
        fprintf(out,"SET NEWP 0 SPA 1 PAGES 0 FEED OFF COLSEP '|'");
        fprintf(out,"HEAD OFF TRIMS ON LINESIZE 1000;\n");
        /* first find out how many solutions to expect */
        fprintf(out,"SELECT count(DB_system_id)\n");
        fprintf(out,"FROM DES_ACTIVE_SESSIONs\n");
        fprintf(out,"WHERE program like '\%%");
        fprintf(out,"%s",name);
        fprintf(out,"\%';\n");
        fprintf(out,"exit;\n");
        fclose(out);

        sprintf(query,"${ORACLE_HOME}/bin/sqlplus -S %s < %s",
          dblogin,queryfile);
        pip=popen(query,"r");
        fscanf(pip,"%d",&numjobs);
        pclose(pip);
        return(numjobs);
}

/* subroutine that returns the number of active DB jobs with program like 'name' */
int table_exists(dblogin,dbtable,root)
        char dblogin[],dbtable[],root[];
{
        FILE    *out,*pip;
        int     numtables,i;
        char    queryfile[500],temp[500],dbuser[500],query[500];

	/* pull dbuser from the dblogin string */
	sprintf(temp,"%s",dblogin);
	for (i=0;i<strlen(temp);i++) if (!strncmp(temp+i,"/",1)  || 
	  !strncmp(temp+i,"@",1)) temp[i]=32;
	sscanf(temp,"%s",dbuser);

	/* now check whether table exists */
        sprintf(queryfile,"%s_table_exists.sql",root);
        out=fopen(queryfile,"w");
        fprintf(out,"SET NEWP 0 SPA 1 PAGES 0 FEED OFF COLSEP '|'");
        fprintf(out,"HEAD OFF TRIMS ON LINESIZE 1000;\n");
        /* first find out how many solutions to expect */
        fprintf(out,"SELECT count(1) from dba_tables\n");
        fprintf(out,"WHERE owner=UPPER('%s') and table_name=UPPER('%s');\n",
	  dbuser,dbtable);
	fprintf(out,"exit;\n");
        fclose(out);

        sprintf(query,"${ORACLE_HOME}/bin/sqlplus -S %s < %s",
          dblogin,queryfile);
        pip=popen(query,"r");
        fscanf(pip,"%d",&numtables);
        pclose(pip);
        return(numtables);
}

/* subroutine that returns the number of active DB jobs with program like 'name' */
int create_table(dblogin,dbtable,tabletype,root)
        char dblogin[],dbtable[],tabletype[],root[];
{
        FILE    *out,*pip;
        int     numtables;
        char    queryfile[500],query[500];

        sprintf(queryfile,"%s_create_table.sql",root);
        out=fopen(queryfile,"w");
	if (!strcmp(tabletype,"OBJECTS"))
        fprintf(out,"create table %s as select * from %s where object_id=-9999;\nexit;",
	  dbtable,tabletype);
	else {
	  printf("  Table template %s not found!\n",tabletype);
	  exit(0);
	}
	fclose(out);

        sprintf(query,"${ORACLE_HOME}/bin/sqlplus -S %s < %s",
          dblogin,queryfile);
	system(query);
}

/* subroutine that returns the number of active DB jobs with program like 'name' */
int create_table_partition(dblogin,dbtable,tabletype,root,tablespace)
        char dblogin[],dbtable[],tabletype[],root[],tablespace[];
{
        FILE    *out,*pip;
        int     numtables,i;
        char    queryfile[500],query[500];
	char	tabletemplate[100][50]={
		"(",
		"OBJECT_ID       Number(11,0) NOT NULL,",
		"EQUINOX         Number(6,2),",
		"BAND	         Varchar2(10),",
		"HTMID	         NUMBER(16,0),",
		"CX              Number(10,6),",
		"CY              Number(10,6),",
		"CZ              Number(10,6),",
		"ParentID        Number(11,0),",
		"SoftID          Number(4,0),",
		"ImageID         Number(9,0),",
		"Zeropoint       Number(7,4),",
		"ERRZeropoint    Number(7,4),",
		"ZeropointID     Number(10,0),",
		"OBJECT_NUMBER   Number(6,0),",
		"MAG_AUTO        Number(7,4),",
		"MAGERR_AUTO     Number(7,4),",
		"MAG_APER_1      Number(7,4),",
		"MAGERR_APER_1   Number(7,4),",
		"MAG_APER_2      Number(7,4),",
		"MAGERR_APER_2   Number(7,4),",
		"MAG_APER_3      Number(7,4),",
		"MAGERR_APER_3   Number(7,4),",
		"MAG_APER_4      Number(7,4),",
		"MAGERR_APER_4   Number(7,4),",
		"MAG_APER_5      Number(7,4),",
		"MAGERR_APER_5   Number(7,4),",
		"MAG_APER_6      Number(7,4),",
		"MAGERR_APER_6   Number(7,4),",
		"ALPHA_J2000     Number(8,5),",
		"DELTA_J2000     Number(8,5),",
		"ALPHAPEAK_J2000 Number(8,5),",
		"DELTAPEAK_J2000 Number(8,5),",
		"X2_WORLD        Number(10,2),",
		"ERRX2_WORLD     Number(10,2),",
		"Y2_WORLD        Number(10,2),",
		"ERRY2_WORLD     Number(10,2),",
		"XY_WORLD        Number(10,2),",
		"ERRXY_WORLD     Number(10,2),",
		"THRESHOLD       Number(6,2),",
		"X_IMAGE         Number(6,2),",
		"Y_IMAGE         Number(6,2),",
		"XMIN_IMAGE      Number(4,0),",
		"YMIN_IMAGE      Number(4,0),",
		"XMAX_IMAGE      Number(4,0),",
		"YMAX_IMAGE      Number(4,0),",
		"X2_IMAGE        Number(10,2),",
		"ERRX2_IMAGE     Number(10,2),",
		"Y2_IMAGE        Number(10,2),",
		"ERRY2_IMAGE     Number(10,2),",
		"XY_IMAGE        Number(10,2),",
		"ERRXY_IMAGE     Number(10,2),",
		"A_IMAGE         Number(6,2),",
		"ERRA_IMAGE      Number(6,2),",
		"B_IMAGE         Number(6,2),",
		"ERRB_IMAGE      Number(6,2),",
		"THETA_IMAGE     Number(4,2),",
		"ERRTHETA_IMAGE  Number(4,2),",
		"ELLIPTICITY     Number(5,4),",
		"CLASS_STAR      Number(3,2),",
		"FLAGS           Number(3,0),",
		"PARTKEY         Varchar2(80)",
		")","endoftable"} ;

	system("date");
	sprintf(queryfile,"%s_create_table_partition.sql",root);
        out=fopen(queryfile,"w");
	if (!strcmp(tabletype,"PART_OBJECTS")) {
          /*fprintf(out,"alter table %s add partition %s\n",tabletype,runid);
          fprintf(out,"values ('%s');\n",runid);*/
          fprintf(out,"create table %s ",dbtable);
	  i=0;
	  while (strcmp(tabletemplate[i],"endoftable")) 
	    fprintf(out,"%s\n",tabletemplate[i++]);
	  fprintf(out,"tablespace %s;\n",tablespace);
          fprintf(out,"exit;\n");
	}
	else {
	  printf("  Table template %s not found!\n",tabletype);
	  exit(0);
	}
	fclose(out);

        sprintf(query,"${ORACLE_HOME}/bin/sqlplus -S %s < %s",
          dblogin,queryfile);
	system(query);
	system("date");
}


#undef MAXNODES
#undef DB_USER
#undef DB_PASSWD
#undef DB_SERVER
#undef DB_NAME


