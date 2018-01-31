/* *
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"

#include <algorithm>
#include <cstdlib>
#include <sys/time.h>
#include <omp.h>
#include <math.h>

#include "acl_cpp/lib_acl.hpp"

using namespace std;
using namespace cv;

void processUsingOpenCvCpu(std::string nput_file, std::string output_file, std::string name_file);

int main(int argc, char **argv) {

	 string input_file;
	 string name_file;

			if(argc <= 1 ){
				input_file="./data/simple_room-wallpaper-4096x3072.jpg";
				name_file="simple_room-wallpaper-4096x3072.jpg";
			}else{
//				input_file = "../data/" + argv[1];
				string dir = "../../imagesim/uploads/";
				input_file = dir.append(argv[1]);
				name_file = argv[1];
			}

			string output_file_OpenCvCpu;

			if(argc <= 1 ){
				output_file_OpenCvCpu="./data/output_OpenCvCpu.jpg";
			}else{
//				input_file = "../data/" + argv[1];
				string dir = "./n03970156/output/";
				output_file_OpenCvCpu = dir.append(argv[1]);
			}

	processUsingOpenCvCpu(input_file, output_file_OpenCvCpu, name_file);


	return 0;
}

double get_walltime() {
	struct timeval tp; gettimeofday(&tp, NULL);
	return (double) (tp.tv_sec + tp.tv_usec*1e-6);
}

template <typename Type>
Mat imHist(Mat hist, float scaleX=1, float scaleY=1){
	double maxVal=0;
	minMaxLoc(hist, 0, &maxVal, 0, 0);
	int rows = 64; //default height size
	int cols = hist.rows; //get the width size from the histogram
	Mat histImg = Mat::zeros(rows*scaleX, cols*scaleY, CV_8UC1);
	//for each bin
	for(int i=0;i<cols-1;i++) {
		Type histValue = hist.at<Type>(i,0);
		Type nextValue = hist.at<Type>(i+1,0);
		Point pt1 = Point(i*scaleX, rows*scaleY);
		Point pt2 = Point(i*scaleX+scaleX, rows*scaleY);
		Point pt3 = Point(i*scaleX+scaleX, (rows-nextValue*rows/maxVal)*scaleY);
		Point pt4 = Point(i*scaleX, (rows-nextValue*rows/maxVal)*scaleY);

		int numPts = 5;
		Point pts[] = {pt1, pt2, pt3, pt4, pt1};

		fillConvexPoly(histImg, pts, numPts, Scalar(255,255,255));
	}
	return histImg;
}



void hist_normalize(int *source, double* destination, int amount){

//	printf("amount %d\n",amount);

	for (int x = 0; x < 256; x++) {
		double depthValue = (double) source[x] / (double) amount;
		destination[x]=depthValue;
//		printf("int array x %d, normal value %.15f\n",source[x],depthValue);
//		printf("%.15f\n",depthValue);
//		printf("%d\n",source[x]);

	}
}

double mean (double *histogram){

	double sum=0;
	double size=256;

	for(int i = 0; i < size; i++){
	    sum += histogram[i]*i;
	}

	return sum/size;

}

double variance(double mean){

	return sqrt(mean);

}

double skweness (double *histogram, double mean, double variance){

	double sum=0;
	double size=256;

	for(int i = 0; i < size; i++){
	    sum += pow((histogram[i]-mean)/variance,3)*i;
	}

	return sum/size;

}

double kurtosis(double *histogram, double mean, double variance){

	double sum=0;
	double size=256;

	for(int i = 0; i < size; i++){
		sum += pow((histogram[i]-mean)/variance,4)*i;
	}

	//-3 for normal dist
	return (sum / size)-3;

}

double p_reduce_array(double *array, int num_threads){

	double sum;
	int size;
	size = 256;
	sum = 0;
	int i;

	double *reduce = (double*) malloc( num_threads * sizeof(double));
	int bins_per_thread = size/num_threads;
	int tid;

	#pragma omp parallel shared(array) private(i,tid) firstprivate(reduce) num_threads(num_threads)
	{
		tid = omp_get_thread_num();
		//printf("id %d from %d to %d\n", tid,bins_per_thread*tid, bins_per_thread*(tid+1));
		for(i = bins_per_thread*tid; i < bins_per_thread*(tid+1); i++){
			reduce[tid] += array[i];
			//printf("forloop tid %d reducearray id %d value %f\n", tid, i, array[i]);
		}
	}

	for(int i = 0; i < num_threads; i++){
		//printf("reduce id %d sum %d\n", i, reduce[i]);
		sum += reduce[i];
	}

	return sum/size;

}

/*Parallel function using openmp*/
double pfirst_movement(double *histogram){

	double sum, avg;
	int size;
	size = 256;
	sum = 0;
	avg = 0;
	int i;
	#pragma omp parallel shared(histogram) private(i) reduction(+:sum) num_threads(8)

		#pragma omp for schedule (dynamic)
			for(i = 0; i < size; i++){
				sum += histogram[i];
			}
			avg = sum / size;

	return avg;



}
/*Parallel function using openmp*/
double psecond_movement (double *histogram, double mean){

	double sum, variance;
	int size;
	size = 256;
	sum = 0;
	variance = 0;
	int i;
	#pragma omp parallel for shared(histogram,mean) private(i) reduction(+:sum)
	for(i = 0; i < size; i++){
	    sum += ((histogram[i]-mean)*(histogram[i]-mean));
	}
	variance = sum / size;

	return variance;

}


