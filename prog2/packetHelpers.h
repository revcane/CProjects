#ifndef packetHelpers_H
#define packetHelpers_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int decodePacket(unsigned char low, unsigned char high) {
    return (int16_t)((high << 8) | low); 
}

void processPacket(const unsigned char *packet,int outputFd) {
    if (packet[0] != 0x55 || packet[1] != 0x61) {
        perror("bad packet header or flag.\n");
        return;
    }
    int axRaw = decodePacket(packet[2], packet[3]);
    int ayRaw = decodePacket(packet[4], packet[5]);
    int azRaw = decodePacket(packet[6], packet[7]);
    double ax = (axRaw / 32768.0) * 16.0; 
    double ay = (ayRaw / 32768.0) * 16.0; 
    double az = (azRaw / 32768.0) * 16.0; 

    int wxRaw = decodePacket(packet[8], packet[9]);
    int wyRaw = decodePacket(packet[10], packet[11]);
    int wzRaw = decodePacket(packet[12], packet[13]);
    double wx = (wxRaw / 32760.0) * 2000.0;
    double wy = (wyRaw / 32768.0) * 2000.0;
    double wz = (wzRaw / 32768.0) * 2000.0;

    int rollRaw = decodePacket(packet[14], packet[15]);
    int pitchRaw = decodePacket(packet[16], packet[17]);
    int yawRaw = decodePacket(packet[18], packet[19]);
    double roll = (rollRaw / 32768.0) * 180.0;
    double pitch = (pitchRaw / 32768.0) * 180.0;
    double yaw = (yawRaw / 32768.0) * 180.0;

    printf("ax: %.6f ay: %.6f az: %.6f\n", ax, ay, az); //rounding of doom
    printf("wx: %.6f wy: %.6f wz: %.6f\n", wx, wy, wz);
    printf("roll: %.6f pitch: %.6f yaw: %.6f\n", roll, pitch, yaw);
    printf("\n");

    if (outputFd != -1) { //writing to output file
        write(outputFd, &ax, sizeof(double));
        write(outputFd, &ay, sizeof(double));
        write(outputFd, &az, sizeof(double));
        write(outputFd, &wx, sizeof(double));
        write(outputFd, &wy, sizeof(double));
        write(outputFd, &wz, sizeof(double));
        write(outputFd, &roll, sizeof(double));
        write(outputFd, &pitch, sizeof(double));
        write(outputFd, &yaw, sizeof(double));
    }
}

#endif