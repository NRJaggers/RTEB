//Group: Nima Behmanesh, Nathan Jaggers, Joshua Rizzolo
//Youtube Demo link: https://youtu.be/c75YS3goRbs

#include<arm_neon.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>

using namespace std;
using namespace cv;


void* grobel(void* input);

//Prototyping threads
pthread_t thread[4]; //4 Threads for 4

//Barriers for synchronization
pthread_barrier_t processingBarrier, frameGetBarrier;

//arguments to be passed to child threads
typedef struct threadArgs{
  Mat *inputFrame;
  Mat *outputFrame;
  int height;
  int width;
  int startind;
  int stopind;
}ArgStruct;




int main(){

  //Loading vid and making a mat for each frame
  VideoCapture cap("dab.mp4");
  Mat frame;

  //Naming threads
  int top, umid, lmid, bot, chunkSize, lastInd;

  //If no vid can be loaded, exit main
  if(!cap.isOpened()){
    cout << "Error reading video file" << endl;
    return -1;
  }

  //load one frame of video into the mat
  cap >> frame;

  //instantiating output frame of same size as input image but with only one color channel (and starting it as all black)
  Mat grobelFrame = Mat(frame.rows, frame.cols, CV_8UC1, Scalar::all(0));
  
  //Getting data by which to split the mat into 4 chunks, one per thread
  lastInd = frame.rows * frame.cols;
  chunkSize = (int) lastInd/4;

  //instantiating an argument struct for each thread with appropriate start and stop indices
  //Sobel function requires padding above and below the center pixel, so give each thread two extra rows to greyscale so it finishes greyscaling everything needed for sobel
  //without chunkSize - frame.cols and chunkSize + frame.cols sobel sees a hard black line below it's last center pixels
  ArgStruct threadArgs0={.inputFrame = &frame, .outputFrame = &grobelFrame, .height = frame.rows, .width = frame.cols, .startind = 0, .stopind = chunkSize+frame.cols};
  ArgStruct threadArgs1={.inputFrame = &frame, .outputFrame = &grobelFrame, .height = frame.rows, .width = frame.cols, .startind = chunkSize-frame.cols, .stopind = 2*chunkSize+frame.cols};
  ArgStruct threadArgs2={.inputFrame = &frame, .outputFrame = &grobelFrame, .height = frame.rows, .width = frame.cols, .startind = 2*chunkSize-frame.cols, .stopind = 3*chunkSize+frame.cols};
  ArgStruct threadArgs3={.inputFrame = &frame, .outputFrame = &grobelFrame, .height = frame.rows, .width = frame.cols, .startind = 3*chunkSize-frame.cols, .stopind = lastInd-1};


  //instantiating barriers to wait for 5 threads: 4 child threads and main
  pthread_barrier_init(&processingBarrier, NULL, 5); //Last arg is numThreads
  pthread_barrier_init(&frameGetBarrier, NULL, 5);

 
  //Starting individual threads and passing pointers to their respective argument structures
  top= pthread_create(&thread[0], NULL, grobel,  (void*) &threadArgs0);
    
  umid = pthread_create(&thread[1], NULL, grobel,  (void*) &threadArgs1);
    
  lmid = pthread_create(&thread[2], NULL, grobel,  (void*) &threadArgs2);
    
  bot = pthread_create(&thread[3], NULL, grobel, (void*) &threadArgs3);

  
  
  //Main loop loads frames into Mat, waits for child threads to finish processing,  gets next frame, and shows the finished previous frame
  while(1){

    
    //Already have first frame, wait for it to finish processing
    pthread_barrier_wait(&processingBarrier);


    //load new frame into mat and make sure it's not empty
    cap >> frame;
    if (frame.empty())
      break;

    //Show finished frame
    imshow("Sobel Image", grobelFrame);
    
    //Wait for user to press esc, and close video when they do
    char c = (char)waitKey(2);
    if (c==27)
      break;

    //wait to work on new frame until the threads are all ready
    pthread_barrier_wait(&frameGetBarrier);
  }

  //Close the video file
  cap.release();
  
  //Close all image windows
  destroyAllWindows();
  
  //Kill threads
  pthread_join(top, NULL);
  pthread_join(umid, NULL);
  pthread_join(lmid, NULL);
  pthread_join(bot, NULL);
  //Finish Main
  return 0;
  
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//Greyscale + sobel function
void* grobel(void* threadArgs){

  //casting void * back to struct * to access struct elements again
  ArgStruct *args = (ArgStruct *)threadArgs;

  //creating greyscale mat of same size as input image
  Mat grey(args->height,args->width, CV_8UC1);

  int8x8_t topL, topM, topR, midL,  midR, botL, botM, botR;
  uint8x8_t w_r = vdup_n_u8(77);
  uint8x8_t w_g = vdup_n_u8(150);
  uint8x8_t w_b = vdup_n_u8(29);
  uint8x8x3_t BGRvector;
  uint16x8_t brightVector;
  uint8x8_t result;
  uint16x8_t sresult;
  uint16x8_t overflow = vdupq_n_u16(255);
  
  while(1){
    
    
    //Getting pointers to Mat pixel data for input and grey images
    const uchar *inPixels =  args->inputFrame->data + args->startind*3;
    uchar *greyPixels = grey.data + args->startind;
    uchar* sobelPixels = args->outputFrame->data + args->startind;
    
    //Iterate thru pixel data to last index, incrementing by length of vector
    for(int index = args->startind; index<(args->stopind); index+=8, greyPixels+=8, inPixels+=8 * 3){
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
    
    
    
    //iterating thru 1D array of grey image pixel values to perform sobel on them
    
    greyPixels = grey.data + args->startind;
    
    for(int ind = args->startind + grey.cols;  ind<(args->stopind - grey.cols); greyPixels+=8, sobelPixels +=8, ind+=8){

      topL = vld1_u8((const uint8_t *)greyPixels);
      topM = vld1_u8((const uint8_t *)greyPixels+1);
      topR = vld1_u8((const uint8_t *)greyPixels+2);

      midL = vld1_u8((const uint8_t *)(greyPixels+grey.cols));
      midR = vld1_u8((const uint8_t *)(greyPixels+grey.cols + 2));
   
      botL = vld1_u8((const uint8_t *)(greyPixels + 2*grey.cols));
      botM = vld1_u8((const uint8_t *)(greyPixels + 2*grey.cols+1));
      botR = vld1_u8((const uint8_t *)(greyPixels + 2*grey.cols+2));

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
    //wait here once done processing current frame before getting new frame
    pthread_barrier_wait(&processingBarrier);
    
    //wait here while getting the new frame so as not to access memory from multiple threads at once
    pthread_barrier_wait(&frameGetBarrier);
  }
  return NULL;
}