static bool test_hmset(acl::redis_hash& redis, std::string name_file, std::string color, double mean, double variance, double skweness, double kurtosis)
{

	acl::string __keypre("proc_");
	acl::string key, attr1, attr2, attr3, attr4, attr5;
	acl::string val1, val2, val3, val4, val5;
	std::map<acl::string, acl::string> attrs;

//	for (int i = 0; i < 5; i++)
//	{
		key.format("%s%s-%s", __keypre.c_str(),color.c_str(),name_file.c_str());

		attr1.format("name");
		attr2.format("mean");
		attr3.format("variance");
		attr4.format("skweness");
		attr5.format("kurtosis");

		val1.format("%s", name_file.c_str());
		val2.format("%.15f", mean);
		val3.format("%.15f", variance);
		val4.format("%.15f", skweness);
		val5.format("%.15f", kurtosis);

		attrs[attr1] = val1;
		attrs[attr2] = val2;
		attrs[attr3] = val3;
		attrs[attr4] = val4;
		attrs[attr5] = val5;

		redis.clear();
		if (redis.hmset(key.c_str(), attrs) == false)
		{
			printf("hmset error: %s, key: %s\r\n",
				redis.result_error(), key.c_str());
			return false;
		}
		else
		{
			printf("hmset ok, key: %s, %s=%s, %s=%s, %s=%s, %s=%s, %s=%s\r\n",
				key.c_str(),
				attr1.c_str(), val1.c_str(),
				attr2.c_str(), val2.c_str(),
				attr3.c_str(), val3.c_str(),
				attr4.c_str(), val4.c_str(),
				attr5.c_str(), val5.c_str());
		}
		attrs.clear();
//	}

	return true;
}

