#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/XTest.h>


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

CvRect ROI = cvRect(100, 100, 200, 200);
CvRect tempROI = cvRect(100, 100, 200, 200);

void mouseCallback(int event, int x, int y, int flags, void *param){

  switch(event){
    case CV_EVENT_LBUTTONDOWN:
      tempROI.x = x;
      tempROI.y = y;
      cout << "x: " << x << " y: " << y << endl;
      break;
    case CV_EVENT_LBUTTONUP:
      ROI.x = tempROI.x;
      ROI.y = tempROI.y;
      ROI.width = x-tempROI.x;
      ROI.height = y-tempROI.y;
      cout << "w: " << x-tempROI.x << " h: " << y-tempROI.y << endl;
      break;
  }

}


int main(void){
  IplImage * frame1 = NULL, * frame1_color=NULL, * frame1b=NULL, *frame2 = NULL, *frame2b = NULL,  *tmp2 = NULL, *tmp1 = NULL, *cp=NULL,
           * pyramid1 = NULL, *pyramid2= NULL;

  CvCapture *pCapturedImage = cvCreateCameraCapture(0);

  cvNamedWindow("GoodFeatures",CV_WINDOW_AUTOSIZE);

  cvSetMouseCallback("GoodFeatures", mouseCallback, NULL);


  while(cvWaitKey(10)){


    frame1 = cvQueryFrame(pCapturedImage);


    CvSize frame_size;
    frame_size.width = frame1->width;
    frame_size.height = frame1->height;

    allocateOnDemand( &frame1_color, frame_size, IPL_DEPTH_8U, 3 );
    cvConvertImage(frame1, frame1_color, 0);
    
    allocateOnDemand( &frame1b, frame_size, IPL_DEPTH_8U, 1 );
    cvConvertImage(frame1, frame1b, 0);

    int nfeat = 400;

    CvPoint2D32f frame1_features[400];

    cvSetImageROI(frame1b, ROI);
    cvGoodFeaturesToTrack(frame1b, tmp1, tmp2, frame1_features, &nfeat, .01, .01, NULL);
    cvResetImageROI(frame1b);


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
    
    cvSetImageROI(frame2b, ROI);
    cvSetImageROI(frame1b, ROI);
    cvCalcOpticalFlowPyrLK(frame1b, frame2b, pyramid1, pyramid2, frame1_features, frame2_features, nfeat, optical_flow_window, 5, optical_flow_found_feature, optical_flow_feature_error,optical_flow_termination_criteria, 0);
    

/* For fun (and debugging :)), let's draw the flow field. */

      int up=0, down=0, left=0, right=0;

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
              p.x = (int) frame1_features[i].x + ROI.x;
              p.y = (int) frame1_features[i].y + ROI.y;
              q.x = (int) frame2_features[i].x + ROI.x;
              q.y = (int) frame2_features[i].y + ROI.y;

              if (q.x - p.x > 10) right++;
              else if (p.x - q.x > 10) left++;

              if  (q.y - p.y > 10) up++;
              else if (p.y - q.y > 10)down++;


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
              cvLine( frame1_color, p, q, line_color, line_thickness, CV_AA, 0 );
              /* Now draw the tips of the arrow.  I do some scaling so that the
               * tips look proportional to the main line of the arrow.
               */
              p.x = (int) (q.x + 9 * cos(angle + pi / 4));
              p.y = (int) (q.y + 9 * sin(angle + pi / 4));
              cvLine( frame1_color, p, q, line_color, line_thickness, CV_AA, 0 );
              p.x = (int) (q.x + 9 * cos(angle - pi / 4));
              p.y = (int) (q.y + 9 * sin(angle - pi / 4));
              cvLine( frame1_color, p, q, line_color, line_thickness, CV_AA, 0 );
  
      }

      CvFont font;
      double hScale=1.0;
      double vScale=1.0;
      int    lineWidth=2;
      cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, hScale,vScale,0,lineWidth);

      if (up > down and up > 30) {
        cvPutText (frame1_color,"Down",cvPoint(100,400), &font, cvScalar(255,255,0));
        cout << "DOWN!" << endl;
      }
      else if(down > 30) cvPutText (frame1_color,"Up",cvPoint(100,400), &font, cvScalar(255,255,0));

      if (left > right and left > 30) cvPutText (frame1_color,"Right",cvPoint(300,400), &font, cvScalar(255,255,0));
      else if(right > 30){
        cvPutText (frame1_color,"Left",cvPoint(300,400), &font, cvScalar(255,255,0));
        Display *display;
        unsigned int keycode;
        display = XOpenDisplay(NULL);
        keycode = XKeysymToKeycode(display, XK_Down);
        XTestFakeKeyEvent(display, keycode, True, 0);
        XTestFakeKeyEvent(display, keycode, False, 0);
        XFlush(display);
      }

      cvResetImageROI(frame2b);
      cvResetImageROI(frame1b);

      cvRectangle(frame1_color, cvPoint(ROI.x, ROI.y), cvPoint(ROI.x+ROI.width,ROI.y+ROI.height), cvScalar(255,0,0), 1);



   /*for( int i = 0; i < nfeat; i++){
      CvPoint p;
      p.x = (int) frame1_features[i].x;
      p.y = (int) frame1_features[i].y;
      cvCircle(frame, p, 2, CV_RGB(255, 0, 0), -1);
    }*/

    cvShowImage("GoodFeatures", frame1_color);
  }
  //cvWaitKey();
}
