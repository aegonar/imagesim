
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

using namespace cv;
using namespace std;

// static void help()
// {
//     cout << "\nThis program demonstrates kmeans clustering.\n"
//             "It generates an image with random points, then assigns a random number of cluster\n"
//             "centers and uses kmeans to move those cluster centers to their representitive location\n"
//             "Call\n"
//             "./kmeans\n" << endl;
// }

float getValues(std::string color, std::string currentResults, acl::redis_hash redis_hash){

					acl::string __keypre("proc_");
					acl::string key, attr1, attr2, attr3, attr4, attr5;
	//				acl::string val1, val2, val3, val4, val5;
	//				std::map<acl::string, acl::string> attrs;

					const char* attrs[5];
					std::vector<acl::string> mresult;

	//				key.format("%sblue-%s", __keypre.c_str(),(*citr).c_str());

					key.format("%s%s-%s",__keypre.c_str(), color.c_str(), currentResults.c_str());

					attr1.format("name");
					attr2.format("mean");
					attr3.format("variance");
					attr4.format("skweness");
					attr5.format("kurtosis");

					attrs[0] = attr1.c_str();
					attrs[1] = attr2.c_str();
					attrs[2] = attr3.c_str();
					attrs[3] = attr4.c_str();
					attrs[4] = attr5.c_str();

					mresult.clear();
					redis_hash.clear();

					if (redis_hash.hmget(key, attrs, 5, &mresult) == false)
					{
						printf("hmget error: %s\r\n", redis_hash.result_error());
						return false;
					}
	//				else if (i >= 10)
	//					continue;

					size_t size = redis_hash.result_size();
//					printf("size: %lu, key: %s\r\n", (unsigned long) size, key.c_str());

					size_t j;
//					for (j = 0; j < size; j++)
//					{
//						const char* val = redis_hash.result_value(j);
//						printf("hmget ok, %s=%s\r\n", attrs[j], val ? val : "null");
//					}

					std::string mean;
					std::vector<acl::string>::const_iterator it= mresult.begin();
					for (j = 0; it != mresult.end(); ++it, j++){
//						printf("hmget %s=%s\r\n", attrs[j], (*it).c_str());

						if(j==1){
							mean = (*it).c_str();
						}
					}

					mresult.clear();
					redis_hash.clear();


					double dMean = strtod(mean.c_str(), NULL);
					dMean*=1000;
					return (float) dMean;

}

std::vector <int>  findCommon(int ar1[], int ar2[], int ar3[], int n1, int n2, int n3);

