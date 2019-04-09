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
 * Author: Vincent Kotzsch <vincent.kotzsch@ni.com>
 *         Clemens Felber <clemens.felber@ni.com>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"

#include <string>
#include <iostream>
#include <unistd.h> //usleep()

// NI includes
#include "ns3/ni.h"

using namespace ns3;


void test_interactive_terminal_demo()
{
    std::cout << "\n------ Running Demo and Interactive Test Prgram" << std::endl;
    std::cout << "Info: Sets up Remote Control Engine and Local Connection through this Terminal" << std::endl;
    std::cout << "Typed in message is sent to Remote Control Engine\nTo check for response from Remote Control Engine press Enter while line is empty\nType \"quit\" to quit (or Ctrl+C)" << std::endl;

    // set remote control thread priority lower than parent thread
    const int parentPriority = NiUtils::GetThreadPrioriy();
    const int niRemoteControlPriority = parentPriority -1;

    RemoteControlEngine c;
    c.Initialize("ns3", 100, niRemoteControlPriority);
    LocalCommsInterface lcitest;
    lcitest.Initialize("rc-srv-local");

    

    // string strin interface level test
    
    std::string message;
    std::string response;
    //while (std::cin >> message) {
    std::cout << "$: ";
    while (getline(std::cin,message)) {
        if(message.empty()){
            ;
        } else if (message == "quit") {
          break;
        } else {
            lcitest.SendMessage(message, "ns3");
        }

        response = lcitest.GetMessage(100);
        if(response.empty()) {
            ; //nothing
        } else {
            std::cout << "RSP: " << response << std::endl;
        }

        std::cout << "$: ";
    }

    c.Deinitialize();

}

void test_standalone_rc_engine(){
    // set remote control thread priority lower than parent thread
    const int parentPriority = NiUtils::GetThreadPrioriy();
    const int niRemoteControlPriority = parentPriority -1;

    // adding thread ID of main NS3 thread for possible troubleshooting
    NiUtils::AddThreadInfo(pthread_self(), "NS3 main thread");

    g_RemoteControlEngine.Initialize("ns3", 100, niRemoteControlPriority); //uses global instance of remote control engine

    //Example Parameters to be access (Write access only through on side each)
    int parameter1 = 0; //Write access only in here, not through testman
    std::string parameter2 = ""; //Write access only through testman, only read here


    // Output Info on program
    std::cout << "\n\n-------Remote Control Standalone Mode-------"
    "\nInfo: Remote Control Engine is running and 2 demo parameters are accessed and output to the terminal"
    "\nAvailable parameters:\n"
    "- ParameterInt1 (Integer): new value is generated cyclically and written to the Remote Control Parameter Database\n"
    "Access through testman with command READ:ParameterInt1\n"
    "- ParameterString1 (String): value is updated cyclically from Remote Control Parameter Database\n"
    "Access through testman with command WRITE:ParameterString1:<string>\n"
    "Press Enter to start... (Quit with Strg+C later)";
    std::cin.get();
    std::cin.get();


    std::cout << "\n\n-------Remote Control Standalone Mode Started-------";
    for(;;) {
      
      // set new value for example parameter "ParameterInt1" (can be read through testman)
      parameter1++;
      std::cout << "\n--------\nParameter \"ParameterInt1\": New value set: " << parameter1 << std::endl;
      g_RemoteControlEngine.GetPdb()->setParameterInt1(parameter1); //Write Parameter to Remote Control Parameter Database
      
      // update value from example parameter "ParameterString1" (can be set through testman, WRITE)
      parameter2 = g_RemoteControlEngine.GetPdb()->getParameterString1(); //Read Parameter from Remote Control Parameter Database
      std::cout << "Reading parameter \"ParameterString1\" from Remote Control Database: \"" << parameter2 << "\"" << std::endl;

      usleep(2000000);
    }
    g_RemoteControlEngine.Deinitialize();
}


int
main (int argc, char *argv[])
{
  RemoteControlEngine rce;
  //rce.Initialize("ns3");
  bool quit = false;
  while (!quit) {
      std::cout << "Remote Control Engine\nDemo and Interactive Test Prgram\n" << std::endl;
      std::cout << "Run Which mode?\n1 ... Terminal Loopback Mode\n2 ... Standalone\nQ ... Quit\nSelect choice and press Enter: ";
      std::string choice = "";

      std::cin >> choice;
      if (choice == "Q" || choice == "q"){
          quit = true;
      } else if (choice == "1") {
          test_interactive_terminal_demo();
      } else if (choice == "2") {
          test_standalone_rc_engine();
      } else {
          std::cout << "Unknown option: " << choice << "\nTry again" << std::endl; 
      }
  }



 return 0;

}

