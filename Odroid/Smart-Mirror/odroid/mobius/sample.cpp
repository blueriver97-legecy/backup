#include "opencv2/objdetect.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
 
#include <iostream>
#include <stdio.h>
#include <string.h>
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

#define SERVER_URL "114.70.21.89:9999/recevier/"
#define SERVER_IP "114.70.21.89"
#define SERVER_PORT 9998
#define MOBIUS_IP "114.70.21.85"
#define MOBIUS_PORT 7622

/** Function Headers */
Mat detectAndDisplay(Mat frame);
void imageSend(char *srcfile);
void urlSend(char *srcfile);

/** Undistortion */
double dCamMat[] = {211.287, 0, 286.364, 0, 205.357, 249.240, 0, 0, 1};
double dDstMat[] = {-0.261287, 0.042950, -0.000123, 0.004093};
Mat camMatrix = Mat(3,3,CV_64FC1, (void *)dCamMat);
Mat distMatrix = Mat(1,4,CV_64FC1, (void *)dDstMat);

/** Global variables */
String cascade_name;
CascadeClassifier cascade;
bool flag = false;
int flag_isSend = 1;
Mat src, dst;

/** @function main */
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
                sprintf(savefile, "img_%d.jpg", snapCnt);
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
 
/** @function detectAndDisplay */
Mat detectAndDisplay(Mat frame)
{
    std::vector<Rect> objects;
 
    undistort(frame, src, camMatrix, distMatrix);
    cascade.detectMultiScale(src, objects, 1.1, 1, 0 | CASCADE_SCALE_IMAGE, Size(60, 120));
    if(objects.size()>0){
        for(int i=0; i<objects.size(); i++){
            if(objects[i].width > 130 && objects[i].height > 260){
                dst = src(objects[i]);
                printf("cols=%d rows=%d\n", dst.cols, dst.rows);
                flag = true;
                break;
            }
        }
    }
    if(flag==false){
        //imshow("", dst);
        return src;
    }else{
        //imshow("", src);
        return dst;
    }
}

void imageSend(char *srcfile){

    int totalBytes=0;
    int retval=0;  
    char fileBuf_Send[BUFSIZE];
    FILE *fp_send=NULL;
    int client_socket=0;
    struct sockaddr_in server_addr;
    int numread=0, numtotal=0;

    if(flag_isSend){
        // connect socket
	client_socket = socket(PF_INET, SOCK_STREAM, 0);
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	retval = connect(client_socket, (struct sockaddr* )&server_addr, sizeof(server_addr));	
        if(retval==-1)
            printf("connect error\n");    
        
        //sendfile open
        fp_send = fopen(srcfile, "rb");
	if(fp_send == NULL) 
            printf("fopen error\n");
        
	//send filename
        retval = send(client_socket, srcfile, 256, 0);
        usleep(500);
        if(retval == -1) 
            printf("send filename error\n");

	//get filesize
        fseek(fp_send, 0, SEEK_END);
        totalBytes = ftell(fp_send);

	//send filesize
	retval = send(client_socket, (char *)&totalBytes, sizeof(totalBytes), 0);
	if(retval == -1) 
            printf("send filesize error\n");
	usleep(500);
	rewind(fp_send);
        // file block send
	while(1) {
	    numread = fread(fileBuf_Send, 1, BUFSIZE, fp_send);
            if(numread > 0) {
	        retval = send(client_socket, fileBuf_Send, numread, 0);
	        usleep(500);
	        if(retval == -1) 
                    printf("send file error\n");
	        numtotal += numread;
	    }
	    else if(numread == 0 && numtotal == totalBytes) {
		printf("Sshot was passed\n");
		break;
	    }
	    else {
	        printf("I/O error\n");
		break;
            } //if end
	}//while
	fclose(fp_send);
    }//flag
    close(client_socket); 
}

void urlSend(char *srcfile){

    int ret = 0;
    char url[128] = {'\0',};
    char msg[256] = {'\0',};
    int client_socket = 0;
    struct sockaddr_in mobius_addr;
    printf("1\n");
    if(flag_isSend){
        client_socket = socket(PF_INET, SOCK_STREAM, 0);
        memset(&mobius_addr, 0, sizeof(mobius_addr));
        mobius_addr.sin_family = AF_INET;
        mobius_addr.sin_port = htons(MOBIUS_PORT);
        mobius_addr.sin_addr.s_addr = inet_addr(MOBIUS_IP);
        // connect
        printf("2\n");
        ret = connect(client_socket, (struct sockaddr*)&mobius_addr, sizeof(mobius_addr));
        printf("2-1\n");
        if(ret < 0){
            printf("connect error\n");
        }
        // msg formatting
        printf("3\n");
        sprintf(url, "%s%s", SERVER_URL, srcfile);
        sprintf(msg,                 \
                "{\n"                  \
                "\"ctname\" : \"odroid\",\n"  \
                "\"con\" : \"%s\"\n"           \
                "}", url);
        printf("%s\n", msg);

        // send message
        printf("4\n");
        ret = send(client_socket, msg, 256, 0);
        usleep(500);
        if(ret < 0){
            printf("send error\n");
        }
    }
    printf("5\n");
    close(client_socket);
}
