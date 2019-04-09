/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 National Instruments
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Daniel Swist
 *         Vincent Kotzsch <vincent.kotzsch@ni.com>
 *         Clemens Felber <clemens.felber@ni.com>
 */

#include "ni-local-comms-interface.h"
#include <iostream>

// todo unistd might nt be needed here
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>



void LocalCommsInterface::Initialize(std::string name){

    sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("ERROR: LocalCommsInterface::Initialize, opening datagram socket");
        exit(1);
    }

    server.sun_family = AF_UNIX;
    // Todo Bconsider identification/addressing and naming issues
    std::string filePath = "/tmp/";
    filePath.append(name);
    filePath.append("_uds");
    strcpy(server.sun_path, filePath.c_str());


    remove(server.sun_path);


    if (bind(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_un))) {
        perror("ERROR: LocalCommsInterface::Initialize, binding stream socket");
        exit(1);
    }
    //printf("Socket has name %s\n", server.sun_path);


    poll_list[0].fd = sock;
    poll_list[0].events = POLLIN|POLLPRI;

}

void LocalCommsInterface::Close(){
    close(sock);
    // todo deleting objects whatsoever?
    return;
}


//todo need data avialable method!


std::string LocalCommsInterface::GetMessage(int timeout){
    // todo option to use blocking get message vs timeout poll get message
    // two methods?
    // one method, depending on parameter


    //int retval = 0;
    ssize_t numBytes = 0;
    socklen_t len;

    //todo consider if timeout value (now: 0) should be passed as argument
    //todo eval: is return value of poll really necessary, -1 is error check errno
    //retval = poll(poll_list,(unsigned long)1,0); //0 --> zero timeout
    poll(poll_list,(unsigned long)1, timeout); //timeout >0 --> timeout in ms; timeout<0 --> no timeout
    if ( (poll_list[0].revents&POLLIN) != POLLIN)
    {
        // no data available
        ;
    } else {
    //std::cout << "data available" << std::endl;
    len = sizeof(struct sockaddr_un);
    numBytes = recvfrom(sock, buf, 1024, 0, (struct sockaddr *) &dstaddr, &len);
    if (numBytes < 0) {
        perror("ERROR: LocalCommsInterface::GetMessage, recvfrom() failed");
    }
    buf[numBytes] = '\0';

    //std::cout << "received bytes: " << numBytes << std::endl;
    //std::cout << buf << std::endl;

    }

    //todo eval: avoid char array buffer and string conversion and use c++ magic?
    
    std::string returnString = "";
    if ( numBytes != 0 ) {
    returnString = buf;
    }
    return returnString;
}

void LocalCommsInterface::SendMessage(std::string message, std::string destination){

    /* construct destination address */
    memset(&dstaddr, 0, sizeof(dstaddr));
    dstaddr.sun_family = AF_UNIX;

    std::string filePath = "/tmp/";
    filePath.append(destination);
    filePath.append("_uds");
    strcpy(dstaddr.sun_path, filePath.c_str());


    /* send message */
    if (sendto(sock, message.c_str(), message.length(), 0, (struct sockaddr *) &dstaddr, sizeof(struct sockaddr_un)) < 0) {
        perror("ERROR: LocalCommsInterface::SendMessage, sendto() failed");
    }

    return;
}
