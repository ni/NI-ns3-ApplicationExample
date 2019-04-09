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

#ifndef LOCAL_COMMS_INTERFACE_H
#define LOCAL_COMMS_INTERFACE_H

#include <string>

#include <poll.h>
#include <sys/un.h>
#include <sys/socket.h>

class LocalCommsInterface
{
    //todo c++ify (use std::string consequently)
    //cleanup (comments, fixed length buffers, naming and general review)
    //correct naming syntax "/tmp/xxx_uds", simple string processing, concatenate

private:
    int sock, msgsock, rval;
    struct sockaddr_un server;
    struct sockaddr_un dstaddr;
    char buf[1024];
    struct pollfd poll_list[1];

public:
	void Initialize(std::string name);
	void Close();
	std::string GetMessage(int timeout);
	void SendMessage(std::string message, std::string destination);

};

#endif
