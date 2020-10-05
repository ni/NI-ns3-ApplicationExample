#!/bin/bash

LOCAL_GIT_REPO="/home/root/ns-3.26"
REMOTE_GIT_REPO_EXTERNAL="https://github.com/ni/NI-ns3-ApplicationExample.git"
REMOTE_GIT_REPO=$REMOTE_GIT_REPO_EXTERNAL

# Update list of available packages
opkg update

# make should be available by default, if not we continue by installing it brute force
opkg install make

# git is needed to download the source code
opkg install git
# clone the NI repository for NS-3 code
# Important! user account + password needed
git clone $REMOTE_GIT_REPO $LOCAL_GIT_REPO

# install required packages to build ns-3 on USRP 2974 running NI Linux Real-Time 5.0 / LabVIEW Communcations 2.0
cd $LOCAL_GIT_REPO
make configure
make prepare_usrp_2974_lvcomms20

# build ns-3
make

# install NI relevant exeutables and libraries to /usr/local 
make install_ni
