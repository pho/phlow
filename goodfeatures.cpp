#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace std;


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



int main(void){
  IplImage * frame = NULL, *frame2 = NULL,  *tmp2 = NULL, *tmp1 = NULL, *cp=NULL;

  CvCapture *pCapturedImage = cvCreateCameraCapture(0);

  cvNamedWindow("GoodFeatures",CV_WINDOW_AUTOSIZE);

  while(cvWaitKey(10)){
  frame = cvQueryFrame(pCapturedImage);

  CvSize frame_size;
  frame_size.width = frame->width;
  frame_size.height = frame->height;


  allocateOnDemand( &frame2, frame_size, IPL_DEPTH_8U, 1 );
  cvConvertImage(frame, frame2, 0);

  CvPoint2D32f frame1_features[300];

  int nfeat = 300;

  cvGoodFeaturesToTrack(frame2, tmp1, tmp2, frame1_features, &nfeat, .01, .01, NULL);

  for( int i = 0; i < nfeat; i++){
    CvPoint p;
    p.x = (int) frame1_features[i].x;
    p.y = (int) frame1_features[i].y;
    cvCircle(frame, p, 2, CV_RGB(255, 0, 0), -1);
  }

  cvShowImage("GoodFeatures", frame);
  }
  //cvWaitKey();
}
