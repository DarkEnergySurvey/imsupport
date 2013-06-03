/* image arithmetic subroutines */
#define MEDIAN_SUB 0
#define polint3 1
#define polint2 2
#define spline2 3
#define spline3 4
#define MEAN_SUB 5

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "imsupport.h"
#include "inlined_qsort.h"

void float_qsort(float *arr, unsigned int n)
{
   #define float_lt(a,b) ((*a)<(*b))
   QSORT(float, arr, n, float_lt);
}


/* ************************************************************** */
/* ******************** overscan correction ********************* */
/* ************************************************************** */

/* summing the min-max rejected array values for calculating mean*/
int os_sum_array(int *a, int num_elements)
{
  int i, sum=0;
  for (i=0; i<num_elements; i++)
    {
      sum = sum + a[i];
    }
  return(sum);
}


void overscan(data,output,flag_verbose,overscantype)
     desimage *data,*output;
     int flag_verbose;
     int overscantype;
{
  float   *vecsort0,*vecsort1;
  float   *overscan0,*overscan1,*overscanwminmax0,*overscanwminmax1;
  int     length,y,x,xmin,xmax,j,overflowA=0,overflowB=0;
  unsigned long width;
  void    shell(),reportevt();
  void    polint();
  void    spline();
  void    splint();
  char    event[10000];
  float   *rowval,yval,dy,yp1,ypa,*y2,ysp;
  float   yval1,dy1,yp11,ypa1,*y21,ysp1;
  float   *yfit,*overscanfit,*overscanfit1;
  float	minoverscan0=10000,maxoverscan0=0,minoverscan1=10000,maxoverscan1=0;	
  /* create overscan vectors for each amp */
  /* assume that the overscan is a set of columns of */
  /* equal size on both sides of image */
  length=data->biassecan[3]-data->biassecan[2]+1;
  overscan0=(float *)calloc(length,sizeof(float));
  if (overscan0==NULL) {
    sprintf(event,"Calloc of overscan0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);

  }
  overscan1=(float *)calloc(length,sizeof(float));
  if (overscan1==NULL) {
    sprintf(event,"Calloc of overscan1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0); 
  }
  y2=(float *)calloc(length,sizeof(float));
  if (y2==NULL) {
    sprintf(event,"Calloc of y2 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  y21=(float *)calloc(length,sizeof(float));
  if (y21==NULL) {
    sprintf(event,"Calloc of y2 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  yfit=(float *)calloc(length,sizeof(float));
  if (yfit==NULL) {
    sprintf(event,"Calloc of yfit failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0); 
  }
  overscanfit=(float *)calloc(length,sizeof(float));
  if (overscanfit==NULL) {
    sprintf(event,"Calloc of overscanfit failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0); 
  }
  overscanfit1=(float *)calloc(length,sizeof(float));
  if (overscanfit1==NULL) {
    sprintf(event,"Calloc of overscanfit1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  overscanwminmax0=(float *)calloc(length,sizeof(float));
  if (overscanwminmax0==NULL) {
    sprintf(event,"Calloc of overscanwminmax0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  overscanwminmax1=(float *)calloc(length,sizeof(float));
  if (overscanwminmax1==NULL) {
    sprintf(event,"Calloc of overscanwminmax1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }

  vecsort0=(float *)calloc(length,sizeof(float));
  if (vecsort0==NULL) {
    sprintf(event,"Calloc of vecsort0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  rowval=(float *)calloc(length,sizeof(float));
  if (rowval==NULL) {
    sprintf(event,"Calloc of rowval failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }

  vecsort0=(float *)calloc(length,sizeof(float));
  if (vecsort0==NULL) {
    sprintf(event,"Calloc of vecsort0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  vecsort1=(float *)calloc(length,sizeof(float));
  if (vecsort1==NULL) {
    sprintf(event,"Calloc of vecsort1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  width=data->biassecan[1]-data->biassecan[0]+1;
  for (y=0;y<length;y++) {
    rowval[y]=y;
    /* copy overscan for current line into sorting vector */
    for (x=0;x<width;x++) {
      vecsort0[x]=data->image[y*data->axes[0]+x+data->biassecan[0]-1];
      vecsort1[x]=data->image[y*data->axes[0]+x+data->biassecbn[0]-1];
      if (overscantype==0 && y==1)printf("x,vecsort[x], %d, %f \n", x, vecsort0[x]);
    }

    /* sort the vectors */
    shell(width,vecsort0-1);
    shell(width,vecsort1-1);

    /* trim overscan by cutting out the lowest two and the highest two values*/
    int newwidth=width-16;
    int minmaxed0[newwidth];
    int minmaxed1[newwidth];   
    int i;
    for (i = 0;i < newwidth;i++){
      minmaxed0[i]=vecsort0[i+8];
      minmaxed1[i]=vecsort1[i+8];	
    }	
    /* copy median into overscan vectors */
    overscan0[y]=vecsort0[width/2]; 
    overscan1[y]=vecsort1[width/2];
    
    int sum0=os_sum_array(minmaxed0, newwidth);
    int sum1=os_sum_array(minmaxed1, newwidth);
    /* copy mean into overscan vectors */
    overscanwminmax0[y]=sum0/newwidth;/*overscan0 with the lowest two and the highest two values removed*/
    overscanwminmax1[y]=sum1/newwidth;/*overscan1 with the lowest two and the highest two values removed*/


    if (overscan0[y] < minoverscan0){minoverscan0=overscan0[y];}/*look at ways to improve this*/
    if (overscan0[y] > maxoverscan0){maxoverscan0=overscan0[y];}
    if (overscan1[y] < minoverscan1){minoverscan1=overscan1[y];}/*look at ways to improve this*/
    if (overscan1[y] > maxoverscan1){maxoverscan1=overscan1[y];}
  }
  sprintf(event,"min,maximum in amplifier A = %f , %f \n", minoverscan0,maxoverscan0);
  reportevt(flag_verbose,STATUS,1,event);
  sprintf(event,"min,maximum in amplifier B = %f , %f \n", minoverscan1,maxoverscan1);
  reportevt(flag_verbose,STATUS,1,event);
		
	
  j=0;
  for (y=0;y<length;y++) {
    yfit[j]=(float) y; /*this creates a new array for later evaluating the median at an intervals of 50*/
		
    /* 
       all of the interpolation except Med_SUB use the min-max rejected overscan values. 
       and redefining a new array called overscanfit makes it easier to change between 
       the min-max rejected and min-max included values by only changing the definition below 
    */
    overscanfit[j]=overscanwminmax0[y];j+=1;
    overscanfit1[j]=overscanwminmax1[y];
	      
  }
  if (overscantype==0){
    sprintf(event,"Doing Median Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  else if (overscantype==1){
    sprintf(event,"Doing 3rd Order Polynomial Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  else if (overscantype==2){
    sprintf(event,"Doing 2rd Order Polynomial Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  else if (overscantype==3){
    sprintf(event,"Doing 2rd Order Spline Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  else if (overscantype==4) {
    sprintf(event,"Doing 3rd Order Spline Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  else {
    sprintf(event,"Doing Mean Interpolation");
    reportevt(flag_verbose,STATUS,1,event);
  }
  /*calcuating derivatives yp1 --at pt 1--and ypa--at last pt-- for overscan 0*/
  yp1=(overscanfit[1]-overscanfit[0])/(1);
  ypa=(overscanfit[length-1]-overscanfit[length-2])/(1);
  spline(rowval-1,overscanfit-1,3,yp1,ypa,y2-1);
  /*calcuating derivatives yp11 --at pt 1--and ypa1--at last pt-- for overscan 1*/
  yp11=(overscanfit1[1]-overscanfit1[0])/(1);
  ypa1=(overscanfit1[length-1]-overscanfit1[length-2])/(1);
  spline(rowval-1,overscanfit1-1,3,yp11,ypa1,y21-1);
  /* create a single image array that fits the trimmed data */
  output->axes[0]=1+data->trimsecn[1]-data->trimsecn[0];
  output->axes[1]=1+data->trimsecn[3]-data->trimsecn[2];
  output->nfound=2;
  output->npixels=output->axes[0]*output->axes[1];
  output->image=(float *)calloc(output->npixels,sizeof(float));
  if (output->image==NULL) {
    sprintf(event,"Calloc of output->image failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  output->saturateA=data->saturateA;output->saturateB=data->saturateB;
  output->gainA=data->gainA;output->gainB=data->gainB;
  output->rdnoiseA=data->rdnoiseA;output->rdnoiseB=data->rdnoiseB;
	  
  /* copy the image into the new data array with overscan correction */
  /* Amp  A/B flip checked  */  
  if(data->ampsecan[0] < data->ampsecan[1]){
    xmin=data->ampsecan[0]+data->datasecn[0]-1;
    xmax=data->ampsecan[1]+data->datasecn[0]-1;
  }
  else{
    xmin=data->ampsecan[1]+data->datasecn[0]-1;
    xmax=data->ampsecan[0]+data->datasecn[0]-1;
  } 
    
  for (y=data->trimsecn[2];y<=data->trimsecn[3];y++) 
    for (x=data->trimsecn[0];x<=data->trimsecn[1];x++) {
      if (x>=xmin && x<=xmax) /* then we are in AMPSECA */   {
	if (overscantype == MEDIAN_SUB) {
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-
	    overscan0[y-data->trimsecn[2]];
	  if (x==xmin)printf("y %d, overscan fit %f\n", (int)(y-data->trimsecn[2]), (overscan0[y-data->trimsecn[2]]));
	}
	else if (overscantype == polint3){
	  if (x==xmin){polint(yfit-1,overscanfit-1,length,(float) y-data->trimsecn[2],&yval,&dy);
	    printf("y %d, overscan fit %d\n", (int)(y-data->trimsecn[2]), (int)yval);

	    if (yval < minoverscan0 || yval > maxoverscan0){
	      /*yval = overscan0[y-data->trimsecn[2]]; based upon Robert G's comments */
	      overflowA++ ;}
	  }
	

	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-yval;
	      			      

	
	}
	else if (overscantype == polint2){
	  if (x==xmin){polint(yfit-1,overscanfit-1,2,(float) y-data->trimsecn[2],&yval,&dy);

	    if (yval < minoverscan0 || yval > maxoverscan0){
	      /*yval = overscan0[y-data->trimsecn[2]];*/ 
	      overflowA++ ;}}
	

	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-yval;
	      		      

	
	}
	else if (overscantype == spline2){
	  if (x==xmin){splint(yfit-1,overscanfit-1,y2-1,2,(float) y-data->trimsecn[2],&ysp);

	    if (ysp < minoverscan0 || ysp > maxoverscan0){
	      /*ysp = overscan0[y-data->trimsecn[2]]; */
	      overflowA++ ;}}
	    	
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-ysp;
	      	      

	
	}
	else if (overscantype == spline3){
	  if (x==xmin){splint(yfit-1,overscanfit-1,y2-1,3,(float) y-data->trimsecn[2],&ysp);

	    if (ysp < minoverscan0 || ysp > maxoverscan0){
	      /*ysp = overscan0[y-data->trimsecn[2]]; */ 
	      overflowA++ ;}}
	    	
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-ysp;

	}
	else {
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-
	    overscanwminmax0[y-data->trimsecn[2]];
	  if (x==xmin)printf("y %d, overscan fit %f\n", (int)(y-data->trimsecn[2]), (overscanwminmax0[y-data->trimsecn[2]]));

	}
      }
      else /* assume we are in AMPSECB */  {
	if (overscantype == MEDIAN_SUB){ 
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=
	    data->image[(y-1)*data->axes[0]+x-1]-
	    overscan1[y-data->trimsecn[2]];
	  /*if (x==xmax+1)printf("y %d, overscan fit %f\n", (int)(y-data->trimsecn[2]), (overscan1[y-data->trimsecn[2]]));*/
		

	}
	else if (overscantype == polint3){
	  if (x==xmax+1){polint(yfit-1,overscanfit1-1,3,(float) y-data->trimsecn[2],&yval1,&dy1);

	    /*printf("y %d, overscan fit %d\n", (int)(y-data->trimsecn[2]), (int)yval1);	  */      

	    if (yval1 < minoverscan1 || yval1 > maxoverscan1){
	      /*yval1 = overscan1[y-data->trimsecn[2]]; */
	      overflowB++ ;}}
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=
	    data->image[(y-1)*data->axes[0]+x-1]-yval1;
	      

		
	}
	else if (overscantype == spline2){
	  if (x==xmax+1){splint(yfit-1,overscanfit1-1,y21-1,2,(float) y-data->trimsecn[2],&ysp1);

	    if (ysp < minoverscan1 || ysp > maxoverscan1){
	      /*ysp1 = overscan1[y-data->trimsecn[2]]; */ 
	      overflowB++ ;}}
	    	
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-ysp;
	      	      

	
	}
	else if (overscantype == spline3){
	  if (x==xmax+1){splint(yfit-1,overscanfit1-1,y21-1,3,(float) y-data->trimsecn[2],&ysp1);
	    if (ysp1 < minoverscan1 || ysp1 > maxoverscan1){
	      /*ysp1 = overscan1[y-data->trimsecn[2]]; */
	      overflowB++;} }
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=
	    data->image[(y-1)*data->axes[0]+x-1]-ysp1;
	}
	else {
	  output->image[(y-data->trimsecn[2])*output->axes[0]+
			(x-data->trimsecn[0])]=data->image[(y-1)*data->axes[0]+x-1]-
	    overscanwminmax1[y-data->trimsecn[2]];
	}
      }
    }
  if (overscantype > 0 && (overflowA>0 || overflowB>0))
    { sprintf(event,"Output of inpoterpolation routines exceeded bounds in Amp. A %d times and in Amp B %d times for image %s\n",overflowA,overflowB,data->name);
      reportevt(flag_verbose,STATUS,4,event);
    }


}


void getxyz(ra,dec,x,y,z)
     double ra,dec;  
     double *x,*y,*z;     
{
  double cd,pr=M_PI/180.0;

  ra*=pr; dec*=pr;
  
  cd=cos(dec);
  *x=cos(ra)*cd;
  *y=sin(ra)*cd;
  *z=sin(dec);
}


/* ************************************************************** */
/* *** subroutine calculates the median value within an image *** */
/* ************************************************************** */

void  retrievescale(desimage *image,int *scaleregionn,float *scalesort,
		    int flag_verbose,float *scalefactor,
		    float *mode,float *sigma)
{
  int	i,x,y,loc,xdim,npixels,width,j,
    jmax,jplus,jminus,num;
  float	*histx,*histy,ymax,fraction;
  void	shell(),reportevt();
  char	event[10000];

  i=0;
  xdim=image->axes[0];
  npixels=image->npixels;
  /* copy good image values into sorting vector */
  for (y=scaleregionn[2]-1;y<scaleregionn[3];y++) {
    for (x=scaleregionn[0]-1;x<scaleregionn[1];x++) {
      loc=x+y*xdim;
      if (loc>=0 && loc<npixels) {
	/*			printf("loc= %d image->mask[loc]= %i\n",loc,image->mask[loc]);*/
	if (!(image->mask[loc])) { /* of pixel not masked */
	  scalesort[i++]=image->image[loc];
	} 
      }
    }
  }
	
  if (i<100) {
    /*  sprintf(event,"Useable scale region = [%d:%d,%d:%d]  & too small for image= %s", */
    /*    scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3],                 */
    /*    image->name); */
    sprintf(event,"image=%s & UseableScaleRegion = [%d:%d,%d:%d]",image->name,
	    scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3]);
    reportevt(flag_verbose,QA,3,event);
    *scalefactor=0.0; /* mark image as unuseable */
  }
  else {
    /* sort list */
    shell(i,scalesort-1);
    /* grab the median */
    if (i%2) *scalefactor=scalesort[i/2];
    else *scalefactor=0.5*(scalesort[i/2]+scalesort[i/2-1]);
    /* examine histogram */
    width=1000;
    num=i/width/2;
    histx=(float *)calloc(num,sizeof(float));
    if (histx==NULL) {
      sprintf(event,"Calloc of histx failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(0);
    }
    histy=(float *)calloc(num,sizeof(float));
    if (histy==NULL) {
      sprintf(event,"Calloc of histy failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(0);
    }
    ymax=0.0;jmax=jplus=jminus=0;
    for (j=num;j>0;j--) {
      loc=width+(j-1)*2*width;
      histx[j-1]=(scalesort[loc+width]+scalesort[loc-width])*0.5;
      histy[j-1]=2.0*width/(scalesort[loc+width]-scalesort[loc-width]);
      if (histy[j-1]>ymax) {
	ymax=histy[j-1];
	jmax=j-1;
      }
    }
    for (j=jmax;j<num;j++) 
      if (histy[j]<0.5*ymax) {
	jplus=j;
	break;
      }
    for (j=jmax;j>=0;j--) 
      if (histy[j]<0.5*ymax) {
	jminus=j;
	break;
      }
	    
    *mode=histx[jmax];
    *sigma=histx[jplus]-histx[jminus];
    *sigma/=2.354;  /* change to sigma assuming gaussian distribution */

    if (flag_verbose) {
      fraction= (float)i/(float)((scaleregionn[1]-scaleregionn[0]+1)*
				 (scaleregionn[3]-scaleregionn[2]+1));
      if (strncmp(image->name,"!",1)) sprintf(event,"Image=  %s & Region = [%d:%d,%d:%d] & FractionalNoOfPixels = %.4f &  Scale=%.1f & Mode=%.1f & Sigma=%.2f",
					      image->name,scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3],fraction,*scalefactor,*mode,*sigma);
      else sprintf(event,"Image= %s & Region = [%d:%d,%d:%d] & FractionaNoOfPixels = %.5f  & Scale=%.1f & Mode=%.1f & Sigma=%.2f",
		   image->name+1, scaleregionn[0],scaleregionn[1],scaleregionn[2],
		   scaleregionn[3],fraction,*scalefactor,*mode,*sigma);
		  
      reportevt(flag_verbose,QA,1,event);
    }

    free(histx);free(histy);
  }
}

void retrievescale_fast(desimage *image, int *scaleregionn, float *scalesort,
			int flag_verbose, float *scalefactor, float *mode, float *sigma)
{
  int i, x, y, yd, loc, xdim, npixels, width, j, jmax, jplus, jminus, num;
  float *histx, *histy, ymax, fraction;
  char event[10000];

  /* copy good image values into sorting vector */
  i = 0;
  xdim = image->axes[0];
  npixels = image->npixels;
  
  for (y = scaleregionn[2]-1; y < scaleregionn[3]; y++)
  {
    yd = y * xdim;
    
    for (x = scaleregionn[0]-1; x < scaleregionn[1]; x++)
    {
      loc = x + yd;
      
      if (loc >= 0 && loc < npixels)
      {
        if (!(image->mask[loc])) /* of pixel not masked */
        { 
            scalesort[i++] = image->image[loc];
        } 
      }
    }
  }

  if (i<100) 
  {
    sprintf(event,"image=%s & UseableScaleRegion = [%d:%d,%d:%d]",image->name,
	    scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3]);
    reportevt(flag_verbose,QA,3,event);
    
    *scalefactor = 0.0; /* mark image as unuseable */
    
    return;
  }
  
  /* sort list */
  //shell(i, scalesort-1);
  //qsort(scalesort, i, sizeof(float), float_cmpfunc);
  float_qsort(scalesort, i);
  
  /* grab the median */
  if (i%2) 
    *scalefactor = scalesort[i/2];
  else 
    *scalefactor = 0.5*(scalesort[i/2] + scalesort[i/2-1]);
  
  /* examine histogram */
  width = 1000;
  num = i/width/2;
  
  histx = (float *)calloc(num, sizeof(float));
  if (histx == NULL) 
  {
    sprintf(event,"Calloc of histx failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  
  histy = (float *)calloc(num, sizeof(float));
  if (histy == NULL) 
  {
    sprintf(event,"Calloc of histy failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }
  
  ymax = 0.0f;
  jmax = jplus = jminus = 0;
  
  for (j = num; j > 0; j--) 
  {
    loc = width + (j-1) * 2 * width;
    histx[j-1] = (scalesort[loc+width] + scalesort[loc-width]) * 0.5f;
    histy[j-1] = 2.0f * width / (scalesort[loc+width] - scalesort[loc-width]);
    
    if (histy[j-1] > ymax) 
    {
      ymax = histy[j-1];
      jmax = j-1;
    }
  }
  
  for (j = jmax; j < num; j++) 
  {
    if (histy[j] < 0.5f*ymax) 
    {
        jplus = j;
        break;
    }
  }
  
  for (j = jmax; j >= 0; j--) 
  {
    if (histy[j] < 0.5f*ymax) 
    {
      jminus = j;
      break;
    }
  }

  *mode = histx[jmax];
  *sigma = histx[jplus] - histx[jminus];
  *sigma /= 2.354;  /* change to sigma assuming gaussian distribution */

  if (flag_verbose)
  {
    fraction= (float)i/(float)((scaleregionn[1]-scaleregionn[0]+1)*(scaleregionn[3]-scaleregionn[2]+1));
    if (strncmp(image->name,"!",1)) 
      sprintf(event,"Image=  %s & Region = [%d:%d,%d:%d] & FractionalNoOfPixels = %.4f &  Scale=%.1f & Mode=%.1f & Sigma=%.2f",
	      image->name,scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3],fraction,*scalefactor,*mode,*sigma);
    else
      sprintf(event,"Image= %s & Region = [%d:%d,%d:%d] & FractionaNoOfPixels = %.5f  & Scale=%.1f & Mode=%.1f & Sigma=%.2f",
	      image->name+1, scaleregionn[0],scaleregionn[1],scaleregionn[2],scaleregionn[3],fraction,*scalefactor,*mode,*sigma);

    reportevt(flag_verbose,QA,1,event);
  }
    
  free(histx);
  free(histy);

  return;
}


/* ************************************************************** */
/* ********** compare two images and return statistics ********** */
/* ************************************************************** */
void image_compare(image,template,offset,rms,maxdev,npixels,flag_verbose)
     desimage *image,*template;
     float *offset,*rms,*maxdev;
     int *npixels,flag_verbose;
{
  double	sum1=0.0,sum2=0.0;
  float	val;
  int	i;
  char	event[10000],type[50];
  void	reportevt();

  (*npixels)=0;

  /* confirm that images are same size */
  if (image->axes[0]!=template->axes[0] ||
      image->axes[1]!=template->axes[1]) {
    sprintf(event,"Image and template different size: %ldX%ld %ldX%ld",
	    image->axes[0],image->axes[1],template->axes[0],template->axes[1]);
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);		  
  }	

	

  /* compare images if those are not nulled */
  if (image->image!=NULL && template->image!=NULL) {
    sprintf(type,"Image");
    for (i=0;i<image->npixels;i++) {
      /* only compare unmasked pixels */
      if (image->mask[i]==0) {
	val=(image->image[i]-template->image[i]);
	/* keep track of maximum deviation */
	if (!(*npixels)) *maxdev=fabs(val);
	else if (fabs(val)>*maxdev) *maxdev=fabs(val);
	/* first moment sum */
	sum1+=val;
	/* second moment sum */
	sum2+=Squ(val);
	/* number of useful pixels */
	(*npixels)++;
      }
    }
  }
  else if (image->varim!=NULL && template->varim!=NULL) {
    sprintf(type,"Weight map");
    for (i=0;i<image->npixels;i++) {
      /* only compare unmasked pixels */
      if (image->mask[i]==0) {
	val=(image->varim[i]-template->varim[i]);
	/* keep track of maximum deviation */
	if (!(*npixels)) *maxdev=fabs(val);
	else if (fabs(val)>*maxdev) *maxdev=fabs(val);
	/* first moment sum */
	sum1+=val;
	/* second moment sum */
	sum2+=Squ(val);
	/* number of useful pixels */
	(*npixels)++;
      }
    }
  }
  else if (image->mask!=NULL && template->mask!=NULL) {
    sprintf(type,"Mask");
    for (i=0;i<image->npixels;i++) {
      val=(image->mask[i]-template->mask[i]);
      /* keep track of maximum deviation */
      if (!(*npixels)) *maxdev=fabs(val);
      else if (fabs(val)>*maxdev) *maxdev=fabs(val);
      /* first moment sum */
      sum1+=val;
      /* second moment sum */
      sum2+=Squ(val);
      /* number of useful pixels */
      (*npixels)++;
    }
  }
  else {
    sprintf(event,"No pair of images, weight maps or bpms present");
    reportevt(flag_verbose,STATUS,5,event);
    exit(0);
  }

  /* ************************************************* */
  /* ***** now prepare and deliver statistics ******** */
  /* ************************************************* */
  if ((*npixels)==0) {
    sprintf(event,"No pixels used in comparison");
    reportevt(flag_verbose,STATUS,4,event);
    (*rms)=(*maxdev)=(*offset)=0.0;
  }
  else {
    *offset=(float)(sum1/(*npixels));
    *rms=(float)(sum2/(*npixels)-Squ(*offset));
    if (*rms>1.0e-10) *rms=sqrt(*rms);
    if (flag_verbose) {
      sprintf(event,
	      "Image= %s & Comparison = %s  & MaxDev=%11.4e & Offset=%11.4e & RMS=%11.4e & NPIX=%d",
	      image->name, type,*maxdev,*offset,*rms,*npixels);
      reportevt(flag_verbose,QA,1,event);
    }
  }
  return;
}


void desimstat(image,scaleregion,meanval,sigma)

     desimage *image;
     int *scaleregion;
     float *meanval,*sigma;
{
  int x,y,npixels,xdim,regsize ,loc;
  float pixval,mean0=0,sigma0=0;

  xdim=image->axes[0];
  /*		printf("xdim= %d\n",xdim);*/ 
  npixels=image->npixels;
  regsize = (scaleregion[1]-scaleregion[0])*
    (scaleregion[3]-scaleregion[2]);
  /* printf("region area = %i\n",regsize); */

  for (y=scaleregion[2];y<scaleregion[3]-1;y++){
    for (x=scaleregion[0];x<scaleregion[1]-1;x++){
      loc=x+y*xdim;
      pixval = image->image[loc];
      mean0 += pixval;
    }
  }
  mean0 /= (float)regsize;
  /*	printf(" Mean Value of the image is %f\n",mean0);*/ 

  for (y=scaleregion[2];y<scaleregion[3]-1;y++){
    for (x=scaleregion[0];x<scaleregion[1]-1;x++){
      loc=x+y*xdim;
      pixval = image->image[loc];
      sigma0 += (pixval-mean0)*(pixval-mean0);
			
    }
  }
  sigma0 = sqrt(sigma0/(float)regsize);
  /*		   printf("Standard Deviation of the image is %f\n",sigma0); */ 
  *meanval = mean0;
  *sigma = sigma0;
}



/* 
    Ricardo's new overscan function.

    *******************************************************************
    ******************** Overscan correction ************************** 
    - osconfig.sample    -1  MEDIAN                                       
                          0  MEAN                                        
                          1  MEAN W/MIN MAX REJECTION                   
                                                                    
    - osconfig.function  -N  SPLINE FITTING W/ N lines/cols TO AVERAGE     
                             OR MEDIAN (given by overscansample)       
                          0  LINE_BY_LINE                                  
                          N  POLYNOMIAL FIT W/ N POINTS TO AVERAGE OR      
                             MEDIAN                                        
                                                                     
    - osconfig.order      1 < n < 6                                        
                                                                     
    - osconfig.trim       N TRIM THE TWO EDGES OF OVERSCAN BY N PIXELS     
    ******************************************************************* 
*/
void OverScan(desimage *input_image,desimage *output_image,
	      overscan_config osconfig,int flag_verbose)
{
  float sum0,sum1,min0,min1,max0,max1,value0,value1;
  float chisqr,stddev0,stddev1;
  float *vecsort0          = NULL;
  float *vecsort1          = NULL;
  float *overscan0         = NULL;
  float *overscan1         = NULL;
  float *overscanwminmax0  = NULL;
  float *overscanwminmax1  = NULL;
  float *overscanmeasured0 = NULL;
  float *overscanmeasured1 = NULL;
  float *xspline           = NULL;
  float *y2nd0             = NULL;
  float *y2nd1             = NULL;
  float *xfitspline0       = NULL;
  float *xfitspline1       = NULL;
  float **covar            = NULL;
  float *coeff0            = NULL;
  float *coeff1            = NULL;
  float *sig               = NULL;
  float *polcoef0          = NULL;
  float *polcoef1          = NULL;
  float *xfitleg0          = NULL;
  float *xfitleg1          = NULL;
  float *xfit0             = NULL;
  float *xfit1             = NULL;
  float *yfit              = NULL;
  float *yfit0             = NULL;
  float *yfit1             = NULL;
  float yp1,ypn,sumoverscan0,sumoverscan1,x0,x1;
  float ysplinefit0,ysplinefit1;
  float variance0,variance1,overscanrms0,overscanrms1,rmssum0,rmssum1;
  int i,j;
  int *inputcoeff = NULL;
  int length,nsample,nbin;
  int *nfit0 = NULL;
  int *nfit1 = NULL;
  int norder,pos;
  int x,xmin,xmax,y,yindex,xindex,yindo,xindo;
  int   indexbin,oline;
  unsigned long width;
  char event[10000];
  void shell(),reportevt();
  void lfit();
  void fleg();
  void spline();
  void splint();
  int overscansample   = osconfig.sample;
  int overscanfunction = osconfig.function;
  int overscanorder    = osconfig.order;
  int overscantrim     = osconfig.trim;
  desimage *data       = input_image;
  desimage *output     = output_image;
  
  if(flag_verbose){
    sprintf(event,"Entering OverScan(samp=%d,func=%d,order=%d,trim=%d)",
	    osconfig.sample,osconfig.function,osconfig.order,osconfig.trim);
     
    reportevt(flag_verbose,STATUS,1,event);
  }
  /* create overscan vectors for each amp */
  /* assume that the overscan is a set of columns of */
  /* equal size on both sides of image */
  /* length is number of lines (rows) the overscan has */

  /*changing name for order of fitting function to a more sensible one */
  norder = overscanorder; 

  length=data->biassecan[3] - data->biassecan[2] + 1;
  overscan0=(float *)calloc(length,sizeof(float));
  if (overscan0==NULL) {
    sprintf(event,"Calloc of overscan0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  overscan1=(float *)calloc(length,sizeof(float));
  if (overscan1==NULL) {
    sprintf(event,"Calloc of overscan1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1); 
  }
  overscanmeasured0=(float *)calloc(length,sizeof(float));
  if (overscanmeasured0==NULL) {
    sprintf(event,"Calloc of overscanmeasured0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  overscanmeasured1=(float *)calloc(length,sizeof(float));
  if (overscanmeasured1==NULL) {
    sprintf(event,"Calloc of overscanmeasured1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  yfit=(float *)calloc(length,sizeof(float));
  if (yfit==NULL) {
    sprintf(event,"Calloc of yfit failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1); 
  }
  yfit1=(float *)calloc(length,sizeof(float));
  if (yfit1==NULL) {
    sprintf(event,"Calloc of yfit1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  yfit0=(float *)calloc(length,sizeof(float));
  if (yfit0==NULL) {
    sprintf(event,"Calloc of yfit0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfit0=(float *)calloc(length,sizeof(float));
  if (xfit0==NULL) {
    sprintf(event,"Calloc of xfit0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfit1=(float *)calloc(length,sizeof(float));
  if (xfit1==NULL) {
    sprintf(event,"Calloc of xfit1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfitleg0=(float *)calloc(length,sizeof(float));
  if (xfit0==NULL) {
    sprintf(event,"Calloc of xfitleg0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfitleg1=(float *)calloc(length,sizeof(float));
  if (xfit1==NULL) {
    sprintf(event,"Calloc of xfitleg1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfitspline0=(float *)calloc(length,sizeof(float));
  if (xfitspline0==NULL) {
    sprintf(event,"Calloc of xfitspline0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xfitspline1=(float *)calloc(length,sizeof(float));
  if (xfitspline1==NULL) {
    sprintf(event,"Calloc of xfitspline1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  xspline=(float *)calloc(length,sizeof(float));
  if (xspline==NULL) {
    sprintf(event,"Calloc of xspline failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  nfit0=(int *)calloc(length,sizeof(float));
  if (nfit0==NULL) {
    sprintf(event,"Calloc of nfit0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  nfit1=(int *)calloc(length,sizeof(float));
  if (nfit1==NULL) {
    sprintf(event,"Calloc of nfit1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  overscanwminmax0=(float *)calloc(length,sizeof(float));
  if (overscanwminmax0==NULL) {
    sprintf(event,"Calloc of overscanwminmax0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  overscanwminmax1=(float *)calloc(length,sizeof(float));
  if (overscanwminmax1==NULL) {
    sprintf(event,"Calloc of overscanwminmax1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  vecsort0=(float *)calloc(length,sizeof(float));
  if (vecsort0==NULL) {
    sprintf(event,"Calloc of vecsort0 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  vecsort1=(float *)calloc(length,sizeof(float));
  if (vecsort1==NULL) {
    sprintf(event,"Calloc of vecsort1 failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }

  /* size (number of cols) of overscan, calculated using the header keywords */
  if (overscantrim >= 0) {
    width=data->biassecan[1] - data->biassecan[0] + 1 - 2*overscantrim;
  } else {
    printf ("overscantrim must be > 0, reseting it to 0\n");
    overscantrim = 0;
    width=data->biassecan[1] - data->biassecan[0] + 1 - 2*overscantrim;
  }

  if(flag_verbose){
    sprintf(event,"Entering OverScan processing");     
    reportevt(flag_verbose,STATUS,1,event);
  }
  /*
    Calculating Mean, Median or Mean with min/max rejection of 
    overscan for each amp 
  */
  if (overscansample == 0) { /* mean */
    sprintf(event,"Calculating overscan using Mean sample");
    reportevt(flag_verbose, STATUS, 1, event);
    
    yindo = overscantrim+data->biassecan[0];
    xindo = overscantrim+data->biassecbn[0];
    for (y = 0; y < length; y++) {
      yindex = (y+data->biassecan[2]-1)*data->axes[0];
      /* sum over overscan lines side A/B */
      sum0 = 0.0;
      sum1 = 0.0;
      for (x = 0; x < width; x++) {
	xindex = x - 1;
	sum0 = sum0 + data->image[yindex + xindex + yindo];
	sum1 = sum1 + data->image[yindex + xindex + xindo];
      }
      overscan0[y] = sum0/width;
      overscan1[y] = sum1/width;
    }
  }
  else if (overscansample == 1) { /*mean with min/max reject */
    sprintf(event,"Calculating overscan using Mean with Min Max Rejection");
    reportevt(flag_verbose,STATUS,1,event);
    
    yindo = overscantrim+data->biassecan[0];
    xindo = overscantrim+data->biassecbn[0];
    for (y = 0; y < length; y++) {
      yindex = (y+data->biassecan[2]-1)*data->axes[0];
      min0 = data->image[yindex+yindo-1];
      min1 = data->image[yindex+xindo-1];
      max0 = min0;
      max1 = min1;
      /* sum over overscan lines */
      sum0 = 0.0;
      sum1 = 0.0;
      value0 = 0;
      value1 = 0;
      for (x = 0; x < width; x++) {
	xindex = x-1;
	value0 = data->image[yindex+yindo+xindex];
	value1 = data->image[yindex+xindo+xindex];
	sum0 = sum0 + value0;
	sum1 = sum1 + value1;
	/* find min and max for each amplifier */
	if (value0 < min0) min0 = value0;
	if (value1 < min1) min1 = value1;
	if (value0 > max0) max0 = value0;
	if (value1 > max1) max1 = value1;
      }
      overscan0[y] = (sum0 - min0 - max0)/(width - 2.0);
      overscan1[y] = (sum1 - min1 - max1)/(width - 2.0);
    }
  }
  // BIASSECA= '[2105:2154,51:4146]' / Overscan from amp A
  // BIASSECB= '[7:56,51:4146]'     / Overscan from amp B
  else if (overscansample == -1) { /*median */
    sprintf(event,"Calculate overscan using Median sample");
    reportevt(flag_verbose, STATUS, 1, event);
    yindo = overscantrim + data->biassecan[0] -1;
    xindo = overscantrim + data->biassecbn[0] -1;
    for (y = 0; y < length; y++) {
      yindex = (y+data->biassecan[2]-1) * data->axes[0];
      /* copy overscan for current line into sorting vector */
      for (x = 0; x < width; x++) {
	vecsort0[x] = data->image[yindex+x+yindo];
	vecsort1[x] = data->image[yindex+x+xindo];
      }
      /* sort the vectors */
      shell(width, vecsort0 - 1);
      shell(width, vecsort1 - 1);
      /* get the median value */
      if (width % 2 == 0) {
	/* even number of data points in overscan */
	/* copy median value into the overscan vectors */
	pos = width/2 - 1;
	overscan0[y] = (vecsort0[pos] + vecsort0[pos+1])/2.0;
	overscan1[y] = (vecsort1[pos] + vecsort1[pos+1])/2.0;
      }
      else {
	/* odd number of data points in overscan */
	pos = (width + 1)/2 - 1;
	overscan0[y] = vecsort0[pos];
	overscan1[y] = vecsort1[pos];
      }
    }
  } /* end of calculations for mean, median and mean with min/max rejection */
   
  /* 
     create an array with the mean, median or mean w/ min/max rejection which 
     will be used to calculate the stddev and rms of the overscan 
  */
  for (i=0; i < length; i++) {
    overscanmeasured0[i] = overscan0[i];
    overscanmeasured1[i] = overscan1[i];
  }
   
  if (overscanfunction !=0 ) {
    /* 
       smooth the array using a bin size (boxcar) given by overscanfunction 
       absolute value 
    */
    nbin = abs(overscanfunction); /* bin size given by user. */
    /* number of data point in the array given by bin size nbin */
    nsample = length/nbin; 
    indexbin = 0;
    oline = 0; /* keep track of full overscan going with nbin steps */

    /* yfit0 and yfit1 contains the mean overscan value in the nbin */
    for (j = 0; j < nsample; j++) {
      yfit0[j] = 0.0;
      yfit1[j] = 0.0;
    }

    /* Build array with sum values of overscan for each bin */
    for (i = 0; i < length; i+=nbin) {
      sumoverscan0 = 0.0;
      sumoverscan1 = 0.0;
      for (j = 0; j < nbin; j++) {
	sumoverscan0 += overscan0[i + j];
	sumoverscan1 += overscan1[i + j];
	/* 
	   fill in the binned overscan array with mean values. Make sure 
	   not to sum values after the last bin until the last element of 
	   the overscan array. Maybe this can be improved summing the last 
	   few points after the last bin.
	*/
	if (j == (nbin - 1) && (i + nbin) < length) {
	  yfit0[indexbin] = sumoverscan0/(float) nbin;
	  yfit1[indexbin] = sumoverscan1/(float) nbin;
	  /*  
	      printf("i+nbin %d , indexbin %d, nbin %d, yfit0 %f, 
	      sumoverscan0 %f, overscan[i] %f, length %d\n", i+nbin,
	      indexbin,nbin,yfit0[indexbin-1],sumoverscan0,overscan0[j],
	      length); 
	  */
	  indexbin++;
	}
      }
    } /* end creating and populating smooth array with boxcar size nbin */
  }/* end of populating arrays with smooth function */
   

  /* doing spline fitting using a "boxcar of size N" */
  if (overscanfunction < 0) {
       
    sprintf(event,"Substracting overscan using cubic spline interpolation");
    reportevt(flag_verbose,STATUS,1,event); 
              
    /* array for second order derivative for Amp A */
    y2nd0 = (float *)calloc(nsample,sizeof(float));
    if (y2nd0==NULL) {
      sprintf(event,"Calloc of y2nd0 failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
      
    /* array for second order derivative for Amp B */
    y2nd1 = (float *)calloc(nsample,sizeof(float));
    if (y2nd1==NULL) {
      sprintf(event,"Calloc of y2nd1 failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
      
    /* 
       setting boundary points for spline to a high value: If yp1 and/or ypn 
       are equal to 1 × 10^30 or larger, the routine is signaled to set the 
       corresponding boundary condition for a natural spline, with zero second 
       derivative on that boundary.
    */
    yp1 = 1e30;
    ypn = 1e30;

    /* 
       For spline fitting, the xfitspline0 and xfitspline1 is an array from 0 
       to 1 with nsample points 
    */ 
    for (j = 0; j < nsample; j++) {
      xfitspline0[j] = 1.0 - ((float) nsample - (float) j)/(float) nsample;
      xfitspline1[j] = 1.0 - ((float) nsample - (float) j)/(float) nsample;
    }
      
    /* obtain second derivatives */
    spline(xfitspline0 - 1, yfit0 - 1, nsample, yp1, ypn, y2nd0 - 1);
    spline(xfitspline1 - 1, yfit1 - 1, nsample, yp1, ypn, y2nd1 - 1);
      
       
    for (i = 0; i < length; i++) {
      xspline[i] =   1.0 - ((float) length - (float) i) / (float) length;
      splint(xfitspline0 - 1, yfit0 - 1, y2nd0 - 1, nsample, 
	     xspline[i], &ysplinefit0);
      splint(xfitspline1 - 1, yfit1 - 1, y2nd1 - 1, nsample, 
	     xspline[i], &ysplinefit1);
      overscan0[i] = ysplinefit0;
      overscan1[i] = ysplinefit1;
      /* printf("%10d %10.3f %10.3f\n",i+1, overscan0[i], overscan1[i]);*/
    }
      
  }
  /* doing polynomial fitting using "boxcar of size N" */
  else if (overscanfunction > 0) {
      
    sprintf(event,"Substracting overscan using legendre interpolation");
    reportevt(flag_verbose,STATUS,1,event);               
      
    /* array with calculated coefficients */
    coeff0 = (float *)calloc(norder,sizeof(float));
    if (coeff0 == NULL) {
      sprintf(event,"Calloc of coeff0 failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
    /* array with calculated coefficients */
    coeff1 = (float *)calloc(norder,sizeof(float));
    if (coeff1 == NULL) {
      sprintf(event,"Calloc of coeff1 failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
    /*input coefficients. 1 mean want to calculate coeff */
    inputcoeff = calloc(norder,sizeof(int));
    if (inputcoeff == NULL) {
      sprintf(event,"Calloc of inputcoeff failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
    /* if inputcoeff = 1, then lfit calculates the coefficients*/
    for (i = 0; i < norder; i++) {
      inputcoeff[i] = 1;
    }
    /* stdev for each datapoint */
    sig = (float *)calloc(length,sizeof(float));
    if (sig == NULL) {
      sprintf(event,"Calloc of sig failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
    for (i = 0; i < nsample; i++) {
      sig[i] = 1.0;
    }
    covar = calloc(norder, sizeof(float *));
    if (covar == NULL) {
      sprintf(event,"Calloc of covar failed");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
    for (i = 0; i < norder; i++) {
      covar[i] = calloc(norder,sizeof(float));
      if (covar[i] == NULL) {
	sprintf(event,"Calloc of covar[%d] failed",i);
	reportevt(flag_verbose,STATUS,5,event);
	exit(1);
      }
    }
    /* 
       constructing arrays to input in legendre polynomial. 
       xfitleg needs to go from -1 to 1 
    */
    for (j = 0; j < nsample; j++) {      
      xfitleg0[j] = (2.0 * (float) j - (float) nsample)/(float) nsample;
      xfitleg1[j] = (2.0 * (float) j - (float) nsample)/(float) nsample;
    }

    /*polynomial fit of order norder (xfit0 and xfit1 go from -1 to 1)*/
    lfit(xfitleg0-1, yfit0-1, sig-1, nsample, coeff0-1, inputcoeff-1, 
	 norder, covar-1, &chisqr, fleg);
    lfit(xfitleg1-1, yfit1-1, sig-1, nsample, coeff1-1, inputcoeff-1, 
	 norder, covar-1, &chisqr, fleg);
      
    /* 
       Get the overscan values at each line using the polynomial fit 
       the values to evaluate are those from overscan0 and overscan1 
       for each line of the image, using the coefficients calculated 
       from lfit 
    */
      
    polcoef0 = (float *)calloc(norder,sizeof(float));
    polcoef1 = (float *)calloc(norder,sizeof(float));
    for (i = 0; i < length; i++) {
      x0 = (2.0 * (float) i - (float) length)/(float) length;
      x1 = (2.0 * (float) i - (float) length)/(float) length;
      overscan0[i] = 0.0;
      overscan1[i] = 0.0;
      fleg(x0, polcoef0, norder);
      fleg(x1, polcoef1, norder);
      for (j = 1; j <= norder; j++) {
	overscan0[i] += coeff0[j-1] * polcoef0[j];
	overscan1[i] += coeff1[j-1] * polcoef1[j];
      }
    }
  } /* end polynomial fitting */
   
  /* create a single image array that contains the trimmed data */
  output->axes[0] = 1 + data->trimsecn[1] - data->trimsecn[0];
  output->axes[1] = 1 + data->trimsecn[3] - data->trimsecn[2];
  output->nfound  = 2;
  output->npixels = output->axes[0] * output->axes[1];

  /* since it is likely this function is going to be called again,
     it is better to reuse image and mask memory rather than 
     free and malloc it each time. Just need to make sure the arrays are big
     enough for all cases: input->axes[0]*input->axes[1] */

  if (output->image == NULL)
    output->image=(float *)calloc(data->axes[0]*data->axes[1],sizeof(float));
  if (output->image==NULL) {
    sprintf(event,"Calloc of output->image failed");
    reportevt(flag_verbose,STATUS,5,event);
    exit(1);
  }
  if(data->mask != NULL){
    if (output->mask == NULL)
      output->mask = (short *)calloc(data->axes[0]*data->axes[1],sizeof(short));
    if(!output->mask){
      sprintf(event,"Allocation for trimmed mask failed.");
      reportevt(flag_verbose,STATUS,5,event);
      exit(1);
    }
  }
   
  if(flag_verbose){
    sprintf(event,"OverScan processing done, new image array:(%ldx%ld,%ld)",
	    output->axes[0],output->axes[1],output->npixels);
    reportevt(flag_verbose,STATUS,1,event);
    sprintf(event,"OverScan processing done, copying image data into place.");
    reportevt(flag_verbose,STATUS,1,event);
  }
  /* set parameters for the array */
  output->saturateA = data->saturateA;
  output->saturateB = data->saturateB;
  output->gainA     = data->gainA;
  output->gainB     = data->gainB;
  output->rdnoiseA  = data->rdnoiseA;
  output->rdnoiseB  = data->rdnoiseB;
   
  /* Amp  A/B flip checked  */
  if(data->ampsecan[0] < data->ampsecan[1]){
    xmin = data->ampsecan[0] + data->datasecn[0] - 1;
    xmax = data->ampsecan[1] + data->datasecn[0] - 1;
  }
  else{
    xmin = data->ampsecan[1] + data->datasecn[0] - 1;
    xmax = data->ampsecan[0] + data->datasecn[0] - 1;
  }
   
  if(flag_verbose){
    sprintf(event,"(1)");
    reportevt(flag_verbose,STATUS,1,event);
  }

  /* copy the image into the new data array with overscan correction */
  for (y = data->trimsecn[2]; y <= data->trimsecn[3]; y++) {
    // yindex for output image
    yindex = (y-data->trimsecn[2])*output->axes[0];
    // yindex for input image
    yindo  = (y-1)*data->axes[0];
    for (x = data->trimsecn[0]; x <= data->trimsecn[1]; x++) {
      // output x index
      xindex = x - data->trimsecn[0];
      if (x >= xmin && x <= xmax) /* then we are in AMPSECA */   {
	output->image[yindex + xindex] = 
	  data->image[yindo + x - 1] - overscan0[y - data->trimsecn[2]];
	if(data->mask)
	  output->mask[yindex + xindex] = data->mask[yindo + x - 1];
      }
      else /* assume we are in AMPSECB */  { 
	
	output->image[yindex + xindex] = 
	  data->image[yindo + x - 1] - overscan1[y - data->trimsecn[2]];
	if(data->mask)
	  output->mask[yindex + xindex] = data->mask[yindo + x - 1];
      }
    }
  }

  if(flag_verbose){
    sprintf(event,"OverScan data copy done, getting statistics.");
    reportevt(flag_verbose,STATUS,1,event);
  }
  /* Overscan statistics (standard deviation and Mean). */
  /* For the case of mean, median and mean with min/max, stddev = 0.0 */
  /* because we are calculating the std dev of the interpolated values */
  /* with repect the mean, median or mean with min/max */ 
  variance0 = 0;
  variance1 = 0;
  rmssum0 = 0;
  rmssum1 = 0;
  /* Calculate the sample stddev for fitted overscan*/
  for (i=0; i < length; i++) {
    variance0 += pow((overscanmeasured0[i] - overscan0[i]),2);
    variance1 += pow((overscanmeasured1[i] - overscan1[i]),2);
  }
  stddev0 = sqrt (variance0/(length - 1));
  stddev1 = sqrt (variance1/(length - 1));
  /* rmse for mean, median or mean with min/max rejection */
  for (i =0; i < length; i++) {
    rmssum0 += pow(overscanmeasured0[i],2);
    rmssum1 += pow(overscanmeasured1[i],2);
  }
  overscanrms0 = sqrt (rmssum0/length);
  overscanrms1 = sqrt (rmssum1/length);

  if(flag_verbose){
    sprintf(event,"STDDEV_A=%6.3f STDDEV_B=%6.3f MEAN_A=%6.3f MEAN_B=%6.3f",
	    stddev0,stddev1,overscanrms0,overscanrms1);
    if(osconfig.debug){
      reportevt(flag_verbose,STATUS,3,"OVERCSAN INDEX        MEASURED VALUES        FIT VALUES");
      for (i=0; i < length; i++) {
    	sprintf(event,"%d\t\t%6.3f\t%6.3f\t\t%6.3f\t%6.3f",i,overscanmeasured0[i],
    		overscanmeasured1[i],overscan0[i],overscan1[i]);
	reportevt(flag_verbose,STATUS,3,event);
      }
    }
  }

  /* memory cleanup */
  if (vecsort0 != NULL) free(vecsort0);
  if (vecsort1 != NULL) free(vecsort1);
  if (overscan0 != NULL) free(overscan0);
  if (overscan1 != NULL) free(overscan1);
  if (overscanwminmax0 != NULL) free(overscanwminmax0);
  if (overscanwminmax1 != NULL) free(overscanwminmax1);
  if (overscanmeasured0 != NULL) free(overscanmeasured0);
  if (overscanmeasured1 != NULL) free(overscanmeasured1);
  if (xspline != NULL) free(xspline);
  if (y2nd0 != NULL) free(y2nd0);
  if (y2nd1 != NULL) free(y2nd1);
  if (xfitspline0 != NULL) free(xfitspline0);
  if (xfitspline1 != NULL) free(xfitspline1);
/*  RAG 2012oct15 removed free for chisqr (was causing seg fault?)
/*  if (chisqr != NULL) free(chisqr); */
  if (coeff0 != NULL) free(coeff0);
  if (coeff1 != NULL) free(coeff1);
  if (sig != NULL) free(sig);
  if (polcoef0 != NULL) free(polcoef0);
  if (polcoef1 != NULL) free(polcoef1);
  if (xfitleg0 != NULL) free(xfitleg0);
  if (xfitleg1 != NULL) free(xfitleg1);
  if (xfit0 != NULL) free(xfit0);
  if (xfit1 != NULL) free(xfit1);
  if (yfit != NULL) free(yfit);
  if (yfit0 != NULL) free(yfit0);
  if (yfit1 != NULL) free(yfit1);
  if (inputcoeff != NULL) free(inputcoeff);
  if (nfit0 != NULL) free(nfit0);
  if (nfit1 != NULL) free(nfit1);
 
  if (covar != NULL) {
   for (i = 0; i < norder; i++)
      if (covar[i] != NULL) free(covar[i]);
   free(covar);
  }

} /* end overscan function */

void orient_section(int *section)
{
  int temp = 0;
  if(section[1] < section[0]) {
    temp = section[1];
    section[1] = section[0];
    section[0] = temp;
  }
  if(section[3] < section[2]){
    temp = section[3];
    section[3] = section[2];
    section[2] = temp;
  }
}

int column_in_section(int col,int *sec)
{
  int lx = sec[0];
  int ux = sec[1];
  int npix = ux - lx;
  if(npix < 0){
    npix = ux;
    ux = lx;
    lx = npix;
  }
  return((col >= lx) && (col <= ux)); 
}

/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */

#define ELEM_SWAP(a,b) { register float t=(a);(a)=(b);(b)=t; }

float quick_select(float *arr, int n)
{
    int low=0, high=n-1 ;
    int median = (low + high) / 2;
    int middle, ll, hh;

    //low = 0 ; high = n-1 ; median = (low + high) / 2;

    while (1)
    {
        if (high <= low) /* One element only */
            return arr[median];

        if (high == low + 1) {  /* Two elements only */
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]);
            return arr[median];
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
        if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
        if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP(arr[middle], arr[low+1]);

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;

        while (1)
        {
            do ll++; while (arr[low] > arr[ll]) ;
            do hh--; while (arr[hh]  > arr[low]) ;
            if (hh < ll) break;
            ELEM_SWAP(arr[ll], arr[hh]) ;
        }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP(arr[low], arr[hh]) ;

        /* Re-set active partition */
        if (hh <= median) low = ll;
        if (hh >= median) high = hh - 1;
    }
}

