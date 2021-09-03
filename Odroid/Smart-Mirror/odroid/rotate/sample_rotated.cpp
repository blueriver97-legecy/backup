#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
 
#include <iostream>
#include <stdio.h>
 
using namespace std;
using namespace cv;
 
#define CAM_WIDTH 320
#define CAM_HEIGHT 240
 
/** Function Headers */
void detectAndDisplay(Mat frame);

/** Global variables */
String cascade_name;
CascadeClassifier cascade;
Mat frame2, src, dst, rot;
bool flag = false;

/** @function main */
int main(int argc, const char** argv)
{
    cascade_name = "/usr/local/share/OpenCV/haarcascades/haarcascade_fullbody.xml";
    if (!cascade.load(cascade_name)) { 
        printf("--(!)Error loading cascade\n"); 
        return -1; 
    }
 
    VideoCapture cam(0); 
    Mat frame;

    cvNamedWindow("", CV_WINDOW_NORMAL);
    cvSetWindowProperty("", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
 
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, CAM_HEIGHT);
    cam.set(CV_CAP_PROP_FRAME_WIDTH, CAM_WIDTH);
 
    if (!cam.isOpened()) { 
        printf("--(!)Error opening video cam\n"); 
        return -1; 
    }
 
    while (cam.read(frame))
    {
        detectAndDisplay(frame);
        char c = (char)waitKey(10);
        if (c == 27) { break; } 
    }
    return 0;
}
 
/** @function detectAndDisplay */
void detectAndDisplay(Mat frame)
{
    //Point center;
    std::vector<Rect> objects;
 
    resize(frame, dst, Size(1024,600), 0, 0, CV_INTER_NN);
    cascade.detectMultiScale(dst, objects, 1.1, 1, 0 | CASCADE_SCALE_IMAGE, Size(60, 120));
/*
    Point center(frame.cols/2.0, frame.rows/2.0);
    rot = getRotationMatrix2D(center, -90, 1.0);
    //printf("rows = %d, cols = %d\n",frame.rows, frame.cols);

    warpAffine(frame, frame2, rot, frame.size());
*/
        
    if(objects.size()>0){
        src = frame(objects[0]);
        resize(src, frame2, Size(src.rows, src.cols), 0, 0, CV_INTER_NN);
        center.x = frame2.cols/2.0;
        center.y = frame2.rows/2.0;
        rot = getRotationMatrix2D(center, -90, 1.0);
        warpAffine(frame2, dst, rot, frame2.size());
        //printf("rows = %d, cols = %d, flag(2,1,dst) = %d %d %d\n",frame2.rows, frame2.cols, frame2.flags, frame.flags, dst.flags);
        flag = true;
    }    
    if(flag==false){
        center.x = frame.cols/2.0;
        center.y = frame.rows/2.0;
        rot = getRotationMatrix2D(center, -90, 1.0);
        warpAffine(frame, dst, rot, frame.size());
    }

    //printf("rows = %d, cols = %d\n",dst.rows, dst.cols);
    imshow("", frame2);
}

