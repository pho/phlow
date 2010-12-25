#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <iostream>
#include <X11/Xlib.h>
#include <X11/keysym.h>
//#include <X11/extensions/XTest.h>


using namespace std;

#define NFEAT 400

static const double pi = 3.14159265358979323846;

inline static double square(int a){
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

IplImage * movement_filter(IplImage * frame1, IplImage * frame2){

  if ( frame1->width != frame2->width || frame1->height != frame2->height ){
    cout << "Diferent Sizes!" << endl;
    exit(-1);
  }

  IplImage * frame1b=cvCreateImage(cvSize(frame1->width, frame1->height), IPL_DEPTH_8U,1);
  IplImage * frame2b=cvCreateImage(cvSize(frame2->width, frame2->height), IPL_DEPTH_8U,1);
  IplImage * ret=cvCreateImage(cvSize(frame2->width, frame2->height), IPL_DEPTH_8U,1);

  int step = frame1->widthStep;

  for ( int i = 0; i < frame1->height; i++){
    for (int j = 0; j < frame1->width; j++){

      uchar cf1 =  (frame1->imageData + i*step)[j];
      uchar cf2 =  (frame2->imageData + i*step)[j];

      if ( abs(cf1-cf2) < 15 ){
        (ret->imageData + i*step)[j] = 0;
      }
      else
        (ret->imageData + i*step)[j] = cf1;
    }
  }
  cvReleaseImage(&frame1b);
  cvReleaseImage(&frame2b);
  return ret;
}


int main(void){
  IplImage * frame1 = NULL, * frame1_color=NULL, * frame1b=NULL, *frame2 = NULL, *frame2b = NULL,  *tmp2 = NULL, *tmp1 = NULL, *cp=NULL,
           * pyramid1 = NULL, *pyramid2= NULL, *motion = NULL;

  CvCapture *pCapturedImage = cvCreateCameraCapture(0);

  if (pCapturedImage == NULL)
  {
    /* Either the video didn't exist OR it uses a codec OpenCV
     *     * doesn't support.
     *         */
    fprintf(stderr, "Error: Can't open video.\n");
    return -1;
  }
  cvNamedWindow("GoodFeatures",CV_WINDOW_AUTOSIZE);
  cvNamedWindow("Motion",CV_WINDOW_AUTOSIZE);

  cvSetMouseCallback("GoodFeatures", mouseCallback, NULL);

  frame1 = cvQueryFrame(pCapturedImage);
  CvSize frame_size;
  frame_size.width = frame1->width;
  frame_size.height = frame1->height;

  allocateOnDemand( &frame1_color, frame_size, IPL_DEPTH_8U, 3 );
  allocateOnDemand( &frame1b, frame_size, IPL_DEPTH_8U, 1 );

  allocateOnDemand( &frame2b, frame_size, IPL_DEPTH_8U, 1 );
  allocateOnDemand( &motion, frame_size, IPL_DEPTH_8U, 1);

  //Storage for the Shi and Tomasi Algorithm
  allocateOnDemand( &tmp1, frame_size, IPL_DEPTH_32F, 1);
  allocateOnDemand( &tmp2, frame_size, IPL_DEPTH_32F, 1);

  allocateOnDemand( &pyramid1, frame_size, IPL_DEPTH_8U, 1);
  allocateOnDemand( &pyramid2, frame_size, IPL_DEPTH_8U, 1);


  while(cvWaitKey(10)){
    int nfeat = NFEAT;
    CvPoint2D32f frame1_features[NFEAT];
    CvPoint2D32f frame1_features_backup[NFEAT];
    
    cvConvertImage(frame1, frame2b, 0);
    frame1 = cvQueryFrame(pCapturedImage);

    cvConvertImage(frame1, frame1_color, 0);
    cvConvertImage(frame1, frame1b, 0);

    //frame2 = cvQueryFrame(pCapturedImage);

    motion = movement_filter(frame2b, frame1b);

    cvSetImageROI(frame2b, ROI);
    cvGoodFeaturesToTrack(frame2b, tmp1, tmp2, frame1_features, &nfeat, .01, .01, NULL);
    cvResetImageROI(frame2b);

    CvPoint2D32f frame2_features[NFEAT];
    char optical_flow_found_feature[NFEAT];
    float optical_flow_feature_error[NFEAT];
    CvSize optical_flow_window = cvSize(3,3);
    CvTermCriteria optical_flow_termination_criteria = cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3);


    cvSetImageROI(frame2b, ROI);
    cvSetImageROI(frame1b, ROI);
    cvCalcOpticalFlowPyrLK(frame2b, frame1b, pyramid1, pyramid2, frame1_features, frame2_features, nfeat, optical_flow_window, 5, optical_flow_found_feature, optical_flow_feature_error,optical_flow_termination_criteria, 0);


    /* For fun (and debugging :)), let's draw the flow field. */

    int up=0, down=0, left=0, right=0, NewCenterX = 0, NewCenterY = 0, f2featfound=0;

    for(int i = 0; i < NFEAT; i++){

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
      p.x = (int) frame2_features[i].x + ROI.x;
      p.y = (int) frame2_features[i].y + ROI.y;
      q.x = (int) frame1_features[i].x + ROI.x;
      q.y = (int) frame1_features[i].y + ROI.y;

      int motionp = (int) (motion->imageData + p.y*motion->widthStep)[p.x];
      int motionq = (int) (motion->imageData + q.y*motion->widthStep)[q.x];
      
      if (motionp == 0 && motionq == 0)
        continue;


      if (q.x - p.x > 10) right++;
      else if (p.x - q.x > 10) left++;

      if  (q.y - p.y > 10) up++;
      else if (p.y - q.y > 10)down++;
      // New ROI Center

      f2featfound++;
      NewCenterX += q.x;
      NewCenterY += q.y;


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
    }

    cvResetImageROI(frame2b);
    cvResetImageROI(frame1b);

    cvRectangle(frame1_color, cvPoint(ROI.x, ROI.y), cvPoint(ROI.x+ROI.width,ROI.y+ROI.height), cvScalar(255,0,0), 1);

    if ( f2featfound > 30){
      //Update ROI
      cout << NewCenterX << " " <<  f2featfound << " " << NewCenterY << endl;
      cout << "NewCenter: " << NewCenterX/f2featfound << " " << NewCenterY/f2featfound << " " << f2featfound << endl;
      ROI.x = NewCenterX/f2featfound - ROI.width/2;
      ROI.y = NewCenterY/f2featfound - ROI.height/2;

      cout << "Next ROI " << ROI.x << " " << ROI.y << endl;
      if (ROI.x < 0) ROI.x = 0;
      if (ROI.x > frame1_color->width) ROI.x = frame1_color->width-1;
      if (ROI.y > frame1_color->height) ROI.y = frame1_color->height-1;
      if (ROI.y < 0) ROI.y = 0;
      cout << "Next ROI Secured" << ROI.x << " " << ROI.y << endl << endl;

      //Draw NewCenter
      CvPoint center;
      center.x = ROI.x+ROI.width/2;
      center.y = ROI.y+ROI.height/2;
      cvCircle(frame1_color, center, 5, CV_RGB(0, 0, 255), -1);
    }

    cvShowImage("GoodFeatures", frame1_color);
    cvShowImage("Motion", motion);

  }// EndWhile
}
