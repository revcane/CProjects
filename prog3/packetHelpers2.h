#ifndef packetHelpers_H
#define packetHelpers_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

void processPacket(const unsigned char *packet, int accelFd, int rotaFd, int anglFd) {
    double value = *(double *)packet;
    static int count = 0;  

    switch (count) {
        case 0: // ax
        case 1: // ay
        case 2: // az
            if (accelFd != -1) {
                if (write(accelFd, &value, sizeof(double)) == -1) {
                    perror("error writing to accel file");
                }
            }
            break;
        case 3: //wx
        case 4: // wy
        case 5: // wz
            if (rotaFd != -1) {
                if (write(rotaFd, &value, sizeof(double)) == -1) {
                    perror("error writing to rota file");
                }
            }
            break;
        case 6: // roll
        case 7: // pitch
        case 8: // yaw
            if (anglFd != -1) {
                if (write(anglFd, &value, sizeof(double)) == -1) {
                    perror("error writing to angl file");
                }
            }
            break;
        default:{
            perror("error in processing packet, index not in range");
            break;
        }
    }
    count = (count + 1) % 9;  //increment

}

#endif