#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <cstdlib>
#include <sys/time.h>
#include <omp.h>
#include <math.h>
#include "omp.h"

double mean,skew,kurtosis,variance,pmean,pskew,pkurtosis,pvariance;

double get_walltime() {
struct timeval tp; gettimeofday(&tp, NULL);
return (double) (tp.tv_sec + tp.tv_usec*1e-6); }


double parallel_moments(double *histogram){

	double sum;
        int size;
        size =4096;
        sum = 0;
	pskew = 0;
	pvariance = 0;
	pkurtosis=0;

        int i;
	
        #pragma omp parallel for shared(histogram) private(i) reduction(+:sum) 
	for(i = 0; i < size; i++){
          sum += histogram[i]*i;
            }
	      
        pmean = sum / size;

	
	sum=0;
	pvariance = sqrt(pmean);

	#pragma omp parallel for shared(histogram) private(i) reduction(+:sum) 
        for(i = 0; i < size; i++){
            sum += (pow((histogram[i]-pmean)/pvariance,3)*i);
        }
	
	pskew= sum/size;
	

	sum=0;
	#pragma omp parallel for shared(histogram,mean,variance) private(i) reduction(+:sum)
	for(i = 0; i < size; i++){
 	        sum += pow((histogram[i]-pmean)/pvariance,4)*i;
		}
		
   			    
	pkurtosis = ((sum / size)-3); //-3 for normal dist
	

return 1.5;

}

double moments(double *histogram){

	double sum;
        int size;
        size =4096;
        sum = 0;
	skew = 0;
	variance = 0;
	kurtosis=0;
        int i;
      
	for(i = 0; i < size; i++){
          sum += histogram[i]*i;
            }
	      
        mean = sum / size;

	
	
	variance = sqrt(mean);

	sum=0;

        for(i = 0; i < size; i++){
            sum += (pow((histogram[i]-mean)/variance,3)*i);
        }
	
	skew= sum/size;
	

	sum=0;
	for(i = 0; i < size; i++){
 	        sum += pow((histogram[i]-mean)/variance,4)*i;
		}
		
   		//-3 for normal dist
		
	    
	kurtosis = ((sum / size)-3);
	

	
return 1.5;
 

}



int main(){

double *histogram;
double  time0 , time1,avg,avg_parallel,v,vp,sk,psk,k,pk;
int i;
double size =4096;



histogram = (double *) malloc(size*sizeof(double));

for (i=0; i<size; i++)
histogram[i] = rand()/100000000;




//=========Prueba============//

time0 = get_walltime();
sk = moments(histogram); 
time1 = get_walltime();

double seqtime = time1-time0;



time0 = get_walltime();
psk = parallel_moments(histogram); 
time1 = get_walltime();

double paralleltime = time1-time0;




printf("\nElapsed sequential time || Elapsed parallel time\n");
printf("==================================================\n");
printf("%f\t\t\t%f\n",seqtime,paralleltime);

printf("         Sequential Calculations || \tParallel Calculations\n");
printf("         =========================================================\n");
printf("Mean \t\t%0.2lf\t\t\t%0.2lf\n",mean,pmean);
printf("Variance \t%0.2lf\t\t\t\t%0.2lf\n",variance,pvariance);
printf("Skewness \t%0.2lf\t\t\t%0.2lf\n",skew,pskew);
printf("Kurtosis \t%0.2lf\t\t\t%0.2lf\n",kurtosis,pkurtosis);




	
return 0;
}




