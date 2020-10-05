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

#include "ni-remote-control-engine.h"
#include "ns3/ni-utils.h"
#include <unistd.h>

//instatntiate global variable
RemoteControlEngine g_RemoteControlEngine;

RemoteControlEngine::RemoteControlEngine() {
        instance_name = "";
        timeout = 1000;
        stop = false;
    }

void RemoteControlEngine::Initialize(std::string name, int tmout, int remoteControlThreadPriority)
{
    instance_name = name;
    timeout = tmout;
    remoteControlInterfaceThreadPriority = remoteControlThreadPriority;
    //Spawn Thread
    pthread_create(&t, NULL, &RemoteControlEngine::ThreadHelper, this);
    //int status = pthread_create(&t, NULL, &RemoteControlEngine::ThreadHelper, this);
    //todo thread error handling
    //std::cout << "thread status: " << status << "\n" << std::endl;
    return;
}

ParameterDataBase *RemoteControlEngine::GetPdb(){
    return &pdb;
}

void RemoteControlEngine::Deinitialize(){
    stop = true;
    pthread_join(t, NULL);
}

void *RemoteControlEngine::RemoteControlInterfaceThread(void)
{
    // set thread priority
    ns3::NiUtils::SetThreadPrioriy(remoteControlInterfaceThreadPriority);
    ns3::NiUtils::AddThreadInfo ("RC Engine Thread");
    //std::cout << "NI.RC: remote-control thread with id:" <<  pthread_self() << " started" << std::endl;

    //todo consider restructuring and cleanup
    //LocalCommsInterface lci;
    //ParameterDataBase pdb;
    lci.Initialize(instance_name);
    std::string temp_buf;
    //todo consider not hardcoding names and further review naming convention as such (could encounter this issue a couple more time)
    std::string destination = "rc-srv-local";

    //todo eval if possibility needed to cancel thread?
    while (stop==false) {
        //todo return value of get message must indicate if valid or not
        //todo or need a "data_avail" method
        temp_buf = lci.GetMessage(timeout);
        
        //here starts command handler!
        if(temp_buf.empty())
        {
            ;  //No message received at all, do nothing
        } else {
            //todo: "handleStringCommand(temp_buf);""

            //First Command Level (Tier 1)

            // todo more hierarchical and structured command processing by tree/level based environment calls
            std::stringstream ss(temp_buf);
            std::string token;
            std::getline(ss,token,':');
            //std::cout << "command (tier 1) token: " << token << std::endl;
            // // Tier 1 if-else ladder
            std::string response = "";
            if (token.empty()) {
                response = "ERR:EMPTY_COMMAND";
            } else if (token == "READ") {
                std::string pname = "";
                std::getline(ss, pname, ':');
                response = pdb.getParameterByName(pname);
            } else if (token == "WRITE") {
                std::string pname = "";
                std::string pvalue = "";
                std::getline(ss, pname, ':');
                std::getline(ss, pvalue, ':');
                response = pdb.setParameterByName(pname, pvalue);
            } else {
                response = "ERR:UNKNOWN_COMMAND";
            }

            //std::cout << response << std::endl;
            lci.SendMessage (response, destination);




        }

        //todo DELET THIS: use timeout in poll() instead or blocking poll() at all
        usleep(1000000);

    }



    return 0;
}

//CAREFUL: ThreadHelper is defined as static  member function in class definition (header), although "static" keyword is left out here
void *RemoteControlEngine::ThreadHelper(void *context)
{
    return ((RemoteControlEngine *)context)->RemoteControlInterfaceThread();
}
