/****************************************************************************/
/* Function: Code for Problem 5 of Exercise 4							    */
/*                                                                          */
/* Prerit Oberai															*/
/* Works by using the 3 real-time interactive transformations 	            */
/* (simply canny, hough-interactive, and hough-eliptical-interactive).      */
/* and then compareing average frame rate for different resolutions and		*/
/* finally choosing deadlines to determine if deadlines are met (FIFO sched)*/
/*                                                                          */
/*                                                                          */
/****************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <pthread.h>  
#include <time.h>
#include <math.h>
#include <sched.h>
#include <semaphore.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>

// So we want to do the processing of the 3 dsifferent transformations using
// 3 different threads which we can then synchronize using semaphores for each
// "state" of execution 
#define NUM_OF_THREADS 3
// global semaphores for sync purposes
sem_t semCannyInteractive, semHoughTransform, semHoughElliptical;

// This is only because we're taking an average of the frames to process..
#define frames 50

// Standard C++ notation
using namespace cv;
using namespace std;

// Similar to sumply canny and other transformation examples. Going to switch this for the 3 different resolutions...
#define HRES 640	
#define VRES 480

// This is the capture that will be referenced by all threads:
CvCapture* capture;

// This is derived from the canny transformation code
int lowThreshold=0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame, cdst, timg_gray, timg_grad;

// Timing analysis declared here
// This will be used for frame rate calculations
struct timespec time1Frame, time2Frame, time3Frame;
double time1FrameCurrent = 0.00;
double time1FramePrev = 0.00;
double time2FrameCurrent = 0.00;
double time2FramePrev = 0.00;
double time3FrameCurrent = 0.00;
double time3FramePrev = 0.00;

double deadline_canny = 200.0;	
double deadline_houghEliptical = 1300.0;		
double deadline_hough = 350.0;

/* The actual Canny threshold transformation - this is pretty much derived from the examples provided */
void* CannyThreshold(void*){
	// Here, we're going to be using semaphores to synchronize timing and so wait for the semCannyInteractive semaphore
	sem_wait(&semCannyInteractive);
	printf("\n****Canny Transform BEGIN**** \n");
	// Same as canny threshold example but we want frame for each thread now so...
	IplImage* frame;
	char timg_window_name[] = "Canny Transformation";
	cvNamedWindow(timg_window_name, CV_WINDOW_AUTOSIZE);
	// These calculations will be the average frame calcs for this specific execution:
	double frame1_avg = 0.0;
	double frame1_rate_avg = 0.0;
	double frame1_count = 0.0;
	double frame1_time = 0.0;
	double jitter1_time = 0.0;
	double average_jitter1_pos = 0.0;
	double average_jitter1_neg = 0.0;
	unsigned int frame1_counter = 0; 
	unsigned int missed_deadlines_counter = 0; 

	int i = 0; // counter for average frames here
	while (i < frames) {
		// Do actual canny threshold operation here with the frame declaration and everything...
		frame = cvQueryFrame(capture);
		if(!frame) break;
		else {
			// Now we do timing for each frame..
			clock_gettime(CLOCK_REALTIME,&time1Frame);
			time1FrameCurrent = ((double)time1Frame.tv_sec * 1000.0)+ ((double)((double)time1Frame.tv_nsec /1000000.0));
			frame1_counter++;
			// for the case if have multiple frames..
			if (frame1_counter > 2) {
				frame1_count = (double)frame1_counter;
				// Get the average here..
				frame1_avg = ((frame1_count-1.0)*frame1_avg + frame1_time)/frame1_count;
				// Get the rate here : 1/average/1000 = 1000/average
				frame1_rate_avg = 1000.0/frame1_avg;
			}
			// Following is derived from the canny example again
			Mat mat_frame(frame);
			cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);
			blur(timg_gray, canny_frame, Size(3,3));
			/// Canny detector
			Canny(canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size);
			/// Using Canny's output as a mask, we display our result
			timg_grad = Scalar::all(0);
			mat_frame.copyTo( timg_grad, canny_frame);
			/// Final canny transform
			imshow(timg_window_name, timg_grad);

			// Now, we do the timing things again..
			frame1_time = time1FrameCurrent - time1FramePrev;
			time1FramePrev = time1FrameCurrent;

			// This is the analysis for the deadlines...
			if (frame1_counter > 2)
			{
				jitter1_time = deadline_canny - frame1_time;
				if (jitter1_time > 0) {
					// calculate the average jitter time for it beating the deadline..
					average_jitter1_pos = jitter1_time + average_jitter1_pos;
				} else {
					printf("Missed Deadline ------ FRAME: %d \n", i);
					// calculate the average jitter time for it missing the deadline..
					average_jitter1_neg = jitter1_time + average_jitter1_neg;
					missed_deadlines_counter++;
				}
			}
			// Same as main loop from canny example
			char q = cvWaitKey(33);
	        if( q == 'q' )
	        {
	            printf("got quit\n"); 
	            break;
	        }
		}
		i++;
	}
	printf("\n****Canny Transform END**** \n");
	printf("Canny Transformation - Average Time per frame (ms): %f \n",frame1_avg);
	printf("Canny Transformation - Frames per second: %f \n",frame1_rate_avg);
	printf("Canny Transformation - Positive Jitter: %f \n",average_jitter1_pos/frames);
	printf("Canny Transformation - Negative Jitter: %f \n",average_jitter1_neg/frames);
	printf("Canny Transformation - Total missed deadlines: %d \n",missed_deadlines_counter);
	printf("Canny Transformation - Percent Deadlines met: %f%\n",((1.0-((double)missed_deadlines_counter/(double)frames))*100.0));
	printf("\n\n");
	// Now, we can release for the execution of the 2nd thread to start
	sem_post(&semHoughElliptical);
}

