obj-m+=Ex_05_dev.o  
    
ARCH :=arm  
PWD := $(shell pwd)  
     
all:  
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) ARCH=$(ARCH) modules  

demo:
	gcc -o Ex_05_app Ex_05_app.c
    
clean:  
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) ARCH=$(ARCH) clean
	rm -rf Ex_05_app