int main( int argc, char** argv )
{

	string target_file;

	if(argc <= 1 ){
		target_file="simple_room-wallpaper-4096x3072.jpg";
	}else{
		target_file = argv[1];
	}

	printf("target_file : %s\r\n", target_file.c_str());

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

		acl::redis_list redis;
		redis.set_client(&client);

		acl::redis_hash redis_hash;
		redis_hash.set_client(&client);

	//-----------------------

		const char *__key = "processed";

		redis.clear();
		int totalPictures = redis.llen(__key);
		if (totalPictures < 0){
			printf("llen key:%s error: %s\r\n", __key, redis.result_error());
			return false;
		}else{
			printf("llen key:%s ret : %d \r\n", __key, totalPictures);
		}

	//-----------------------

		std::vector<acl::string> result;

		redis.clear();
		bool ret = redis.lrange(__key, 0, totalPictures, &result);
		if (ret == false)
		{
			printf("lrang key: %s error: %s\r\n",
				__key, redis.result_error());
			return false;
		}

		std::vector<acl::string>::const_iterator citr = result.begin();
//		int i = 0;
//		printf("lrang key: %s result:\r\n",__key);


		//-----------------------

		vector <string> names;

		float *blue = (float*) malloc( totalPictures * sizeof(float));
		float *green = (float*) malloc( totalPictures * sizeof(float));
		float *red = (float*) malloc( totalPictures * sizeof(float));

		//-----------------------
		int count=0;
		int TARGET_INDEX=-1;

		for (;citr != result.end(); ++citr)
		{
			std::string currentResults= (*citr).c_str();
//			printf("%d %s, ",count,currentResults.c_str());

				//-----------------------
				//blue
				acl::string __keypre("proc_");
				acl::string key, attr1, attr2, attr3, attr4, attr5;

				const char* attrs[5];
				std::vector<acl::string> mresult;

				key.format("%sblue-%s",__keypre.c_str(), (*citr).c_str());

				attr1.format("name");
				attr2.format("mean");
				attr3.format("variance");
				attr4.format("skweness");
				attr5.format("kurtosis");

				attrs[0] = attr1.c_str();
				attrs[1] = attr2.c_str();
				attrs[2] = attr3.c_str();
				attrs[3] = attr4.c_str();
				attrs[4] = attr5.c_str();

				mresult.clear();
				redis_hash.clear();

				if (redis_hash.hmget(key, attrs, 5, &mresult) == false)
				{
					printf("hmget error: %s\r\n", redis_hash.result_error());
					return false;
				}

				std::string mean, name;
				std::vector<acl::string>::const_iterator it= mresult.begin();
				for (size_t j = 0; it != mresult.end(); ++it, j++){
					if(j==0){
						name = (*it).c_str();
						names.push_back(name);
						if (name.compare(target_file) == 0){
//							printf("target index found for: %s. %d, \n",name.c_str(),count);
							TARGET_INDEX=count;
						}
					}else if(j==1){
						mean = (*it).c_str();
					}
				}

				double dMean = strtod(mean.c_str(), NULL);
				dMean*=1000;


				blue[count]=dMean;

//				printf("blue: %f, ",blue[count]);

				//-----------------------
				//green
				key.format("%sgreen-%s",__keypre.c_str(), (*citr).c_str());

				attr1.format("name");
				attr2.format("mean");
				attr3.format("variance");
				attr4.format("skweness");
				attr5.format("kurtosis");

				attrs[0] = attr1.c_str();
				attrs[1] = attr2.c_str();
				attrs[2] = attr3.c_str();
				attrs[3] = attr4.c_str();
				attrs[4] = attr5.c_str();

				mresult.clear();
				redis_hash.clear();

				if (redis_hash.hmget(key, attrs, 5, &mresult) == false)
				{
					printf("hmget error: %s\r\n", redis_hash.result_error());
					return false;
				}

//				std::string mean;
//				std::vector<acl::string>::const_iterator
				it= mresult.begin();
				for (size_t j = 0; it != mresult.end(); ++it, j++){
					if(j==1){
						mean = (*it).c_str();
					}
				}

				dMean = strtod(mean.c_str(), NULL);
				dMean*=1000;

				green[count]=dMean;

//				printf("green: %f, ",green[count]);

				//-----------------------
				//red
				key.format("%sred-%s",__keypre.c_str(), (*citr).c_str());

				attr1.format("name");
				attr2.format("mean");
				attr3.format("variance");
				attr4.format("skweness");
				attr5.format("kurtosis");

				attrs[0] = attr1.c_str();
				attrs[1] = attr2.c_str();
				attrs[2] = attr3.c_str();
				attrs[3] = attr4.c_str();
				attrs[4] = attr5.c_str();

				mresult.clear();
				redis_hash.clear();

				if (redis_hash.hmget(key, attrs, 5, &mresult) == false)
				{
					printf("hmget error: %s\r\n", redis_hash.result_error());
					return false;
				}

//				std::string mean;
//				std::vector<acl::string>::const_iterator
				it= mresult.begin();
				for (size_t j = 0; it != mresult.end(); ++it, j++){
					if(j==1){
						mean = (*it).c_str();
					}
				}

				dMean = strtod(mean.c_str(), NULL);
				dMean*=1000;

				red[count]=dMean;

//				printf("red %f\n",red[count]);

				count++;

		}

		printf("Target index found for: %s. %d\n",target_file.c_str(),TARGET_INDEX);
	//-----------------------

    const int MAX_CLUSTERS = 9;
    Scalar colorTab[] =
    {
        Scalar(0,0,255),
        Scalar(0,255,0),
		Scalar(255,0,0),

        Scalar(255,127,127),
		Scalar(127,127,255),
		Scalar(127,255,127),

        Scalar(255,255,0),
        Scalar(255,0,255),
		Scalar(0,255,255)

    };

    Mat img(1000, 1000, CV_8UC3);
//    RNG rng(12345);

//    for(;;)
//    {
        int k, clusterCount = 9;
        int i, sampleCount = totalPictures;

        Mat pointsBG(sampleCount, 1, CV_32FC2), labelsBG;
        Mat pointsGR(sampleCount, 1, CV_32FC2), labelsGR;
        Mat pointsRB(sampleCount, 1, CV_32FC2), labelsRB;

        clusterCount = MIN(clusterCount, sampleCount);
        std::vector<Point2f> BG_centers;
        std::vector<Point2f> GR_centers;
        std::vector<Point2f> RB_centers;

		for( i = 0; i < sampleCount; i++ )
		{

			Point sample;
		//	sample.x = data[i][0];
		//	sample.y = data[i][1];

			sample.x = blue[i];
			sample.y = green[i];

			pointsBG.at<Point2f>(i)=sample;

			sample.x = green[i];
			sample.y = red[i];

			pointsGR.at<Point2f>(i)=sample;

			sample.x = red[i];
			sample.y = blue[i];

			pointsRB.at<Point2f>(i)=sample;

		}

		//-----------------------

        double GR_compactness = kmeans(pointsGR, clusterCount, labelsGR,
            TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 15, 1.0),
               5, KMEANS_PP_CENTERS, GR_centers);


        img = Scalar::all(0);

        for( i = 0; i < sampleCount; i++ )
        {
            int clusterIdx = labelsGR.at<int>(i);
            Point ipt = pointsGR.at<Point2f>(i);
//            printf("colorid %d, point %d,%d\n", clusterIdx, ipt.x,ipt.y);
            circle( img, ipt, 2, colorTab[clusterIdx], FILLED, LINE_AA );
            if(i==TARGET_INDEX){
            	circle( img, ipt, 8, colorTab[clusterIdx], FILLED, LINE_AA );
            }
        }

        //get cluster for target
        int GR_TARGET_CLUSTER = labelsGR.at<int>(TARGET_INDEX);

        int GR_cluster_totals[9];
        for( i = 0; i < 9; i++ ){GR_cluster_totals[i]=0;}

        	//find cluster sizes
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsGR.at<int>(i);
				GR_cluster_totals[clusterIdx]++;
			}


			//create result array that contains indexes of the samples in the same cluster as target
	        int GR_results[GR_cluster_totals[GR_TARGET_CLUSTER]];
	        for( i = 0; i < GR_cluster_totals[GR_TARGET_CLUSTER]; i++ ){GR_results[i]=0;}

