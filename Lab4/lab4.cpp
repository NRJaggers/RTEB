/*****************************************************
* Filename: lab4.cpp
*
* Description: brief description of file purpose
*
* Author: Nathan Jaggers, Josh Rizzolo
******************************************************/

#include <iostream>
#include <opencv2/opencv.hpp>
#include <pthread.h>

using namespace cv;
using namespace std;

//defines for any values you want to change easily
#define THREADCOUNT 4

//Create threads, and related pointers
pthread_t thread[THREADCOUNT];

void *threadStatus;

//set thread attributes
//pthread_attr_t attr;
//pthread_attr_init(&attr);
//pthread_attr_setdetachstate(&attr,  PTHREAD_CREATE_JOINABLE);

//create and initialize barriers for threads
pthread_barrier_t grayBarrier, sobelBarrier;

//pthread_barrier_init(&grayBarrier, NULL, THREADCOUNT);
//pthread_barrier_init(&sobelBarrier, NULL, THREADCOUNT + 1); 

//create struct to hold arguments to pass to thread function
struct threadArgs {
    Mat *input;
    Mat *output;
    int start1, stop1, start2, stop2;
};

//prototypes for functions
void to442_grayscale(Mat*, Mat*, int, int);
void to442_sobel(Mat*, Mat*, int, int);
void *grobel(void *args);

int main(int argc, char *argv[])
{
	// convert command line arg to string
	if (!argv[1])
	{
		printf("No argument given to program.\n");
		return -1;
	}

	string mediaName(argv[1]);
	
	// Read the video file
	VideoCapture cap(mediaName);

	// Check for failure
	if (!cap.isOpened()) 
 	{
		cout << "Could not open or find the file" << endl;
		cin.get(); //wait for any key press
 		return -1;
 	}
	
	//Create Mats and read in one frame from video
	Mat frame; 
    cap.read(frame);
    Mat sobelimage(frame.rows-2, frame.cols-2, CV_8UC1);

    //determine chunck size for threads
    int graychunk = (frame.rows*frame.cols)/THREADCOUNT;
    int sobelchunk = ((frame.rows-2)*(frame.cols-2))/THREADCOUNT;

    //create and initialize arguments for threaded functions
    struct threadArgs multi_t[THREADCOUNT]; 
    
    for(int i = 0; i < THREADCOUNT; i++)
    {
        multi_t[i].input  = &frame;
        multi_t[i].output = &sobelimage;
        multi_t[i].start1 = i*graychunk;
        multi_t[i].stop1  = i*graychunk + graychunk - 1;
        multi_t[i].start2 = i*sobelchunk;
        multi_t[i].stop2  = i*sobelchunk + sobelchunk - 1;
    }  

   
    //create barrier for main thread
    pthread_barrier_init(&sobelBarrier, NULL, THREADCOUNT + 1); 
    
    //create persistent threads

    for(int i = 0; i < THREADCOUNT; i++)
    {
        pthread_create(&thread[i], NULL, grobel, (void *)(&multi_t[i]));
    }

    //wait for processing of first frame
    //pthread_barrier_wait(&sobelBarrier);

    while(cap.read(frame))
    //while(true)
	{
            //grobel(multi_t);    

            //after grabbing new frame, wait until processing is finished 
            pthread_barrier_wait(&sobelBarrier);
            
            //cap >>frame;
            //if(frame.empty()){break;}

            //show final image
        	imshow("Sobel", sobelimage);

            //escape early if desired
        	if(waitKey(30) == 27) // Wait for 'esc' key press to exit
        	{ 
            		break; 
        	}

            //wait for image to show and ensure esc is not pressed
            //pthread_barrier_wait(&mainBarrier);
	}
    
    //barrier to let threads finish and exit 
    pthread_barrier_wait(&sobelBarrier);

    //Close the video file
    cap.release();
  
    //Close all image windows
    destroyAllWindows();

    //Join threads
    for(int i = 0; i < THREADCOUNT; i++)
    {
        pthread_join(thread[i], NULL);
    }

	return 0;
}