// This is also kind of mimiced around the canny threshold examples but for elliptical houghs
void* HoughElliptical(void*){
	// Similar to Canny example, use semaphores to synchronize timing and so wait for the semHoughElliptical semaphore
	sem_wait(&semHoughElliptical);
	printf("\n****Hough Elliptical Transform BEGIN**** \n");

	Mat gray;
	vector<Vec3f> circles;
	double frame2_avg = 0.0;
	double frame2_rate_avg = 0.0;
	double frame2_count = 0.0;
	double frame2_time = 0.0;
	double jitter2_time = 0.0;
	double average_jitter2_pos = 0.0;
	double average_jitter2_neg = 0.0;
	unsigned int frame2_counter = 0; 
	unsigned int missed_deadlines_counter2 = 0;

	IplImage* frame;
	char timg_window_name2[] = "Hough Elliptical";
	cvNamedWindow(timg_window_name2, CV_WINDOW_AUTOSIZE);
	// cvNamedWindow("Hough Circles", CV_WINDOW_AUTOSIZE);
	// // These calculations will be the average frame calcs for this specific execution:
	int i = 0; // counter for average frames here
	while (i < frames) {
		frame=cvQueryFrame(capture);
		if(!frame) break;
		// Similar to canny example here...
		else {
			// Now we do timing for each frame..
			clock_gettime(CLOCK_REALTIME,&time2Frame);
			time2FrameCurrent = ((double)time2Frame.tv_sec * 1000.0)+ ((double)((double)time2Frame.tv_nsec /1000000.0));
			frame2_counter++;
			// for the case if have multiple frames..
			if (frame2_counter > 2) {
				frame2_count = (double)frame2_counter;
				// Get the average here..
				frame2_avg = ((frame2_count-1.0)*frame2_avg + frame2_time)/frame2_count;
				// Get the rate here : 1/average/1000 = 1000/average
				frame2_rate_avg = 1000.0/frame2_avg;
			}
		}
		// Following is derived from the canny example again
		Mat mat_frame(frame);
		cvtColor(mat_frame, gray, CV_RGB2GRAY);
		// Now, we actually do the hough elliptical transformation from the example...
		GaussianBlur(gray, gray, Size(9,9), 2, 2);
		/// Hough  Elliptical
		HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);
		/// Using Canny's output as a mask, we display our result
		for( size_t k = 0; k < circles.size(); k++ )
        {
          Point center(cvRound(circles[k][0]), cvRound(circles[k][1]));
          int radius = cvRound(circles[k][2]);
          // circle center
          circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
          // circle outline
          circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
        }
        cvShowImage(timg_window_name2, frame);
        // Now, we do the timing things again..
		frame2_time = time2FrameCurrent - time2FramePrev;
		time2FramePrev = time2FrameCurrent;

		// This is the analysis for the deadlines...
		if (frame2_counter > 2)
		{
			jitter2_time = deadline_houghEliptical - frame2_time;
			if (jitter2_time > 0) {
				// calculate the average jitter time for it beating the deadline..
				average_jitter2_pos = jitter2_time + average_jitter2_pos;
			} else {
				printf("Missed Deadline ------ FRAME: %d \n", i);
				// calculate the average jitter time for it missing the deadline..
				average_jitter2_neg = jitter2_time + average_jitter2_neg;
				missed_deadlines_counter2++;
			}
		}

		// Same as main loop from canny example
		char q = cvWaitKey(33);
        if( q == 'q' )
        {
            printf("got quit\n"); 
            break;
        }
		i++;
	}
	printf("\n****Hough Elliptical Transform END**** \n");
	printf("Hough Elliptical Transformation - Average Time per frame (ms): %f \n",frame2_avg);
	printf("Hough Elliptical Transformation - Frames per second: %f \n",frame2_rate_avg);
	printf("Hough Elliptical Transformation - Positive Jitter: %f \n",average_jitter2_pos/frames);
	printf("Hough Elliptical Transformation - Negative Jitter: %f \n",average_jitter2_neg/frames);
	printf("Hough Elliptical Transformation - Total missed deadlines: %d \n",missed_deadlines_counter2);
	printf("Hough Elliptical Transformation - Percent Deadlines met: %f%\n",((1.0-((double)missed_deadlines_counter2/(double)frames))*100.0));
	printf("\n\n");
	//Now, we can release for the execution of the 2nd thread to start
	
	sem_post(&semHoughTransform);
}