//	        printf("Target Cluster: %d, count: %d\n", GR_TARGET_CLUSTER, GR_cluster_totals[GR_TARGET_CLUSTER]);

	        int tmp=0;
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsGR.at<int>(i);
				if(clusterIdx == GR_TARGET_CLUSTER){
					GR_results[tmp]=i;
//					printf("%d Indexes of target cluster %d name: %s\n", tmp,i,names[i].c_str());
					tmp++;
				}
			}

        for( i = 0; i < 9; i++ ){printf("GR Cluster id %d, count %d\n", i, GR_cluster_totals[i]);}

//        printf("Target Cluster: %d, count: %d\n", GR_TARGET_CLUSTER, GR_cluster_totals[GR_TARGET_CLUSTER]);

			for (i = 0; i < (int)GR_centers.size(); ++i)
			{
				Point2f c = GR_centers[i];
				circle( img, c, 40, colorTab[i], 1, LINE_AA );
			}

        cout << "Compactness: " << GR_compactness << endl;


//        imshow("clusters", img);
//
//        char key = (char)waitKey();
//        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
//            break;

//        }

    	//-----------------------

        double BG_compactness = kmeans(pointsBG, clusterCount, labelsBG,
            TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 15, 1.0),
               5, KMEANS_PP_CENTERS, BG_centers);


        img = Scalar::all(0);

        for( i = 0; i < sampleCount; i++ )
        {
            int clusterIdx = labelsBG.at<int>(i);
            Point ipt = pointsBG.at<Point2f>(i);
//            printf("%d colorid %d, point %d,%d\n",i, clusterIdx, ipt.x,ipt.y);
            circle( img, ipt, 2, colorTab[clusterIdx], FILLED, LINE_AA );
            if(i==TARGET_INDEX){
            	circle( img, ipt, 8, colorTab[clusterIdx], FILLED, LINE_AA );
            }
        }

        //get cluster for target
        int BG_TARGET_CLUSTER = labelsBG.at<int>(TARGET_INDEX);

