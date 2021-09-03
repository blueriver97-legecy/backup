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

/** Function Prototype */
Mat detectAndDisplay(Mat frame);
void imageSend(char *srcfile);

/** Global variables */
String cascade_name;
CascadeClassifier cascade;
bool flag = false;
int flag_isSend = 1;
Mat src;

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
    char savefile[128]={0,};
    int spCnt = 0;

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
        output  = detectAndDisplay(frame);
        imshow("", output);
        char c = (char)waitKey(10);
        switch(c){ 
            case 27: // esc
                goto exit;
                break;
            case 32: // space
                sprintf(savefile, "img_%d.jpg", spCnt);
                imwrite(savefile, output);
                imageSend(savefile);
                spCnt++;
                break;
        }
    }
exit:
    return 0;
}
 
/** @function detectAndDisplay */
Mat detectAndDisplay(Mat frame)
{
    Point center;
    std::vector<Rect> objects;

    cascade.detectMultiScale(frame, objects, 1.1, 1, 0 | CASCADE_SCALE_IMAGE, Size(60, 120));
        
    if(objects.size()>0){
        src = frame(objects[0]);
        flag = true;
    }
    if(flag==false){
        //imshow("", frame);
        return frame;
    }else{
        //imshow("", src);
        return src;
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
