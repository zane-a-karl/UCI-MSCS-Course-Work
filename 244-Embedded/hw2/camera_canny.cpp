/**
 */
#include <stdlib.h>
#include <sys/time.h>

#include <ctime>
#include <iostream>
#include <unistd.h>
#include "opencv2/opencv.hpp"
#include "canny_util.h"

using namespace std;
using namespace cv;

/* Possible options: 320x240, 640x480, 1024x768, 1280x1040, and so on. */
/* Pi Camera MAX resolution: 2592x1944 */

#define WIDTH 640
#define HEIGHT 480
#define NFRAME 1.0

int main(int argc, char **argv)
{
	char* dirfilename;        /* Name of the output gradient direction image */
	char outfilename[128];    /* Name of the output "edge" image */
	char composedfname[128];  /* Name of the output "direction" image */
	unsigned char **images;     /* The input image */
	unsigned char *edge;      /* The output edge image */
	int rows, cols;           /* The dimensions of the image. */
	float sigma,              /* Standard deviation of the gaussian kernel. */
		tlow,               /* Fraction of the high threshold in hysteresis. */
		thigh;              /* High hysteresis threshold control. The actual
													 threshold is the (100 * thigh) percentage point
													 in the histogram of the magnitude of the
													 gradient image that passes non-maximal
													 suppression. */
	int n_imgs; /* Number of images to process */

	/****************************************************************************
   * Get the command line arguments.
   ****************************************************************************/
	if(argc < 4){
		fprintf(stderr,"\n<USAGE> %s sigma tlow thigh n_imgs [writedirim]\n",argv[0]);
		fprintf(stderr,"      sigma:      Standard deviation of the gaussian");
		fprintf(stderr," blur kernel.\n");
		fprintf(stderr,"      tlow:       Fraction (0.0-1.0) of the high ");
		fprintf(stderr,"edge strength threshold.\n");
		fprintf(stderr,"      thigh:      Fraction (0.0-1.0) of the distribution");
		fprintf(stderr," of non-zero edge\n                  strengths for ");
		fprintf(stderr,"hysteresis. The fraction is used to compute\n");
		fprintf(stderr,"                  the high edge strength threshold.\n");
		fprintf(stderr,"      n_imgs:     Positive Integer.\n");
		fprintf(stderr,"      writedirim: Optional argument to output ");
		fprintf(stderr,"a floating point");
		fprintf(stderr," direction image.\n\n");
		exit(1);
	}

	sigma = atof(argv[1]);
	tlow = atof(argv[2]);
	thigh = atof(argv[3]);
	n_imgs = atoi(argv[4]);
	rows = HEIGHT;
	cols = WIDTH;

	if(argc == 6) dirfilename = (char *) "dummy";
	else dirfilename = NULL;

	VideoCapture cap;
	// open the default camera (/dev/video0)
	// Check VideoCapture documentation for more details
	if(!cap.open(0)){
		cout<<"Failed to open /dev/video0"<<endl;
		return 0;
	}
	cap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
	cap.set(CAP_PROP_FRAME_HEIGHT,HEIGHT);

	//	Mat frame[], grayframe;
	Mat *frame = (Mat *)calloc(n_imgs, sizeof(*frame));
	Mat *grayframe = (Mat *)calloc(n_imgs, sizeof(*grayframe));
	images =  (unsigned char **)calloc(n_imgs, sizeof(unsigned char *));
	for(int i = 0; i < n_imgs; ++i)
		{
			images[i] =  (unsigned char *)calloc(1, sizeof(unsigned char));
		}

	double avg_cpu_proc_time = 0.0, avg_wall_proc_time;
	char *frame_title = (char *)calloc(256, sizeof(char));

	//	printf("[INFO] (On the pop-up window) Press ESC to start Canny edge detection...\n");
	printf("[INFO] Beginning image capture...\n");
	// for(int i = 0; i < n_imgs; ++i)
	// 	{
	// 		cap >> frame[i];
	//  		if( frame[i].empty() ) break; // end of video stream
	// 		sprintf(frame_title, "[RAW] frame%03d", i);
	// 		imshow(frame_title, frame[i]);
	//  		if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
	// 	}

	clock_t begin, mid, end;
	double time_elapsed, time_capture, time_process;
	struct timeval begin_tv, mid_tv, end_tv;
	double time_elapsed_wall, time_capture_wall, time_process_wall;
	/* has members
		 time_t       tv_sec;   seconds since Jan. 1, 1970 
		 suseconds_t  tv_usec;  and microseconds 
	*/

	//capture
	for(int i = 0; i < n_imgs; ++i) {
		begin = clock();// START
		if ( 0 != gettimeofday(&begin_tv, NULL) ) {
			perror("Trouble with begin_tv");
			exit(errno);
		}

		cap >> frame[i];
		if( frame[i].empty() ) break; // end of video stream
		mid = clock();// MIDDLE
		if ( 0 != gettimeofday(&mid_tv, NULL) ) {
			perror("Trouble with mid_tv");
			exit(errno);
		}

		sprintf(frame_title, "[RAW] frame%03d", i);
		imshow(frame_title, frame[i]);

		cvtColor(frame[i], grayframe[i], COLOR_BGR2GRAY);
		images[i] = grayframe[i].data;


		/****************************************************************************
		 * Perform the edge detection. All of the work takes place here.
		 ****************************************************************************/
		if(VERBOSE) printf("Starting Canny edge detection.\n");
		if(dirfilename != NULL){
			sprintf(composedfname, "camera_s_%3.2f_l_%3.2f_h_%3.2f.fim",
							sigma, tlow, thigh);
			dirfilename = composedfname;
		}
		canny(images[i], rows, cols, sigma, tlow, thigh, &edge, dirfilename);

		/****************************************************************************
		 * Write out the edge image to a file.
		 ****************************************************************************/
		//			sprintf(outfilename, "camera_s_%3.2f_l_%3.2f_h_%3.2f.pgm", sigma, tlow, thigh);
		sprintf(outfilename, "frame%03d_%3.2f_l_%3.2f_h_%3.2f.pgm", i, sigma, tlow, thigh);
		if(VERBOSE) printf("Writing the edge iname in the file %s.\n", outfilename);
		if(write_pgm_image(outfilename, edge, rows, cols, NULL, 255) == 0){
			fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
			exit(1);
		}

		end = clock();//FINISH
		if ( 0 != gettimeofday(&end_tv, NULL) ) {
			perror("Trouble with end_tv");
			exit(errno);
		}
		
		time_elapsed = (double) (end - begin) / CLOCKS_PER_SEC;
		time_capture = (double) (mid - begin) / CLOCKS_PER_SEC;
		time_process = (double) (end - mid)   / CLOCKS_PER_SEC;

		time_elapsed_wall = (double) (end_tv.tv_sec - begin_tv.tv_sec);
		time_capture_wall = (double) (mid_tv.tv_sec - begin_tv.tv_sec);
		time_process_wall = (double) (end_tv.tv_sec - mid_tv.tv_sec);


		avg_cpu_proc_time  += time_process;
		avg_wall_proc_time += time_process_wall;

		imshow("[GRAYSCALE] this is you, smile! :)", grayframe[i]);

		printf("Elapsed time for capturing+processing one frame: %lf + %lf => %lf seconds\n", time_capture, time_process, time_elapsed);
		printf("FPS: %01lf\n", NFRAME/time_elapsed);

		grayframe[i].data = edge;
		printf("[INFO] (On the pop-up window) Press ESC to terminate the program...\n");
		imshow("[EDGE] this is you, smile! :)", grayframe[i]);
		if( waitKey(10) == 27 ) break; // stop capturing by pressing ESC
	}

	avg_cpu_proc_time  /= (double)n_imgs;
	avg_wall_proc_time /= (double)n_imgs;

	printf("The difference in the cpu and wall time measurements is = %lf\n", avg_wall_proc_time - avg_cpu_proc_time);

	//free resrources
	free(frame);
	free(grayframe);
	// for(int i = 0; i < n_imgs; ++i){
	// 	free(images[i]);
	// }
	free(images);
	//		grayframe.release();
	//    delete images;
	return 0;
}