//        printf("Target index %d\n",TARGET_INDEX);

        int BG_cluster_totals[9];
        for( i = 0; i < 9; i++ ){BG_cluster_totals[i]=0;}

        	//find cluster sizes
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsBG.at<int>(i);
				BG_cluster_totals[clusterIdx]++;
			}


			//create result array that contains indexes of the samples in the same cluster as target
	        int BG_results[BG_cluster_totals[BG_TARGET_CLUSTER]];
	        for( i = 0; i < BG_cluster_totals[BG_TARGET_CLUSTER]; i++ ){BG_results[i]=0;}

//	        printf("Target Cluster: %d, count: %d\n", BG_TARGET_CLUSTER, BG_cluster_totals[BG_TARGET_CLUSTER]);

	        tmp=0;
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsBG.at<int>(i);
				if(clusterIdx == BG_TARGET_CLUSTER){
					BG_results[tmp]=i;
//					printf("%d Indexes of target cluster %d name: %s\n", tmp,i,names[i].c_str());
					tmp++;
				}
			}

        for( i = 0; i < 9; i++ ){printf("BG Cluster id %d, count %d\n", i, BG_cluster_totals[i]);}
//        printf("Target Cluster: %d, count: %d\n", BG_TARGET_CLUSTER, BG_results[BG_TARGET_CLUSTER]);

			for (i = 0; i < (int)BG_centers.size(); ++i)
			{
				Point2f c = BG_centers[i];
				circle( img, c, 40, colorTab[i], 1, LINE_AA );
			}

        cout << "Compactness: " << BG_compactness << endl;


//        imshow("clusters", img);
//
//        key = (char)waitKey();
//        if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
//            break;

        //-----------------------

		double RB_compactness = kmeans(pointsRB, clusterCount, labelsRB,
			TermCriteria( TermCriteria::EPS+TermCriteria::COUNT, 15, 1.0),
			   5, KMEANS_PP_CENTERS, RB_centers);


		img = Scalar::all(0);

		for( i = 0; i < sampleCount; i++ )
		{
			int clusterIdx = labelsRB.at<int>(i);
			Point ipt = pointsRB.at<Point2f>(i);
//            printf("%d colorid %d, point %d,%d\n",i, clusterIdx, ipt.x,ipt.y);
			circle( img, ipt, 2, colorTab[clusterIdx], FILLED, LINE_AA );
			if(i==TARGET_INDEX){
				circle( img, ipt, 8, colorTab[clusterIdx], FILLED, LINE_AA );
			}
		}

		//get cluster for target
		int RB_TARGET_CLUSTER = labelsRB.at<int>(TARGET_INDEX);

//        printf("Target index %d\n",TARGET_INDEX);

		int RB_cluster_totals[9];
		for( i = 0; i < 9; i++ ){RB_cluster_totals[i]=0;}

			//find cluster sizes
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsRB.at<int>(i);
				RB_cluster_totals[clusterIdx]++;
			}


			//create result array that contains indexes of the samples in the same cluster as target
			int RB_results[RB_cluster_totals[RB_TARGET_CLUSTER]];
			for( i = 0; i < RB_cluster_totals[RB_TARGET_CLUSTER]; i++ ){RB_results[i]=0;}

