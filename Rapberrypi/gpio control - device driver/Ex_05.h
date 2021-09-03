#ifndef __SIMPLE__
#define __SIMPLE__

typedef struct _LedCtl{
    int pin;
    char act[5];
}LedCtl;

#define MY_IOC_MAGIC 'c'

#define MY_IOC_SET      _IOW(MY_IOC_MAGIC, 0, int)
#define MY_IOC_ON       _IOW(MY_IOC_MAGIC, 1, LedCtl)
#define MY_IOC_OFF      _IOW(MY_IOC_MAGIC, 2, LedCtl)
#define MY_IOC_CLEAR    _IO(MY_IOC_MAGIC, 3)

#define MY_IOC_MAXNR 4

#endif

