#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace std;

template<class T> class Image
{
  private:
  IplImage* imgp;
  public:
  Image(IplImage* img=0) {imgp=img;}
  ~Image(){imgp=0;}
  void operator=(IplImage* img) {imgp=img;}
  inline T* operator[](const int rowIndx) {
    return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));}
};

typedef struct{
  unsigned char b,g,r;
} RgbPixel;

typedef struct{
  float b,g,r;
} RgbPixelFloat;

typedef Image<RgbPixel>       RgbImage;
typedef Image<RgbPixelFloat>  RgbImageFloat;
typedef Image<unsigned char>  BwImage;
typedef Image<float>          BwImageFloat;


inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels )
{
  if ( *img != NULL ) return;

   *img = cvCreateImage( size, depth, channels );
        if ( *img == NULL )
            {
                  fprintf(stderr, "Error: Couldn't allocate image.  Out of memory?\n");
                      exit(-1);
       }
}

IplImage * movement_filter(IplImage * frame1, IplImage * frame2){

  if ( frame1->width != frame2->width || frame1->height != frame2->height ){
    cout << "Diferent Sizes!" << endl;
    exit(-1);
  }

  IplImage * frame1b=cvCreateImage(cvSize(frame1->width, frame1->height), IPL_DEPTH_8U,1);
  IplImage * frame2b=cvCreateImage(cvSize(frame2->width, frame2->height), IPL_DEPTH_8U,1);
  IplImage * ret=cvCreateImage(cvSize(frame2->width, frame2->height), IPL_DEPTH_8U,1);

  int step = frame1->widthStep;
  uchar* data    = (uchar *)ret->imageData;

  for ( int i = 0; i < frame1->height; i++){
    for (int j = 0; j < frame1->width; j++){

      uchar cf1 =  (frame1->imageData + i*step)[j];
      uchar cf2 =  (frame2->imageData + i*step)[j];

      if ( abs(cf1-cf2) < 25 ){
        (ret->imageData + i*step)[j] = 0;
        }
      else
        (ret->imageData + i*step)[j] = cf1;
    }
  }
  return ret;
}

int main(void){
  IplImage * frame = NULL, * motion = NULL, * fbn = NULL, *frame2 = NULL,  *tmp2 = NULL, *tmp1 = NULL, *cp=NULL;

  CvCapture *pCapturedImage = cvCreateCameraCapture(0);

  cvNamedWindow("GoodFeatures",CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Motion", CV_WINDOW_AUTOSIZE);
  
  frame = cvQueryFrame(pCapturedImage);

  CvSize frame_size;
  frame_size.width = frame->width;
  frame_size.height = frame->height;

  allocateOnDemand( &frame2, frame_size, IPL_DEPTH_8U, 1 );
  allocateOnDemand( &fbn, frame_size, IPL_DEPTH_8U, 1 );

  while(cvWaitKey(10)){

    allocateOnDemand(&motion, frame_size, IPL_DEPTH_8U, 1);
    cvConvertImage(frame, motion, 0);

    frame = cvQueryFrame(pCapturedImage);

    cvConvertImage(frame, frame2, 0);

    cvConvertImage(frame, fbn, 0);

//    cvShowImage("Motion", movement_filter(motion,fbn));

    CvPoint2D32f frame1_features[300];

    int nfeat = 300;

    cvGoodFeaturesToTrack( movement_filter(motion,fbn), tmp1, tmp2, frame1_features, &nfeat, .01, .01, NULL);

    for( int i = 0; i < nfeat; i++){
      CvPoint p;
      p.x = (int) frame1_features[i].x;
      p.y = (int) frame1_features[i].y;
      cvCircle(frame2, p, 2, CV_RGB(255, 0, 0), -1);
    }

    cvShowImage("GoodFeatures", frame2);
  }
  //cvWaitKey();
}