void processUsingOpenCvCpu(std::string input_file, std::string output_file, std::string name_file) {
	//Read input image from the disk
	Mat input = imread(input_file, CV_LOAD_IMAGE_COLOR);
	if(input.empty())
	{
		std::cout<<"Image Not Found: "<< input_file << std::endl;
		return;
	}

	//Create output image
	Mat output;

	//Hold the histogram
	Mat hist, histImg;
	int nbins = 256; // lets hold 256 levels
	int hsize[] = { nbins }; // just one dimension
	float range[] = { 0, 256 };
	const float *ranges[] = { range };
	int chnls[] = { 0 };

	// create colors channels
	vector<Mat> colors;
	split(input, colors);

//-----------------------
	//redis
	int  ch, n = 1, conn_timeout = 10, rw_timeout = 10;
	acl::string addr("127.0.0.1:6379"), cmd;
	bool slice_req = false, cluster_mode = false;

	acl::acl_cpp_init();
	acl::log::stdout_open(true);

	acl::redis_client_cluster cluster;
	cluster.set(addr.c_str(), 100, conn_timeout, rw_timeout);

	acl::redis_client client(addr.c_str(), conn_timeout, rw_timeout);
	client.set_slice_request(slice_req);

	acl::redis_hash redis;
	redis.set_client(&client);

	acl::redis_list redisList(&client);
	redisList.set_client(&client);

//-----------------------

	double mea, var, skw, kur;
	double *normal = (double*) malloc( 256 * sizeof(double));
	int *array = (int*) malloc( 256 * sizeof(int));
	std::vector<float> farray;

//-----------------------
	//blue
	calcHist(&colors[0], 1, chnls, Mat(), hist, 1, hsize, ranges);

	if (hist.isContinuous()) {
	  farray.assign((float*)hist.datastart, (float*)hist.dataend);
	} else {
	  for (int i = 0; i < hist.rows; ++i) {
		farray.insert(farray.end(), hist.ptr<float>(i), hist.ptr<float>(i)+hist.cols);
	  }
	}

	for (int x = 0; x < 256; x++){
		array[x] = (int)farray[x];
	}

	hist_normalize(array, normal, input.rows*input.cols);

	 mea = mean(normal);
	 var = variance(mea);
	 skw = skweness(normal, mea, var);
	 kur = kurtosis(normal, mea, var);

	// create an image out of the hist to be displayed
//	histImg = imHist<float>(hist, 3, 3);
//	imwrite(output_file, histImg);

	printf("blue %s,%.15f,%.15f,%.15f,%.15f\n",name_file.c_str(),mea,var,skw,kur);

	test_hmset(redis, name_file, "blue", mea, var, skw, kur);
//-----------------------
	//green
	calcHist(&colors[1], 1, chnls, Mat(), hist, 1, hsize, ranges);

	if (hist.isContinuous()) {
	  farray.assign((float*)hist.datastart, (float*)hist.dataend);
	} else {
	  for (int i = 0; i < hist.rows; ++i) {
	    farray.insert(farray.end(), hist.ptr<float>(i), hist.ptr<float>(i)+hist.cols);
	  }
	}

	for (int x = 0; x < 256; x++){
		array[x] = (int)farray[x];
	}

	hist_normalize(array, normal, input.rows*input.cols);

	 mea = mean(normal);
	 var = variance(mea);
	 skw = skweness(normal, mea, var);
	 kur = kurtosis(normal, mea, var);

	// create an image out of the hist to be displayed
//	histImg = imHist<float>(hist, 3, 3);
//	imwrite(output_file, histImg);

	printf("green %s,%.15f,%.15f,%.15f,%.15f\n",name_file.c_str(),mea,var,skw,kur);

	test_hmset(redis, name_file, "green", mea, var, skw, kur);

//-----------------------
	//red
	calcHist(&colors[2], 1, chnls, Mat(), hist, 1, hsize, ranges);

	if (hist.isContinuous()) {
	  farray.assign((float*)hist.datastart, (float*)hist.dataend);
	} else {
	  for (int i = 0; i < hist.rows; ++i) {
	    farray.insert(farray.end(), hist.ptr<float>(i), hist.ptr<float>(i)+hist.cols);
	  }
	}

	for (int x = 0; x < 256; x++){
		array[x] = (int)farray[x];
	}

	hist_normalize(array, normal, input.rows*input.cols);

	 mea = mean(normal);
	 var = variance(mea);
	 skw = skweness(normal, mea, var);
	 kur = kurtosis(normal, mea, var);

	// create an image out of the hist to be displayed
//	histImg = imHist<float>(hist, 3, 3);
//	imwrite(output_file, histImg);

	printf("red %s,%.15f,%.15f,%.15f,%.15f\n",name_file.c_str(),mea,var,skw,kur);

	test_hmset(redis, name_file, "red", mea, var, skw, kur);


	//-----------------------

	acl::string value;

	const char *__key = "processed";
	value.format("%s_%s", __key, name_file.c_str());
	redis.clear();
	int ret = redisList.lpush(__key, value.c_str(), NULL);
	if (ret <= 0)
	{
		printf("lpush key: %s error: %s, ret: %d\r\n",
			__key, redis.result_error(), ret);
//		return false;
	}
	else {
//		if (i < max_print_line)
		printf("lpush ok, key:%s, value:%s, ret: %d\r\n",
			__key, value.c_str(), ret);
	}

}

