/*
 Copyright (c) 2014, Pure Engineering LLC
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <limits.h>
#include "ros/ros.h"
#include "ros/time.h"
#include "std_msgs/String.h"
#include "std_msgs/MultiArrayLayout.h"
#include "std_msgs/MultiArrayDimension.h"
#include "std_msgs/Int32MultiArray.h"
#include "sensor_msgs/Image.h"
#include "SPI.h"

#include <sstream>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s) {
	perror(s);
	abort();
}

#define VOSPI_FRAME_SIZE (164)
uint8_t lepton_frame_packet[VOSPI_FRAME_SIZE];
static unsigned int lepton_image[80][80] = { 0 };
ros::Time capture_time;



void resync() {
	// This function tries to resynchronize the video stream

	// Close SPI Port
	SpiClosePort(0);

	// Sleep for 0.75 second to trigger resynch.
	printf("Synchronizing video stream...\n");
	usleep(750000);

	// Open Port again
	SpiOpenPort(0);

	return;

}

void reboot() {
	// This functions reboots lepton camera

	// Close SPI Port
	SpiClosePort(0);

	// Unplug power
	printf("Unplug power for 40 ms\n");
	system("gpio -g write 97 0");
	usleep(40000);
	system("gpio -g write 97 1");

	// Open SPI Port again
	SpiOpenPort(0);

	return;

}

int transfer(int fd, int current_frame) {
	int ret;
	int i;
	int packet_number;
	uint8_t tx[VOSPI_FRAME_SIZE] = { 0, };


	// Take capture time of image corresponding to first package
	if(current_frame==0){
		 capture_time=ros::Time::now();
	}

	struct spi_ioc_transfer tr;

	tr.tx_buf = (unsigned long) tx;
	tr.rx_buf = (unsigned long) lepton_frame_packet;
	tr.len = VOSPI_FRAME_SIZE;
	tr.delay_usecs = spi_delay;
	tr.speed_hz = spi_speed;
	tr.bits_per_word = spi_bitsPerWord;

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	if (ret < 1)
		pabort("can't send spi message");

	if (((lepton_frame_packet[0] & 0xf) != 0x0f)) {
		packet_number = lepton_frame_packet[1];

		//printf("%d", packet_number);

		if (packet_number == current_frame) {
			for (i = 0; i < 80; i++) {
				lepton_image[packet_number][i] = (lepton_frame_packet[2 * i + 4]
						<< 8 | lepton_frame_packet[2 * i + 5]);
			}
		} else {
			packet_number = -1;
		}
	}
	return packet_number;
}

sensor_msgs::Image normalize(sensor_msgs::Image I,
		unsigned int Data_raw[80][80]) {

	int i;
	int j;
	unsigned int maxval = 0;
	unsigned int minval = UINT_MAX;

	// Calculating extrema
	for (i = 0; i < 60; i++) {
		for (j = 0; j < 80; j++) {
			if (Data_raw[i][j] > maxval) {
				maxval = Data_raw[i][j];
			}
			if (Data_raw[i][j] < minval) {
				minval = Data_raw[i][j];
			}
		}
	}
	float diff = maxval - minval;
	if (diff == 0.0) {
		diff = 1.0;
	}
	float scale = 255 / diff;

	// File array with normalized pixel points
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 60; j++) {
			I.data[j * 80 + i] = (Data_raw[j][i] - minval) * scale;
		}
	}

	return I;
}

sensor_msgs::Image agc(sensor_msgs::Image I,
	unsigned int Data_raw[80][80]) {
	int i;
	int j;
	// File array with normal
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 60; j++) {
			I.data[j * 80 + i] = Data_raw[j][i];
		}
	}

	return I;
}


sensor_msgs::Image calib(sensor_msgs::Image I,
		unsigned int Data_raw[80][80], int threshold) {

	int i;
	int j;
	unsigned int maxval = 0;
	unsigned int minval = UINT_MAX;

	// Calculating extrema
	for (i = 0; i < 60; i++) {
		for (j = 0; j < 80; j++) {
			if (Data_raw[i][j] > maxval) {
				maxval = Data_raw[i][j];
			}
			if (Data_raw[i][j] < minval) {
				minval = Data_raw[i][j];
			}
		}
	}
	float diff = maxval - minval;
	if (diff == 0.0) {
		diff = 1.0;
	}
	float scale = 255 / diff;

	// File array with thresholded pixel points
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 60; j++) {
			I.data[j * 80 + i] = (Data_raw[j][i] - minval) * scale;
			if (I.data[j * 80 + i]<threshold){
				I.data[j * 80 + i]=0;
			}
			else{
				I.data[j * 80 + i]=255;
			}

		}
	}

	return I;
}
sensor_msgs::Image inversion(sensor_msgs::Image I,
		unsigned int Data_raw[80][80]) {

	int i;
	int j;
	unsigned int maxval = 0;
	unsigned int minval = UINT_MAX;

	// Calculating extrema
	for (i = 0; i < 60; i++) {
		for (j = 0; j < 80; j++) {
			if (Data_raw[i][j] > maxval) {
				maxval = Data_raw[i][j];
			}
			if (Data_raw[i][j] < minval) {
				minval = Data_raw[i][j];
			}
		}
	}
	float diff = maxval - minval;
	if (diff == 0.0) {
		diff = 1.0;
	}
	float scale = 255 / diff;

	// File array with normalized inverted pixel points
	for (i = 0; i < 80; i++) {
		for (j = 0; j < 60; j++) {
			I.data[j * 80 + i] = 255 - (Data_raw[j][i] - minval) * scale;
		}
	}

	return I;
}

int AskFormat() {
	int Videoformat;
	printf("Please enter video format? (Press 0 to list different formats)\n");
	scanf("%d", &Videoformat);
	if (Videoformat == 0) {
		printf("Available formats\n");
		printf("*****************\n");
		printf("1: Linear normalization (1 Pixel=8 Bit)\n");
		printf("2: AGC mode (1 Pixel=8 Bit)\n");
		printf("3: Calibration mode (1 Pixel=8 Bit)\n");
		printf("4: Inversion mode (1 Pixel=8 Bit)\n");
		exit(0);
	}
	return Videoformat;

}

int AskThreshold() {
	int temp;
	printf("Please define threshold value [0,255]\n");
	scanf("%d", &temp);
	return temp;

}





int main(int argc, char *argv[]) {

	// Ask user which mode video format shall be
	int format = AskFormat();

	// Optionaly ask user which threshold should be used
	int c_threshold;
	if (format == 3) {
		c_threshold = AskThreshold();
	}

	int i, j;
	int i_sync = 0;
	ros::init(argc, argv, "arrayPublisher");
	ros::NodeHandle n;
	ros::Publisher pub = n.advertise < std_msgs::Int32MultiArray
			> ("array", 100);
	ros::Publisher data = n.advertise < sensor_msgs::Image > ("IR_data", 100);

	sensor_msgs::Image IR_data;
	IR_data.height = 60;
	IR_data.width = 80;
	IR_data.step = 80;
	IR_data.encoding = "mono8";
	system("gpio -g write 97 1");
	int ret = 0;
	int reset = 0;
	while (ros::ok()) {

		IR_data.data.clear();
		for (j = 0; j < (int) IR_data.height; j++) {
			for (i = 0; i < (int) IR_data.width; i++) {
				IR_data.data.push_back(0);
			}
		}
		//open spi port
		SpiOpenPort(0);

		// Retrieving frame data
		reset = 0;
		for (j = 0; j < 60; j++) {
			j = transfer(spi_cs0_fd, j); // j:=Packet number

			if (j < 0 && j > -2) {
				reset += 1;
				usleep(1000);

			} else if (j < -2) {
				j = -1;
			}

			// Try to resynchronize video stream or reboot camera after 750 false packages
			// First try to resynchronize video stream.
			// After 3 failed attempts. -> Reboot camera
			if (reset == 750) {

				if (i_sync < 3) {
					resync();
					i_sync++;
					reset = 0;
				} else {
					reboot();
					i_sync = 0;
					reset = 0;

				}
			}
		}

		printf("\n");

		// Process raw data
		switch (format) {
		case 1: // linear normalization
			IR_data = normalize(IR_data, lepton_image);
			break;
		case 2: // AGC mode
			IR_data = agc(IR_data, lepton_image);
			break;
		case 3: // calib mode
					IR_data = calib(IR_data, lepton_image,c_threshold);
					break;
		case 4: // inversion mode
						IR_data = inversion(IR_data, lepton_image);
						break;
		default:
			printf("Unknown video data. Cannot process data\n");
			return 0;

		}

		// Publish data over ROS

		IR_data.header.stamp.sec=capture_time.sec;
		IR_data.header.stamp.nsec=capture_time.nsec;
		data.publish(IR_data);
		ROS_INFO("Successfully published IR image!");

		ros::spinOnce();

		SpiClosePort(0);
		//usleep(37038);
	}

	return ret;
}
