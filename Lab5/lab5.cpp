/*****************************************************
* Filename: lab5.cpp
*
* Description: brief description of file purpose
*
* Author: Nima Behmanesh, Nathan Jaggers, Josh Rizzolo
******************************************************/

#include <iostream>
#include <opencv2/opencv.hpp>
#include <pthread.h>
//#include <arm_neon.h>

using namespace cv;
using namespace std;

//defines for any values you want to change easily
#define THREADCOUNT 4

//Create threads, and related pointers
pthread_t thread[THREADCOUNT];

void *threadStatus;

//create and initialize barriers for threads
pthread_barrier_t grayBarrier, sobelBarrier;

//create struct to hold arguments to pass to thread function
struct threadArgs {
    Mat *input;
    Mat *grayTemp;
    Mat *output;
    int start1, stop1, start2, stop2;
};

//prototypes for functions
void to442_grayscale(Mat*, Mat*, int, int);
void to442_sobel(Mat*, Mat*, int, int, int);
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
    Mat grayimage(frame.rows, frame.cols, CV_8UC1);
    Mat sobelimage(frame.rows, frame.cols, CV_8UC1);

    //determine chunck size for threads
    int graychunk = (frame.rows*frame.cols)/THREADCOUNT;
    int sobelchunk = (sobelimage.rows*sobelimage.cols)/THREADCOUNT;

    //create and initialize arguments for threaded functions
    struct threadArgs multi_t[THREADCOUNT]; 
    
    for(int i = 0; i < THREADCOUNT; i++)
    {
        multi_t[i].input  = &frame;
        multi_t[i].grayTemp = &grayimage;
        multi_t[i].output = &sobelimage;
        multi_t[i].start1 = i*graychunk;
        multi_t[i].stop1  = i*graychunk + graychunk - 1;
        multi_t[i].start2 = i*sobelchunk;
        multi_t[i].stop2  = i*sobelchunk + sobelchunk - 1;
    }  

   
    //create barrier for main and threads
    pthread_barrier_init(&sobelBarrier, NULL, THREADCOUNT + 1); 
    pthread_barrier_init(&grayBarrier, NULL, THREADCOUNT);
    
    //create persistent threads

    for(int i = 0; i < THREADCOUNT; i++)
    {
        pthread_create(&thread[i], NULL, grobel, (void *)(&multi_t[i]));
    }

    //wait for processing of first frame
    pthread_barrier_wait(&sobelBarrier);

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
	uchar *inPixels = (uchar*) imcolor->data + start*3;
 	uchar *greyPixels = (uchar*) imgray->data + start;


    //define vectors for vector math and storing results
    uint8x8_t w_r = vdup_n_u8(77);
    uint8x8_t w_g = vdup_n_u8(150);
    uint8x8_t w_b = vdup_n_u8(29);
    uint8x8x3_t BGRvector;
    uint16x8_t brightVector;
    uint8x8_t result;

  	//for loops iterating thru the image pixel-by-pixel
  	for (int index = start; index<(stop+1); index+=8, greyPixels+=8, inPixels+=(8*3))
	{
          //Reading color values into a structure of 3 vectors of 16 8-bit entries
          //vld3 has a stride of 3, automatically separating entries into BGR
	  BGRvector =  vld3_u8(inPixels);
          //Turning BGR vector into brightness by adding them together post-scaling
          brightVector = vmull_u8(BGRvector.val[0], w_r);
          //multiply-accumulate-widen
          brightVector = vmlal_u8(brightVector, BGRvector.val[1], w_g);
          brightVector = vmlal_u8(brightVector, BGRvector.val[2], w_b);
          result = vshrn_n_u16(brightVector, 8);
          //Storing converted pixels in grey image
          vst1_u8(greyPixels, result);
	}
}

void to442_sobel(Mat *imgray, Mat *imsobel, int grayStart, int start, int stop)
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
	uchar *greyPixels = (uchar*) imgray->data + start;
	uchar *sobelPixels = (uchar*) imsobel->data + start;

	int8x8_t topL, topM, topR, midL,  midR, botL, botM, botR;
	uint16x8_t sresult;
	uint16x8_t overflow = vdupq_n_u16(255);

	//for loops iterating thru the image pixel-by-pixel
	for (int index = start; index<((stop+1)); index+=8, greyPixels+=8, sobelPixels+=8)
        {
		topL = vld1_u8((const uint8_t *)greyPixels);
      		topM = vld1_u8((const uint8_t *)greyPixels+1);
      		topR = vld1_u8((const uint8_t *)greyPixels+2);

      		midL = vld1_u8((const uint8_t *)(greyPixels+imgray->cols));
      		midR = vld1_u8((const uint8_t *)(greyPixels+imgray->cols + 2));

      		botL = vld1_u8((const uint8_t *)(greyPixels + 2*imgray->cols));
      		botM = vld1_u8((const uint8_t *)(greyPixels + 2*imgray->cols+1));
      		botR = vld1_u8((const uint8_t *)(greyPixels + 2*imgray->cols+2));

      		sresult = vaddq_u16(
                        vabsq_s16(
                                  vaddq_u16(
                        vsubq_u16(vaddl_u8(topL, topR), vaddl_u8(botL, botR)),
                        vsubq_u16(vshll_n_u8(topM, 1), vshll_n_u8(botM, 1)))),
                         vabsq_s16(
                                   vaddq_u16(
                        vsubq_u16(vaddl_u8(topR, botR), vaddl_u8(topL, botL)),
                        vsubq_u16(vshll_n_u8(midR, 1), vshll_n_u8(midL,1)))));

   sresult = vminq_u16(sresult, overflow);

   vst1_u8(sobelPixels, vmovn_u16(sresult));

	}

        return;

}

void *grobel(void *args)
{
    //cast arguments passed in back to struct 
    threadArgs temp = *((threadArgs*)args);
    
    while(!temp.input->empty())
    {

        //create pointer for grayscale image
        //Mat grayTemp(temp.input->rows, temp.input->cols, CV_8UC1);

        //convert to grayscale
        to442_grayscale(temp.input, temp.grayTemp, temp.start1, temp.stop1);

        //wait for threads to finish grayscale of image
        pthread_barrier_wait(&grayBarrier);
      
        //apply sobel filter
        to442_sobel(temp.grayTemp, temp.output, temp.start1, temp.start2, temp.stop2);

        //wait for threads to finish sobel filtering the image
        pthread_barrier_wait(&sobelBarrier);
    
    }

    return NULL;
}

