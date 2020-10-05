# NI Makefile wrapper for waf

CXXOPTIONS := CXXFLAGS='-O3 -g -rdynamic -std=c++0x'

all:	optimized
optimized:	configure compile
debug:	configure_debug compile

# MODIFY HERE --------------------------------------------------------------------------------------
configure:
	$(CXXOPTIONS) ./waf configure -d optimized --enable-examples --enable-sudo
configure_debug:
	$(CXXOPTIONS) ./waf configure -d debug --enable-examples --enable-sudo
configure_tests:
	$(CXXOPTIONS) ./waf configure -d debug --enable-examples --enable-tests --enable-sudo
# MODIFY HERE --------------------------------------------------------------------------------------

# free free to change this part to suit your requirements
#configure:
#	./waf configure --enable-examples --enable-tests

#build:
#	./waf build

install:
	./waf install

clean:
	./waf clean
	rm -fr *.pcap *Stats.txt *.flowmon *.log testpy-output

distclean:
	./waf distclean

compile:
#	sh -c ulimit -s unlimited
	ulimit -s unlimited; $(CXXOPTIONS) ./waf build
	
# install NI relevant exeutables and libraries to /usr/local
# usage: "sudo make install_ni"
install_ni:
	cp build/libns3.26*.so /usr/local/lib
	cp build/src/ni/examples/ns3.26-ni* /usr/local/bin/
	cp build/src/tap-bridge/ns3.26-tap* /usr/local/bin/
	cp build/src/fd-net-device/ns3.26-raw* /usr/local/bin/
	cp build/src/dali/examples/* /usr/local/bin/
	chmod +s /usr/local/bin/ns3.26*
	ldconfig
		
# sync local ns-3 directory to remote machine (e.g. Linux RT controller) 
# useage: "make sync target=DRE-ELBE-C02"
sync:
	rsync -e ssh -avu --exclude=build --exclude=.svn --exclude=.git --exclude=testpy-output --exclude=*.txt --exclude=*.log --exclude=*.pcap --delete $(shell pwd) root@$(target):"~/"
	
prepare_usrp_2974_lvcomms20:
	opkg update; opkg install --force-downgrade packagegroup-core-buildessential 
	
prepare_usrp_2974_lvcomms20_dev:
	opkg update; opkg install --force-downgrade packagegroup-core-buildessential gcc-dev python-pip python-dev boost boost-dev mono-dev sudo rsync tcpdump htop git
	
prepare_usrp_2974_lvcomms21:
	opkg update; opkg install packagegroup-core-buildessential
	
prepare_usrp_2974_lvcomms21_dev:
	opkg update; opkg install packagegroup-core-buildessential gcc-dev python-pip python-dev boost boost-dev mono-dev sudo rsync tcpdump htop git
	
run_tests_ni:
	cd src/ni/test/; ./ni_test_single_instance.sh; ./ni_test.sh; cd -
	