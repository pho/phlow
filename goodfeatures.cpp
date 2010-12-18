#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>

using namespace std;


static const double pi = 3.14159265358979323846;

inline static double square(int a)
{
          return a * a;
}



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
  IplImage * frame1 = NULL, * frame1_color=NULL, * frame1b=NULL, *frame2 = NULL, *frame2b = NULL,  *tmp2 = NULL, *tmp1 = NULL, *cp=NULL,
           * pyramid1 = NULL, *pyramid2= NULL;

  CvCapture *pCapturedImage = cvCreateCameraCapture(0);

  cvNamedWindow("GoodFeatures",CV_WINDOW_AUTOSIZE);

  frame1 = cvQueryFrame(pCapturedImage);
  CvPoint2D32f frame1_features[300];

  while(cvWaitKey(10)){
    
    CvSize frame_size;
    frame_size.width = frame1->width;
    frame_size.height = frame1->height;

    allocateOnDemand( &frame1b, frame_size, IPL_DEPTH_8U, 1 );
    cvConvertImage(frame1, frame1b, 0);

    int nfeat = 300;

    cvGoodFeaturesToTrack(frame1b, tmp1, tmp2, frame1_features, &nfeat, .01, .01, NULL);

    frame2 = cvQueryFrame(pCapturedImage);
    allocateOnDemand( &frame2b, frame_size, IPL_DEPTH_8U, 1 );
    cvConvertImage(frame2, frame2b, 0);
    

    //Storage for the Shi and Tomasi Algorithm
    allocateOnDemand( &tmp1, frame_size, IPL_DEPTH_32F, 1);
    allocateOnDemand( &tmp2, frame_size, IPL_DEPTH_32F, 1);
    
    CvPoint2D32f frame2_features[400];

    char optical_flow_found_feature[400];

    float optical_flow_feature_error[400];

    CvSize optical_flow_window = cvSize(3,3);

    CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3);

    allocateOnDemand( &pyramid1, frame_size, IPL_DEPTH_8U, 1);
    allocateOnDemand( &pyramid2, frame_size, IPL_DEPTH_8U, 1);
    

    cvCalcOpticalFlowPyrLK(frame1b, frame2b, pyramid1, pyramid2, frame1_features, frame2_features, nfeat, optical_flow_window, 5, optical_flow_found_feature, optical_flow_feature_error,optical_flow_termination_criteria, 0);


/* For fun (and debugging :)), let's draw the flow field. */
     for(int i = 0; i < nfeat; i++)
      {
              /* If Pyramidal Lucas Kanade didn't really find the feature, skip it. */
              if ( optical_flow_found_feature[i] == 0 )       continue;

              int line_thickness;                             line_thickness = 1;
              /* CV_RGB(red, green, blue) is the red, green, and blue components
               * of the color you want, each out of 255.
               */
              CvScalar line_color;                    line_color = CV_RGB(255,0,0);

              /* Let's make the flow field look nice with arrows. */

              /* The arrows will be a bit too short for a nice visualization because of the high framerate
               * (ie: there's not much motion between the frames).  So let's lengthen them by a factor of 3.
               */
              CvPoint p,q;
              p.x = (int) frame1_features[i].x;
              p.y = (int) frame1_features[i].y;
              q.x = (int) frame2_features[i].x;
              q.y = (int) frame2_features[i].y;

              double angle;           angle = atan2( (double) p.y - q.y, (double) p.x - q.x );
              double hypotenuse;      hypotenuse = sqrt( square(p.y - q.y) + square(p.x - q.x) );

              /* Here we lengthen the arrow by a factor of three. */
              q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
              q.y = (int) (p.y - 3 * hypotenuse * sin(angle));

              /* Now we draw the main line of the arrow. */
              /* "frame1" is the frame to draw on.
               * "p" is the point where the line begins.
               * "q" is the point where the line stops.
               * "CV_AA" means antialiased drawing.
               * "0" means no fractional bits in the center cooridinate or radius.
               */
              cvLine( frame1b, p, q, line_color, line_thickness, CV_AA, 0 );
              /* Now draw the tips of the arrow.  I do some scaling so that the
               * tips look proportional to the main line of the arrow.
               */
              p.x = (int) (q.x + 9 * cos(angle + pi / 4));
              p.y = (int) (q.y + 9 * sin(angle + pi / 4));
              cvLine( frame1b, p, q, line_color, line_thickness, CV_AA, 0 );
              p.x = (int) (q.x + 9 * cos(angle - pi / 4));
              p.y = (int) (q.y + 9 * sin(angle - pi / 4));
              cvLine( frame1b, p, q, line_color, line_thickness, CV_AA, 0 );
      }


   /*for( int i = 0; i < nfeat; i++){
      CvPoint p;
      p.x = (int) frame1_features[i].x;
      p.y = (int) frame1_features[i].y;
      cvCircle(frame, p, 2, CV_RGB(255, 0, 0), -1);
    }*/

    cvShowImage("GoodFeatures", frame1b);
  }
  //cvWaitKey();
}