void to442_grayscale(Mat *imcolor, Mat *imgray, int start, int stop)
{

	//using pixels from color image, generate grayscale
	uchar *inPixels = (uchar*) imcolor->data;
 	uchar *greyPixels = (uchar*) imgray->data;
	unsigned int brightness;

  	//for loops iterating thru the image pixel-by-pixel
  	for (int rows = (start/imcolor->cols); rows<((stop+1)/imcolor->cols); rows++)
	{
    		for (int cols = 0; cols<(imcolor->cols); cols++)
		{

      			brightness =
			0.0722*inPixels[rows*imcolor->cols*3 + cols*3 + 0]   //B
			+ 0.7152*inPixels[rows*imcolor->cols*3 + cols* 3 + 1]//G
			+ 0.2126*inPixels[rows*imcolor->cols*3 + cols*3 + 2];//R


      			greyPixels[rows*imcolor->cols + cols] = (uchar) brightness;
		}
	}
	
	return;
}

void to442_sobel(Mat *imgray, Mat *imsobel, int start, int stop)
{
	/*
	 * Gx		Gy
	 * [-1 +0 +1 	[+1 +2 +1
	 *  -2 +0 +2 	 +0 +0 +0
	 *  -1 +0 +1] 	 -1 -2 -1]
	 *
	 * |G| = |Gx| + |Gy|
	 */

	//using pixels from grayscale image, apply sobel
	uchar *greyPixels = (uchar*) imgray->data;
	uchar *sobelPixels = (uchar*) imsobel->data;
	int gx, gy, g;

	//for loops iterating thru the image pixel-by-pixel
        for (int rows = (start/imsobel->cols); rows<((stop+1)/imsobel->cols); rows++)
        {
                for (int cols = 0; cols<(imsobel->cols); cols++)
                {

                	gx = greyPixels[rows*imgray->cols+cols+2]
			    -greyPixels[rows*imgray->cols+cols]
			    +greyPixels[(rows+1)*imgray->cols+cols+2]*2
			    -greyPixels[(rows+1)*imgray->cols+cols]*2
			    +greyPixels[(rows+2)*imgray->cols+cols+2]
			    -greyPixels[(rows+2)*imgray->cols+cols];

			gy = greyPixels[rows*imgray->cols+cols+2]
			    +greyPixels[rows*imgray->cols+cols+1]*2
			    +greyPixels[rows*imgray->cols+cols+0]
			    -greyPixels[(rows+2)*imgray->cols+cols+2]
			    -greyPixels[(rows+2)*imgray->cols+cols+1]*2
			    -greyPixels[(rows+2)*imgray->cols+cols+0];
			
			g = (abs(gx)+abs(gy)) > 255 ? 255 : (abs(gx)+abs(gy));
		       	sobelPixels[rows*imsobel->cols + cols] = (uchar) g;
                }
        }

        return;

}

void *grobel(void *args)
{
    //initialize barrier for image processing threads
    pthread_barrier_init(&grayBarrier, NULL, THREADCOUNT);
    
    //cast arguments passed in back to struct 
    threadArgs temp = *((threadArgs*)args);
    
    while(!temp.input->empty())
    {

        //create pointer for grayscale image
        Mat grayTemp(temp.input->rows, temp.input->cols, CV_8UC1);

        //convert to grayscale
        to442_grayscale(temp.input, &grayTemp, temp.start1, temp.stop1);

        //wait for threads to finish grayscale of image
        pthread_barrier_wait(&grayBarrier);
        
        //apply sobel filter
        to442_sobel(&grayTemp, temp.output, temp.start2, temp.stop2);

        //wait for threads to finish sobel filtering the image
        pthread_barrier_wait(&sobelBarrier);
    
    }

    return NULL;
}

