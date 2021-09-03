#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
 
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std;
using namespace cv;
 
#define CAM_WIDTH 640 
#define CAM_HEIGHT 480
#define BUFSIZE 2048 

#define SERVER_URL "114.70.21.95/temp/"
#define SERVER_IP "114.70.21.95"
#define SERVER_PORT 8888
#define MOBIUS_IP "114.70.21.85"
#define MOBIUS_PORT 7622

// func prototype
Mat detectAndDisplay(Mat frame);
void imageSend(char *srcfile);
void urlSend(char *srcfile);
char * timeToString(struct tm *t);

// Undistortion variables
double dCamMat[] = {211.287, 0, 286.364, 0, 205.357, 249.240, 0, 0, 1};
double dDstMat[] = {-0.261287, 0.042950, -0.000123, 0.004093};
Mat camMatrix = Mat(3,3,CV_64FC1, (void *)dCamMat);
Mat distMatrix = Mat(1,4,CV_64FC1, (void *)dDstMat);

// global variables. 
String cascade_name;
CascadeClassifier cascade;
bool flag = false;
int flag_isSend = 1;
Mat src, dst;

// main
int main(int argc, const char** argv)
{
    cascade_name = "/usr/local/share/OpenCV/haarcascades/haarcascade_fullbody.xml";
    if (!cascade.load(cascade_name)) { 
        printf("--(!)Error loading cascade\n"); 
        return -1; 
    }
 
    VideoCapture cam(0); 
    Mat frame, output;
    char savefile[128] = {0,};
    int snapCnt = 0;
    time_t ctime = 0;
    struct tm *t = NULL;

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
        output = detectAndDisplay(frame);
        imshow("", output);
        char c = (char)waitKey(10);
        switch(c){ 
            case 27: // esc
                goto exit;
                break;
            case 32: // space
                ctime = time(NULL);
                t = localtime(&ctime);
                sprintf(savefile, "img_%s.jpg", timeToString(t));
                imwrite(savefile, output);
                imageSend(savefile);
                urlSend(savefile);
                snapCnt++;
                break;
        }
    }
exit:
    return 0;
}
 
// detectAndDisplay
Mat detectAndDisplay(Mat frame)
{
    std::vector<Rect> objects;
    Mat rot, tmp;
    undistort(frame, src, camMatrix, distMatrix);
    cascade.detectMultiScale(src, objects, 1.1, 1, 0 | CASCADE_SCALE_IMAGE, Size(60, 120));
    if(objects.size()>0){
        for(int i=0; i<objects.size(); i++){
            if(objects[i].width > 150 && objects[i].height > 300){
                tmp = src(objects[i]);
                rotate(tmp, dst, ROTATE_90_CLOCKWISE);
                flag = true;
//                rectangle(src, objects[i], Scalar(0,0,255),4,8,0);
//                rotate(src, dst, ROTATE_90_CLOCKWISE);
//                flag = true;
                break;
            }
        }
    }
    if(flag==false){
        return src;
    }else{
        return dst;
    }
}

void imageSend(char *srcfile){

    int totalSize=0;
    int ret=0;  
    char buf[BUFSIZE];
    FILE *fp=NULL;
    int client_socket=0;
    struct sockaddr_in server_addr;
    int rdata=0, sum=0;

    if(flag_isSend){
        // connect socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	ret = connect(client_socket, (struct sockaddr* )&server_addr, sizeof(server_addr));	
        if(ret==-1)
            printf("connect error\n");    
        
        //sendfile open
        fp = fopen(srcfile, "rb");
	if(fp == NULL) 
            printf("fopen error\n");
        
	//send filename
        ret = send(client_socket, srcfile, 256, 0);
        usleep(500);
        if(ret == -1) 
            printf("send filename error\n");

	//get filesize
        fseek(fp, 0, SEEK_END);
        totalSize = ftell(fp);

	//send filesize
	ret = send(client_socket, (char *)&totalSize, sizeof(totalSize), 0);
	if(ret == -1) 
            printf("send filesize error\n");
	usleep(500);
	rewind(fp);
        // file block send
	while(1) {
	    rdata = fread(buf, 1, BUFSIZE, fp);
            if(rdata > 0) {
	        ret = send(client_socket, buf, rdata, 0);
	        usleep(500);
	        if(ret == -1) 
                    printf("send file error\n");
	        sum += rdata;
	    }
	    else if(rdata == 0 && sum == totalSize) {
		printf("Sshot was passed\n");
		break;
	    }
	    else {
	        printf("I/O error\n");
		break;
            } //if end
	}//while
	fclose(fp);
    }//flag
    close(client_socket); 
}

void urlSend(char *srcfile){

    int ret = 0;
    char url[128] = {'\0',};
    char msg[256] = {'\0',};
    int client_socket = 0;
    struct sockaddr_in mobius_addr;

    if(flag_isSend){
        client_socket = socket(PF_INET, SOCK_STREAM, 0);
        memset(&mobius_addr, 0, sizeof(mobius_addr));
        mobius_addr.sin_family = AF_INET;
        mobius_addr.sin_port = htons(MOBIUS_PORT);
        mobius_addr.sin_addr.s_addr = inet_addr(MOBIUS_IP);
        
        // connect
        ret = connect(client_socket, (struct sockaddr*)&mobius_addr, sizeof(mobius_addr));
        if(ret < 0){
            printf("connect error\n");
        }
        
        // msg formatting
        sprintf(url, "%s%s", SERVER_URL, srcfile);
        sprintf(msg,                 \
                "{\n"                  \
                "\"ctname\" : \"odroid\",\n"  \
                "\"con\" : \"%s\"\n"           \
                "}", url);
        printf("%s\n", msg);

        // send message
        ret = send(client_socket, msg, 256, 0);
        usleep(500);
        if(ret < 0){
            printf("send error\n");
        }
    }
    close(client_socket);
}

char * timeToString(struct tm *t){
    
    static char str[20];
    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    return str;
}
