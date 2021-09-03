#include <stdio.h>    
#include <sys/types.h>    
#include <sys/stat.h>    
#include <sys/ioctl.h>    
#include <fcntl.h>    
#include <unistd.h>    
#include <string.h>
#include "Ex_05.h"

#define DEVICE_FILENAME  "/dev/Ex_05_dev"    
        
int main(int argc, char* argv[])    
{    
    int dev, ret; 
    LedCtl data;

    memset(&data, 0, sizeof(LedCtl));    

    data.pin = atoi(argv[1]);
    strcpy(data.act, (char *)argv[2]);
    printf("%d, %s\n", data.pin, data.act);

    printf( "device file open\n");     
    dev = open( DEVICE_FILENAME, O_RDWR|O_NDELAY );    
        
    if( dev >= 0 )
    {
        printf("IOCTL --> MY_IOC_SET\n");
        ret = ioctl(dev, MY_IOC_SET, &(data.pin));
        printf("MY_IOC_SET = %d\n", ret);

        if(!strcmp(data.act, "ON") || !strcmp(data.act, "on")){
            ret = ioctl(dev, MY_IOC_ON, &data);
            printf("MY_IOC_ON = %d\n", ret);
        }else if(!strcmp(data.act, "OFF") || !strcmp(data.act, "off")){
            ret = ioctl(dev, MY_IOC_OFF, &data);
            printf("MY_IOC_OFF = %d\n", ret);
        }
    }    
        
    return 0;    
}