// Finally, we do the hough lines tranformation here, again, a combination of the hough interactive example and canny example
void* HoughTransform(void*){
	// Similar to Canny example, use semaphores to synchronize timing and so wait for the semHoughTransform semaphore
	sem_wait(&semHoughTransform);
	printf("\n****Hough Normal Transform BEGIN**** \n");
	IplImage* frame;
    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

    char timg_window_name3[] = "Hough Lines Transformation";
	cvNamedWindow(timg_window_name3, CV_WINDOW_AUTOSIZE);
	// These calculations will be the average frame calcs for this specific execution:
	double frame3_avg = 0.0;
	double frame3_rate_avg = 0.0;
	double frame3_count = 0.0;
	double frame3_time = 0.0;
	double jitter3_time = 0.0;
	double average_jitter3_pos = 0.0;
	double average_jitter3_neg = 0.0;
	unsigned int frame3_counter = 0;
	unsigned int missed_deadlines_counter3 = 0; 

	int i = 0; // counter for average frames here
	while (i < frames) {
		frame=cvQueryFrame(capture);
		if(!frame) break;
		// Similar to canny example here...
		else {
			// Now we do timing for each frame..
			clock_gettime(CLOCK_REALTIME,&time3Frame);
			time3FrameCurrent = ((double)time3Frame.tv_sec * 1000.0)+ ((double)((double)time3Frame.tv_nsec /1000000.0));
			frame3_counter++;
			// for the case if have multiple frames..
			if (frame3_counter > 2) {
				frame3_count = (double)frame3_counter;
				// Get the average here..
				frame3_avg = ((frame3_count-1.0)*frame3_avg + frame3_time)/frame3_count;
				// Get the rate here : 1/average/1000 = 1000/average
				frame3_rate_avg = 1000.0/frame3_avg;
			}
		}
		// Following is derived from the canny example again
		Mat mat_frame(frame);
		// Now, we do what Sam siewart did in his example code for the hough normal tranformation
		Canny(mat_frame, canny_frame, 50, 200, 3);
		cvtColor(canny_frame, cdst, CV_GRAY2BGR);
        cvtColor(mat_frame, gray, CV_BGR2GRAY);
		HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);
		for( size_t k = 0; k < lines.size(); k++ )
        {
          Vec4i l = lines[k];
          line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
        }
        cvShowImage(timg_window_name3, frame);
        // Now, we do the timing things again..
		frame3_time = time3FrameCurrent - time3FramePrev;
		time3FramePrev = time3FrameCurrent;

		// This is the analysis for the deadlines...
		if (frame3_counter > 2)
		{
			jitter3_time = deadline_hough - frame3_time;
			if (jitter3_time > 0) {
				// calculate the average jitter time for it beating the deadline..
				average_jitter3_pos = jitter3_time + average_jitter3_pos;
			} else {
				printf("Missed Deadline ------ FRAME: %d \n", i);
				// calculate the average jitter time for it missing the deadline..
				average_jitter3_neg = jitter3_time + average_jitter3_neg;
				missed_deadlines_counter3++;
			}
		}

		// Same as main loop from canny example
		char q = cvWaitKey(33);
        if( q == 'q' )
        {
            printf("got quit\n"); 
            break;
        }
		i++;
	}
	printf("\n****Hough Normal Transform END**** \n");
	printf("Hough Normal Transformation - Average Time per frame (ms): %f \n",frame3_avg);
	printf("Hough Normal Transformation - Frames per second: %f \n",frame3_rate_avg);
	printf("Hough Normal Transformation - Positive Jitter: %f \n",average_jitter3_pos/frames);
	printf("Hough Normal Transformation - Negative Jitter: %f \n",average_jitter3_neg/frames);
	printf("Hough Normal Transformation - Total missed deadlines: %d \n",missed_deadlines_counter3);
	printf("Hough Normal Transformation - Percent Deadlines met: %f%\n",((1.0-((double)missed_deadlines_counter3/(double)frames))*100.0));
	printf("\n\n");
	// Now, we can release for the execution of the Canny thread to start again and just loop...
	// sem_post(&semCannyInteractive);
}