//			printf("Target Cluster: %d, count: %d\n", RB_TARGET_CLUSTER, RB_cluster_totals[RB_TARGET_CLUSTER]);

			tmp=0;
			for( i = 0; i < sampleCount; i++ ){
				int clusterIdx = labelsRB.at<int>(i);
				if(clusterIdx == RB_TARGET_CLUSTER){
					RB_results[tmp]=i;
//					printf("%d Indexes of target cluster %d name: %s\n", tmp,i,names[i].c_str());
					tmp++;
				}
			}

		for( i = 0; i < 9; i++ ){printf("RB Cluster id %d, count %d\n", i, RB_cluster_totals[i]);}
//        printf("Target Cluster: %d, count: %d\n", RB_TARGET_CLUSTER, RB_results[RB_TARGET_CLUSTER]);

			for (i = 0; i < (int)RB_centers.size(); ++i)
			{
				Point2f c = RB_centers[i];
				circle( img, c, 40, colorTab[i], 1, LINE_AA );
			}

		cout << "Compactness: " << RB_compactness << endl;


//		imshow("clusters", img);
//
//		key = (char)waitKey();
//		if( key == 27 || key == 'q' || key == 'Q' ) // 'ESC'
//			break;


		acl::redis_key redis_key;
		redis_key.set_client(&client);

		acl::string keyk;
		keyk.format("%s", target_file.c_str());
		redis.clear();
		int retk = redis_key.del_one(keyk.c_str());
		if (retk < 0)
		{
			printf("del key: %s error: %s\r\n",
					keyk.c_str(), redis_key.result_error());
//			return false;
		}
		else {
			printf("del ok, key: %s, ret: %d\r\n", keyk.c_str(), retk);

		}

		std::vector <int> common = findCommon(GR_results,BG_results,RB_results,GR_cluster_totals[GR_TARGET_CLUSTER],BG_cluster_totals[BG_TARGET_CLUSTER],RB_cluster_totals[RB_TARGET_CLUSTER]);
		int iti = 0;
		vector<int>::iterator it;
		for(it = common.begin(); it != common.end(); ++it, iti++)    {
			printf("Common %d pos %d name %s\n",iti, *it, names[*it].c_str());


			acl::string value;
			//			const char *__key = target_file.c_str();

			char* __key = new char[target_file.length()+1];
			strcpy(__key,target_file.c_str());

			value.format("%s",names[*it].c_str());
			redis.clear();
			int retL = redis.lpush(__key, value.c_str(), NULL);
			if (retL <= 0)
			{
				printf("lpush key: %s error: %s, ret: %d\r\n",
						__key, redis.result_error(), retL);
			}
			else {
		//		printf("lpush ok, key:%s, value:%s, ret: %d\r\n",
		//			__key, value.c_str(), ret);
			}
		}

//    }



    return 0;
}

std::vector <int> findCommon(int ar1[], int ar2[], int ar3[], int n1, int n2, int n3) {
    // Initialize starting indexes for ar1[], ar2[] and ar3[]
    int i = 0, j = 0, k = 0;
    vector <int> positions;
    // Iterate through three arrays while all arrays have elements
    while (i < n1 && j < n2 && k < n3)
    {
         // If x = y and y = z, print any of them and move ahead
         // in all arrays
         if (ar1[i] == ar2[j] && ar2[j] == ar3[k])
         {
//        	 cout << ar1[i] << " ";
//        	 printf("%d\n",ar1[i]);
        	 positions.push_back(ar1[i]);
        	 i++; j++; k++; }

         // x < y
         else if (ar1[i] < ar2[j])
             i++;

         // y < z
         else if (ar2[j] < ar3[k])
             j++;

         // We reach here when x > y and z < y, i.e., z is smallest
         else
             k++;
    }
    return positions;
}
