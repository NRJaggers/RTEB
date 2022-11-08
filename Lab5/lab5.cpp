//Group: Nima Behmanesh, Nathan Jaggers, Joshua Rizzolo
//Youtube Demo link: https://youtu.be/c75YS3goRbs

#include<arm_neon.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat to442_grayscale(Mat &image);
Mat to442_sobel(Mat &greyImage);

int main(){
  VideoCapture cap("Pigs.gif");

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


    Mat BWFrame = to442_grayscale(frame);
    
    Mat SobelFrame = to442_sobel(BWFrame);
    
    imshow("Sobel Image", BWFrame);

    imshow("Original Image", frame);

    //waitKey(1);
    char c = (char)waitKey(2);
    if (c==27)
      break;
    
  }

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

  uint8x8_t w_r = vdup_n_u8(77);
  uint8x8_t w_g = vdup_n_u8(150);
  uint8x8_t w_b = vdup_n_u8(29);
  uint16x8x3_t BGRvector;
  uint16x8_t brightVector;
  
  //for loops iterating thru the image pixel-by-pixel
  for(int index = 0; index<(image.rows*image.cols-1); index+=8, greyPixels+=8, inPixels+=8 * 3){
      //Reading color values into a structure of 3 vectors of 16 8-bit entries
      //vld3 has a stride of 3, automatically separating entries into BGR
      BGRvector =  vld3_u8((const uint8_t *)inPixels);
      //Turning BGR vector into brightness by adding them together post-scaling
      brightVector = vmull_u8(BGRvector.val[0], w_r);
      //multiply-accumulate-widen
      brightVector = vmlal_u8(brightVector, BGRvector.val[1], w_g);
      brightVector = vmlal_u8(brightVector, BGRvector.val[2], w_b);
      uint8x8_t result = vshrn_n_u16(brightVector, 8);
      //Storing converted pixels in grey image
      vst1_u8((uint8_t *)greyPixels, result);
			 				    
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
  uint8x8_t topL, topM, topR, midL,  midR, botL, botM, botR, result;
  
  for(int ind = greyImage.cols;  ind<((greyImage.rows-1) * greyImage.cols -1); greyPixels+=8, sobelPixels +=8, ind+=8){

    vld1_u8((const uint8_t *)greyPixels);
    
  }
   
      if (total>255)
	total = 255;
      
      sobelPixels[rows*greyImage.cols + cols] = (uchar) total;
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

