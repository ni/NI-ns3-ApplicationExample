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

#ifndef REMOTE_CONTROL_ENGINE_H
#define REMOTE_CONTROL_ENGINE_H

#include "ni-local-comms-interface.h"
#include "ni-parameter-data-base.h"
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

class RemoteControlEngine
{
private:
    pthread_t t;
    std::string instance_name;
    int timeout;
    bool stop;
    LocalCommsInterface lci;
    ParameterDataBase pdb;
    int remoteControlInterfaceThreadPriority;

    void *RemoteControlInterfaceThread(void);
    static void *ThreadHelper(void *context);

public:
    RemoteControlEngine();
    void Initialize(std::string name, int tmout, int remoteControlThreadPriority);
    void Deinitialize();

    ParameterDataBase *GetPdb();

};


//global variable for class RemoteControlEngine
extern RemoteControlEngine g_RemoteControlEngine;
    

#endif
