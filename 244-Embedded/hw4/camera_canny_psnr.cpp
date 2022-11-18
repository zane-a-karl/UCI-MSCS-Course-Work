/**
 */
#include <iostream>
#include <unistd.h>
#include "opencv2/opencv.hpp"
#include "canny_util.h"
#include "calcpsnr.h"
#include <sys/time.h>
#include <stdlib.h>
using namespace std;
using namespace cv;

/* Pi Camera MAX resolution: 2592x1944 (default: 640x480) */
/* ground_crew.h264: 1280x720 */
/* tiger_face.jpg: 888x900 */

// #define PICAM_WIDTH 640
// #define PICAM_HEIGHT 480

// #define GROUND_WIDTH 1280
// #define GROUND_HEIGHT 720

// #define TIGER_WIDTH 888
// #define TIGER_HEIGHT 900

//#define NFRAME 30.0

enum IMGSRC {PICAM, GROUND, TIGER};

int main(int argc, char **argv)
{
	char* dirfilename;        /* Name of the output gradient direction image */
	char outfilename[128];    /* Name of the output "edge" image */
	char composedfname[128];  /* Name of the output "direction" image */
	unsigned char *image;     /* The input image */
	unsigned char *edge;      /* The output edge image */
	int rows, cols;           /* The dimensions of the image. */
	float sigma,              /* Standard deviation of the Gaussian kernel. */
		tlow,               /* Fraction of the high threshold in hysteresis. */
		thigh;              /* High hysteresis threshold control. The actual
													 threshold is the (100 * thigh) percentage point
													 in the histogram of the magnitude of the
													 gradient image that passes non-maximal
													 suppression. */
	int count;            /* Frame count iterator */
	enum IMGSRC img_src = PICAM;
	int mp;
	float NFRAME = 30.0;

	/****************************************************************************
   * Get the command line arguments.
   ****************************************************************************/
	if(argc < 8){
		fprintf(stderr,"\n<USAGE> %s sigma tlow thigh imgsrc [writedirim]\n",argv[0]);
		fprintf(stderr,"      sigma:      Standard deviation of the gaussian");
		fprintf(stderr," blur kernel.\n");
		fprintf(stderr,"      tlow:       Fraction (0.0-1.0) of the high ");
		fprintf(stderr,"edge strength threshold.\n");
		fprintf(stderr,"      thigh:      Fraction (0.0-1.0) of the distribution");
		fprintf(stderr," of non-zero edge\n                  strengths for ");
		fprintf(stderr,"hysteresis. The fraction is used to compute\n");
		fprintf(stderr,"                  the high edge strength threshold.\n");
		fprintf(stderr,"      imgwidth:    integer 400 850 1300\n");
		fprintf(stderr,"      imgheight:   integer 100 550 1000\n");
		fprintf(stderr,"      multi-thread:integer 0=none, 1=pt, 2=omp\n");				
		fprintf(stderr,"      imgsrc:      integer 0=picam, 1=ground_ctrl, 2=tiger\n");		
		fprintf(stderr,"      writedirim: Optional argument to output ");
		fprintf(stderr,"a floating point");
		fprintf(stderr," direction image.\n\n");
		exit(1);
	}

	sigma = atof(argv[1]);
	tlow  = atof(argv[2]);
	thigh = atof(argv[3]);
	cols  = atoi(argv[4]);
	rows  = atoi(argv[5]);
	mp    = atoi(argv[6]);

	int len = 256;
	char *line = (char *)calloc(len, sizeof(char));
	
	FILE *fout;
	VideoCapture cap;
	// open the default camera (/dev/video0) OR a video OR an image
	// Check VideoCapture documentation for more details
	img_src = (enum IMGSRC)atoi(argv[7]);
	switch (img_src) {
	case PICAM:
		if ( NULL == (fout = fopen(picam.csv, "a"))) {
			printf("Failed to open picam.csv\n");
			return 0;			
		}
		if(!cap.open(0)){
			printf("Failed to open media\n");
			return 0;
		}
		break;
	case GROUND:
		NFRAME = 30.0;
		if ( NULL == (fout = fopen(ground.csv, "a"))) {
			printf("Failed to open ground.csv\n");
			return 0;			
		}
		if(!cap.open("ground_crew.h264")){
			printf("Failed to open media\n");
			return 0;
		}
		break;
	case TIGER:
		NFRAME = 1.0;
		if ( NULL == (fout = fopen(tiger.csv, "a"))) {
			printf("Failed to open tiger.csv\n");
			return 0;			
		}
		if(!cap.open("tiger_face.jpg")){
			printf("Failed to open media\n");
			return 0;
		}
		break;
	}


	if(argc == 8) dirfilename = (char *) "dummy";
	else dirfilename = NULL;


	// Set input resolution when the video is captured from /dev/video*, i.e. the webcam.
	//	 cap.set(CAP_PROP_FRAME_WIDTH, WIDTH);
	//   cap.set(CAP_PROP_FRAME_HEIGHT,HEIGHT);
	printf("Media Input: %.0f, %.0f\n", cap.get(CAP_PROP_FRAME_WIDTH), cap.get(CAP_PROP_FRAME_HEIGHT));

	// For low-end CPUs, may wait a while until camera stabilizes
	printf("Sleep 3 seconds for camera stabilization...\n");
	usleep(3*1e6);
	printf("=== Start Canny Edge Detection: %.0f frames ===\n", NFRAME);

	Mat frame, grayframe;

	count = 0;
	struct timeval start, end;
	gettimeofday(&start,NULL);
	while(count<NFRAME) {
		//capture
		cap >> frame;
		switch (img_src) {
		case PICAM:
			resize(frame, frame, Size(PICAM_WIDTH, PICAM_HEIGHT), 0, 0, INTER_LINEAR);			
			break;
		case GROUND:
			resize(frame, frame, Size(GROUND_WIDTH, GROUND_HEIGHT), 0, 0, INTER_LINEAR);
			break;
		case TIGER:
			resize(frame, frame, Size(TIGER_WIDTH, TIGER_HEIGHT), 0, 0, INTER_LINEAR);
			break;
		}

		//extract the image in gray format
		cvtColor(frame, grayframe, COLOR_BGR2GRAY);
		image = grayframe.data;

		/****************************************************************************
     * Perform the edge detection. All of the work takes place here.
     ****************************************************************************/
		if(VERBOSE) printf("Starting Canny edge detection.\n");
		if(dirfilename != NULL){
			sprintf(composedfname, "camera_s_%3.2f_l_%3.2f_h_%3.2f_%d.fim",
							sigma, tlow, thigh,count);
			dirfilename = composedfname;
		}

		/****************************************************************************
     *Apply Canny Edge Detection Algorithm
     ****************************************************************************/
		canny(image, rows, cols, sigma, tlow, thigh, &edge, dirfilename);


		/****************************************************************************
     * Write out the edge image to a file.
     ****************************************************************************/
		sprintf(outfilename, "edge/EDGE_%03d.pgm", count);
		if(VERBOSE) printf("Writing the edge image in the file %s.\n", outfilename);
		if(write_pgm_image(outfilename, edge, rows, cols, NULL, 255) == 0){
			fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
			exit(1);
		}

		/****************************************************************************
     * Write out the captured image to a file.
     ****************************************************************************/
		sprintf(outfilename, "raw/RAW_%03d.pgm", count);
		if(VERBOSE) printf("Writing the edge image in the file %s.\n", outfilename);
		if(write_pgm_image(outfilename, image, rows, cols, NULL, 255) == 0){
			fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
			exit(1);
		}
		count++;
	} //end of while loop

	/****************************************************************************
	 * Get FPS information.
	 ****************************************************************************/
	gettimeofday(&end,NULL);
	double time_elapsed =  ((end.tv_sec*1000000 + end.tv_usec) - (start.tv_sec*1000000 +start.tv_usec));
	printf("=== Finish Canny Edge Detection ===\n");
	printf("Total Elapsed Time : %lf sec.\n", time_elapsed/1000000);
	printf("FPS: %4f.\n", NFRAME/(time_elapsed/1000000));

	/****************************************************************************
   * Read the edge detected files and raw_images and calculate PSNR.
   ****************************************************************************/
	count = 0;
	char raw_image_name[128];
	char edge_image_name[128];
	double PSNR = 0.0;

	if(VERBOSE) printf("Calculating PSNR value...\n");
	while(count<NFRAME) {

		/* Get the edge image name */
		sprintf(edge_image_name, "edge/EDGE_%03d.pgm", count);

    /* Get the raw image name */
		sprintf(raw_image_name, "raw/RAW_%03d.pgm", count);
		PSNR = PSNR + calcpsnr(raw_image_name, edge_image_name);
		count++;
	}

	PSNR = PSNR / NFRAME;
	printf("Average PSNR value = %3.2f \n", PSNR);

	snprintf(line, len, "%.0f, %.0f, %.0f, %i, %i, %i, %lf, %4f, %3.2f\n",
					 sigma, tlow, thigh, cols, rows, mp,
					 time_elapsed/1000000, NFRAME/(time_elapsed/1000000), PSNR);
	if ( fputs(line, fout) < 0 ) {
		printf("Error appending line to csv file\n");
		return 0;
	}

	//free resources
	fclose(fout);
	//    delete image;
	return 0;
}
