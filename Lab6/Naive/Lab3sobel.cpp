//Group: Nima Behmanesh, Nathan Jaggers, Joshua Rizzolo
//Youtube Demo link: https://youtu.be/c75YS3goRbs

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

using namespace std;
using namespace cv;

Mat to442_grayscale(Mat &image);
Mat to442_sobel(Mat &greyImage);

int counter = 0;
struct timeval start, stop;
int microSec = 0;

int main(int argc, char *argv[]){
  // convert command line arg to string
  if (!argv[1])
  {
    printf("No argument given to program.\n");
    return -1;
  }

  //Convert Terminal Command line args to string
  string mediaName(argv[1]);
	
  // Read the video file
  VideoCapture cap(mediaName);

  //Breaks if no video is accesible
  if(!cap.isOpened()){
    cout << "Error reading video file" << endl;
    return -1;
  }

  
  //Main loop taking frames, turning to greyscale, and calculating gradient
  while(1){
    Mat frame;


    cap >> frame;
    if (frame.empty())
      break;
      
    gettimeofday(&start, NULL);
    counter++;

    Mat BWFrame = to442_grayscale(frame);
    
    Mat SobelFrame = to442_sobel(BWFrame);
    
    //waitKey(1);
    char c = (char)waitKey(1);
    if (c==27)
      break;
    
    imshow("Sobel Image", SobelFrame);
    
    gettimeofday(&stop, NULL);
    
    microSec += 1000000 * (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec);

    //imshow("Original Image", frame);
    
  }
  
  cout<<(1000000.0*counter)/((float)microSec)<<endl;

  //Close the video file
  cap.release();
  //Close all image windows
  destroyAllWindows();
  //Finish Main
  return 0;
  
}


Mat to442_grayscale(Mat &image){
  unsigned int brightness;

  Mat grey(image.rows, image.cols, CV_8UC1);

  uchar *inPixels = (uchar*) image.data;
  uchar *greyPixels = (uchar*) grey.data;

  

  //for loops iterating thru the image pixel-by-pixel
  for (int rows = 0; rows<(image.rows-1); rows++){
    for (int cols = 0; cols<(image.cols-1); cols++){

      brightness =
	0.0722*inPixels[rows*image.cols*3 + cols*3 + 0]   //B
	+ 0.7152*inPixels[rows*image.cols*3 + cols* 3 + 1]//G
	+ 0.2126*inPixels[rows*image.cols*3 + cols*3 + 2];//R


      greyPixels[rows*image.cols + cols] = (uchar) brightness;
	  
	
      }
    }
  return grey;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////


Mat to442_sobel(Mat &greyImage){
  //   [-1 0 1]    [ 1  2  1] |           
  //Gx [-2 0 2] Gy [ 0  0  0] |  Gx * A = 1*(i-1)(j-1) - 1*(i-1)(j+1) - 2*(i)(j+1) + 2*(i)(j-1) - 1*(i+1)(j+1) + 1*(i+1)(j-1)
  //   [-1 0 1]    [-1 -2 -1] |  Gy * A = -1*(i-1)(j+1) - 2*(i-1)(j) - 1*(i-1)(j-1) + 1*(i+1)(j-1) + 2*(i+1)(j) + 1*(i+1)(j+1)

  Mat SobelFrame(greyImage.rows, greyImage.cols, CV_8UC1);
  uchar *greyPixels = (uchar*) greyImage.data;
  uchar *sobelPixels = (uchar*) SobelFrame.data;
  uchar p1, p2, p3, p4, p6, p7, p8, p9;

  short xtotal, ytotal, total;
  
  for(int rows=1;  rows<(greyImage.rows-1); rows++){
    for(int cols=1; cols<(greyImage.cols-1); cols++){
      //Getting neighboring Pixels
      p1 = greyPixels[(rows-1)*greyImage.cols + cols - 1];
      p2 = greyPixels[(rows-1)*greyImage.cols + cols + 0];
      p3 = greyPixels[(rows-1)*greyImage.cols + cols + 1];
      p4 = greyPixels[rows*greyImage.cols + cols - 1];
      //p5 = greyPixels[rows*greyImage.cols + cols + 0];
      p6 = greyPixels[rows*greyImage.cols + cols + 1]; 
      p7 = greyPixels[(rows+1)*greyImage.cols + cols - 1];
      p8 = greyPixels[(rows+1)*greyImage.cols + cols + 0];
      p9 = greyPixels[(rows+1)*greyImage.cols + cols + 1]; 
	
      //Finding X Gradient
      xtotal = p1 - p3 + 2*p4 - 2*p6 + p7 - p9;
 
      //Finding y gradient
      ytotal = p1 + 2*p2 + p3 - p7 - 2*p8 - p9;

      total = (abs(ytotal) + abs(xtotal));

      if (total>255)
	total = 255;
      
      sobelPixels[rows*greyImage.cols + cols] = (uchar) total;
    }
  }
  return SobelFrame;
}

//Makefile Code:
//#PHONY Targets to Execute Reciple
//.PHONY: all clean
//
//#compiler and linker options
//CC = gcc
//CXX = g++
//OUT = sobel
//LDFLAGS = -g
//CFLAGS = -Werror
//
//OPENCV = `pkg-config opencv4 --cflags --libs`
//
//#executable dependencies
//DEPS = ${wildcard *.h}
//SRCS = ${wildcard *.cpp}
//
//#default build target
//all: default
//
//default:
//${CXX} ${CFLAGS} ${LDFLAGS} ${SRCS} ${DEPS} -o ${OUT} ${OPENCV}
//Sobel: #Force this make statement by typing 'make Sobel'
//g++ sobel.cpp -o sobel 'pkg-config opencv4 --cflags --libs'
//clean:
//rm -f ${OUT}
