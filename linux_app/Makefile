#
TARGET = spider

# add 'export SOCEDS_DEST_ROOT=~/altera/14.0/embedded' in /etc/profile
export SOCEDS_DEST_ROOT=~/work/intelFPGA/16.1/embedded

# 
#CROSS_COMPILE = arm-linux-gnueabihf-
CROSS_COMPILE = /home/habr/work2/R/buildroot-2015.08/output/host/usr/bin/arm-buildroot-linux-gnueabihf-

#CFLAGS = -g -Wall -static -std=gnu++11 -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include -I./bt/inc
CFLAGS = -g -Wall -std=gnu++11 -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include -I./bt/inc \
  -I ${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include/soc_cv_av -Dsoc_cv_av -static-libgcc -static-libstdc++
LDFLAGS =  -g -Wall  -lstdc++ -lrt -pthread #-L./bt/lib -lbluetooth 

#CC = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)g++
#CC = g++
ARCH= arm

all: $(TARGET)

#$(TARGET): Main.o CSpider.o CSpiderLeg.o CMotor.o terasic_os.o mmap.o BtSppCommand.o BtSpp.o  Queue.o QueueCommand.o PIO_LED.o PIO_BUTTON.o
$(TARGET): Main.o CSpider.o CSpiderLeg.o CMotor.o terasic_os.o mmap.o Queue.o QueueCommand.o PIO_LED.o PIO_BUTTON.o
	$(CC) $(LDFLAGS)   $^ -o $@ 

%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