int main(int argc, char** argv) {
	// Here, we initialize the semaphores with the Canny interactive going first
	sem_init(&semCannyInteractive, 0, 1);
	sem_init(&semHoughElliptical, 0, 0);
	sem_init(&semHoughTransform, 0, 0);

	// Here, we want to use the standard threads implemention that we've been using in the past
	pthread_attr_t rt_sched_attr;
	pthread_t threads[NUM_OF_THREADS];
	struct sched_param rt_param;
	rt_param.sched_priority=98;

	// Set the attributes (this is where we implement it in FIFO SCHED)
	pthread_attr_init(&rt_sched_attr);
	pthread_attr_setinheritsched(&rt_sched_attr,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&rt_sched_attr,SCHED_FIFO);
	pthread_attr_setschedparam(&rt_sched_attr,&rt_param);

	// And finally, just like the other main loops for the hough, canny, take a command line argument for the
	// device num
	int dev = 0;
	 if(argc > 1)
    {
        sscanf(argv[1], "%d", &dev);
        printf("using %s\n", argv[1]);
    }
    else if(argc == 1)
        printf("using default\n");

    else
    {
        printf("usage: capture [dev]\n");
        exit(-1);
    }

    capture = (CvCapture *)cvCreateCameraCapture(dev);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

    // And start the threads here and wait for them to finish... 
    while(1) {
    	pthread_create(&threads[0],&rt_sched_attr,HoughTransform,NULL);
    	pthread_create(&threads[1],&rt_sched_attr,HoughElliptical,NULL);
    	pthread_create(&threads[2],&rt_sched_attr,CannyThreshold,NULL);
    	// Wait for the threads to finish execution
    	for(int threads_num = 0; threads_num < NUM_OF_THREADS; threads_num++){
    		pthread_join(threads[threads_num],NULL);
    	}
    }
    cvReleaseCapture(&capture);
};