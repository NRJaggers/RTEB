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

#define THREADCOUNT 1

//Create threads
pthread_t thread[THREADCOUNT]

struct threadArgs {
    Mat input;
    Mat output;
    int thread;
}

Mat to442_grayscale(Mat);
Mat to442_sobel(Mat);
void *grobel(void *threadArgs);

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
	
	//Create Mat and read in one frame at a time from video
	Mat frame, sobelimage;

	//Create threads
    //pthread_t thread[THREADCOUNT]
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,  PTHREAD_CREATE_JOINABLE);
    //prob will need to change to array of thread args
    //or will need to hard code for either 1 or 4/5 threads
    struct threadArgs single_t =  {.input  =  frame,  .output  =  sobelimage, thread = 1};
   
    //can create threads and join them in while loop each time through
    //can create persistant threads outside of loop, and have barriers that sync 
    //main and frame threads between image processing and image show

    while(cap.read(frame))
	{
            //take frame, apply grayscale and sobel through threads
            

            //show final image
        	imshow("Sobel", sobelimage);

            //escape early if desired
        	if(waitKey(30) == 27) // Wait for 'esc' key press to exit
        	{ 
            		break; 
        	}
	}

    //Join threads

	return 0;
}

Mat to442_grayscale(Mat imcolor)
{
	//Define new matrix with one channel
	Mat imgray(imcolor.rows, imcolor.cols, CV_8UC1);
	
	//using pixels from color image, generate grayscale
	uchar *inPixels = (uchar*) imcolor.data;
 	uchar *greyPixels = (uchar*) imgray.data;
	unsigned int brightness;

  	//for loops iterating thru the image pixel-by-pixel
  	for (int rows = 0; rows<(imcolor.rows); rows++)
	{
    		for (int cols = 0; cols<(imcolor.cols); cols++)
		{

      			brightness =
			0.0722*inPixels[rows*imcolor.cols*3 + cols*3 + 0]   //B
			+ 0.7152*inPixels[rows*imcolor.cols*3 + cols* 3 + 1]//G
			+ 0.2126*inPixels[rows*imcolor.cols*3 + cols*3 + 2];//R


      			greyPixels[rows*imcolor.cols + cols] = (uchar) brightness;
		}
	}
	
	return imgray;
}

Mat to442_sobel(Mat imgray)
{
	/*
	 * Gx		Gy
	 * [-1 +0 +1 	[+1 +2 +1
	 *  -2 +0 +2 	 +0 +0 +0
	 *  -1 +0 +1] 	 -1 -2 -1]
	 *
	 * |G| = |Gx| + |Gy|
	 */

	//Define new matrix for sobel
	Mat imsobel(imgray.rows-2, imgray.cols-2, CV_8UC1);

	//using pixels from grayscale image, apply sobel
	uchar *greyPixels = (uchar*) imgray.data;
	uchar *sobelPixels = (uchar*) imsobel.data;
	int gx, gy, g;

	//for loops iterating thru the image pixel-by-pixel
        for (int rows = 0; rows<(imsobel.rows); rows++)
        {
                for (int cols = 0; cols<(imsobel.cols); cols++)
                {

                	gx = greyPixels[rows*imgray.cols+cols+2]
			    -greyPixels[rows*imgray.cols+cols]
			    +greyPixels[(rows+1)*imgray.cols+cols+2]*2
			    -greyPixels[(rows+1)*imgray.cols+cols]*2
			    +greyPixels[(rows+2)*imgray.cols+cols+2]
			    -greyPixels[(rows+2)*imgray.cols+cols];

			gy = greyPixels[rows*imgray.cols+cols+2]
			    +greyPixels[rows*imgray.cols+cols+1]*2
			    +greyPixels[rows*imgray.cols+cols+0]
			    -greyPixels[(rows+2)*imgray.cols+cols+2]
			    -greyPixels[(rows+2)*imgray.cols+cols+1]*2
			    -greyPixels[(rows+2)*imgray.cols+cols+0];
			
			g = (abs(gx)+abs(gy)) > 255 ? 255 : (abs(gx)+abs(gy));
		       	sobelPixels[rows*imsobel.cols + cols] = (uchar) g;
                }
        }

        return imsobel;

}

void groble(image, threadno)
{
    //convert to grayscale
    //will prob need to separate definition of image from filling it with data
    Mat grayImage = to442_grayscale(frame);

    //barrier
    
    //apply sobel filter
    //same deal, will prob need to separate definition from data filling
    Mat sobelimage = to442_sobel(grayImage);

    //barrier 


    return;
}

