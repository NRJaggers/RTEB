/*****************************************************
* Filename: lab3.cpp
*
* Description: brief description of file purpose
*
* Author: Nathan Jaggers, Josh Rizzolo
******************************************************/

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat to442_grayscale(Mat);
Mat to442_sobel(Mat);

int main(int argc, char *argv[])
{
	// convert command line arg to string
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
	Mat frame;

	while(cap.read(frame))
	{
	        //convert image to grayscale
        	Mat grayimage = to442_grayscale(frame);

	        //apply sobel filter
	        Mat sobelimage = to442_sobel(grayimage);
        	imshow("Sobel", sobelimage);

        	waitKey(60);

	}

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
