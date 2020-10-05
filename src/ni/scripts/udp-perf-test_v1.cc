#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <cstdint>

#include <boost/date_time/posix_time/posix_time.hpp>  // timestamps

using boost::asio::ip::udp;
namespace po = boost::program_options;

struct pkt_event_t
{
  unsigned id;
  unsigned tsSent;
  unsigned tsRecv;
  unsigned tsDiff;
  unsigned long tsMs;
  unsigned numBytes;
};

class UdpPerfServer
{
public:
  UdpPerfServer(boost::asio::io_service& io_service, unsigned short port, bool verbose, unsigned numPkts, std::string outfile)
: io_service_(io_service),
  socket_(io_service, udp::endpoint(udp::v4(), port)),
  running_(true),
  messages_received_(0u),
  verbose_(verbose),
  numPkts_(numPkts),
  outfile_(outfile)
{   
    boost::thread   t(boost::bind(&UdpPerfServer::MessageWorker, this));
    t_.swap(t);

    socket_.async_receive_from(
    boost::asio::buffer(data_, max_length), client_endpoint_,
    boost::bind(&UdpPerfServer::HandleReceive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    pktEvents_.resize (numPkts);
}

  ~UdpPerfServer()
  {
    running_ = false;
    t_.join();
  }

  void HandleReceive(const boost::system::error_code& error, size_t bytes_recvd)
  {
    if (error) {
    std::cout << "receive error " << error << std::endl;
    return;
    }

    std::string  message(data_, bytes_recvd);

    // start receiving again
    socket_.async_receive_from(
    boost::asio::buffer(data_, max_length), client_endpoint_,
    boost::bind(&UdpPerfServer::HandleReceive, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));

    // store the received message in a vector
    boost::lock_guard<boost::recursive_mutex>    lock(mutex_);
    messages_.push_back(message);
    messages_received_++;
  }

  void MessageWorker () {
    while (running_) {
    // work on the messages received
    boost::lock_guard<boost::recursive_mutex>    lock(mutex_);
    timespec tsRecv;
    clock_gettime(CLOCK_REALTIME, &tsRecv);
    if (messages_.size () > 0) {
        if(verbose_) std::cout << "UdpPerfClientServer: Received " << messages_.size () << " messages at time " << tsRecv.tv_nsec << "ns" << std::endl;
        for (unsigned i = 0; i < messages_.size (); ++i) {
        pkt_event_t pktEvent = {0, 0, 0, 0, 0, 0};
        memcpy(&pktEvent, &messages_[i][2*sizeof(uint32_t)], sizeof(pktEvent));
        pktEvent.tsRecv = tsRecv.tv_nsec;
        if (pktEvent.tsRecv < pktEvent.tsSent) {
            pktEvent.tsDiff = 1000000000 - pktEvent.tsSent + pktEvent.tsRecv;  // wrapped around after 1s
        }
        else {
            pktEvent.tsDiff = pktEvent.tsRecv - pktEvent.tsSent;
        }
        if(verbose_) std::cout << "UdpPerfClientServer: Message " << pktEvent.id << " sent at time " << pktEvent.tsSent << "ns (delay " << pktEvent.tsDiff << "ns)" << std::endl;
        if(pktEvent.id >= pktEvents_.size ()) {
            pktEvents_.resize ((pktEvents_.size () + 1) * 2);
        }
        pktEvents_[pktEvent.id] = pktEvent;
        }
        messages_.clear();
    }
    }
  }

  void WriteFile ()
  {
    std::ofstream ofs;
    try {
    ofs.open(outfile_.c_str());
    for (unsigned i = 0; i < numPkts_; ++i) {
        ofs << pktEvents_[i].id << " " << pktEvents_[i].tsSent << " " << pktEvents_[i].tsRecv << " " << pktEvents_[i].tsDiff << " " << pktEvents_[i].tsMs << " " << pktEvents_[i].numBytes << std::endl;
    }
    ofs.close ();
    }
    catch (std::exception& e) {
    std::cerr << "Exception while writing file: " << e.what() << "\n";
    }
  }


  std::vector<std::string>  messages_;
  unsigned messages_received_;

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint client_endpoint_;
  enum { max_length = 1024 , max_msg_length = 1024};
  char data_[max_length];
  boost::recursive_mutex  mutex_;
  bool running_;
  boost::thread t_;
  std::vector<pkt_event_t> pktEvents_;
  bool verbose_;
  unsigned numPkts_;
  std::string outfile_;
};

class UdpPerfClient
{
public:
  UdpPerfClient(boost::asio::io_service& io_service, unsigned short port, bool verbose)
: io_service_(io_service),
  socket_(io_service, udp::endpoint(udp::v4(), port)),
  messages_sent_(0u),
  verbose_(verbose)
{
}
  UdpPerfClient(boost::asio::io_service& io_service, bool verbose)
  : io_service_(io_service),
    socket_(io_service, udp::endpoint(udp::v4(),0) ),
    messages_sent_(0u),
    verbose_(verbose)
  {
  }

  ~UdpPerfClient()
  {

  }


  void Send (udp::endpoint other_end, const std::string &message) {
    unsigned sent = socket_.send_to(
    boost::asio::buffer(message.c_str(), message.size()),
    other_end);

    if (sent < message.size()) {
    std::cout << "only " << sent << " bytes sent from " << message.size()
                            << std::endl;
    }

    ++messages_sent_;
  }

  unsigned messages_sent_;

private:
  boost::asio::io_service& io_service_;
  udp::socket socket_;
  udp::endpoint client_endpoint_;
  bool verbose_;
};

static boost::asio::io_service io_service;
static bool                    running;

void IoServiceThread(void) {
  while (running) {
      io_service.run();
      io_service.reset();
  }
}


int main(int argc, char* argv[])
{
  try
  {
      unsigned        numPkts     = 1000;    // number of UDP packets to send
      unsigned        numBytes    = 900;    // number of bytes per UDP packet
      unsigned        intervalUs  = 1000;  // inter-packet interval in microseconds
      unsigned        burst       = 1;
      bool            verbose     = false;    // print debugging messages
      bool            enableClient  = false;
      bool            enableServer  = false;
      std::string     outfile       = "boost_udp_perf.tr";
      unsigned        timeoutMs     = 100000;    // timeout in   ms
      std::string     serverIpAddr  = "localhost";
      unsigned short  serverPort    = 0; //6661;
      timespec        ts;

      po::options_description desc("Boost UDP Client/Server Perf Test");

      desc.add_options()
                    ("help", "display help")
                    ("client", "enable UDP client")
                    ("server", "enable UDP server")
                    ("pkts", po::value<unsigned> (), "number of packets for client to send")
                    ("bytes", po::value<unsigned> (), "packet size")
                    ("interval", po::value<unsigned> (), "inter-packet interval (microseconds)")
                    ("burst", po::value<unsigned> (), "number of packets per interval")
                    ("verbose", "enable debug messages")
                    ("addr", po::value<std::string> (), "server IP address")
                    ("port", po::value<unsigned short> (), "server UDP port")
                    ("timeout", po::value<unsigned> (), "time for server to listen for UDP packets (ms)")
                    ("outfile", po::value<std::string> (), "trace file name");

      po::variables_map vm;
      po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
      po::notify(vm);

      if(vm.count("help") || (vm.size() == 0)) {
      std::cout << desc << std::endl;
      std::cout << "Client example:" << std::endl;
      std::cout << "./udp-perf-test_v1 --client --pkts=10000 --bytes=1000 --burst=1 --interval=1000 --addr=7.0.0.2 --port=8990" << std::endl;
      std::cout << "Server example:" << std::endl;
      std::cout << "./udp-perf-test_v1 --server --pkts=10000 --timeout=15000 --port=8990 --outfile=\"traffic.log\"" << std::endl;
      }
      if(vm.count("client")) {
      enableClient = true;
      }
      if(vm.count("server")) {
      enableServer = true;
      }
      if(vm.count("pkts")) {
      numPkts = vm["pkts"].as<unsigned> ();
      std::cout << "numPkts: " << numPkts << std::endl;
      }
      if(vm.count("bytes")) {
      numBytes = vm["bytes"].as<unsigned> ();
      }
      if(vm.count("interval")) {
      intervalUs = vm["interval"].as<unsigned> ();
      }
      if(vm.count("burst")) {
      burst = vm["burst"].as<unsigned> ();
      }
      if(vm.count("port")) {
      serverPort = vm["port"].as<unsigned short> ();
      }
      if(vm.count("addr")) {
      serverIpAddr = vm["addr"].as<std::string> ();
      }
      if(vm.count("timeout")) {
      timeoutMs = vm["timeout"].as<unsigned> ();
      }
      if(vm.count("outfile")) {
      outfile = vm["outfile"].as<std::string> ();
      }

      boost::thread       ioServiceThread (IoServiceThread);

      UdpPerfServer* server = 0;
      if (enableServer) {
      running = true;
      server = new UdpPerfServer (io_service, serverPort, verbose, numPkts, outfile);
      }

      if(enableClient) {
      UdpPerfClient client (io_service, verbose);
      //         generate an endpoint for 'a'
      udp::resolver           resolver(io_service);
      udp::resolver::query    query(udp::v4(), serverIpAddr, boost::lexical_cast<std::string>(serverPort)); // std::to_string(serverPort));
      udp::endpoint           recv_endpoint = *resolver.resolve(query);

      char* data  = new char [numBytes];
//      uint32_t syncWord1 = 0xefbeadde;
//      uint32_t syncWord2 = 0x0dd001c0;
      uint32_t syncWord1 = htonl(0xdeadbeef);
      uint32_t syncWord2 = htonl(0xc001d00d);

	// NMi: fill rest of "data" with random numbers.	
	srand(0);
	int i;
	for (i=0; i<numBytes;i++){
		data[i] = rand();
	}

      memcpy(&data[0], &syncWord1, sizeof(syncWord1));
      memcpy(&data[sizeof(syncWord1)], &syncWord2, sizeof(syncWord2));

      //data[numBytes - ] = 0xFF;
      unsigned sleepNow = burst;
      
    boost::posix_time::ptime tStart  = boost::posix_time::microsec_clock::local_time();

    for (unsigned i = 0; i < numPkts; ++i) {
	//while(1) {    // NMi: generate data indefinitely
          clock_gettime(CLOCK_REALTIME, &ts);
          
          boost::posix_time::time_duration dt = boost::posix_time::microsec_clock::local_time() - tStart;
		  unsigned long tsMs = dt.total_microseconds();
		  
          //pkt_event_t pktEvent = {client.messages_sent_, (unsigned)ts.tv_nsec, 1, 1};
          pkt_event_t pktEvent = {client.messages_sent_, (unsigned)ts.tv_nsec, 1, 1, tsMs, numBytes};
          memcpy(&data[2 * sizeof(uint32_t)], &pktEvent, sizeof(pktEvent));
          client.Send(recv_endpoint, std::string(data, numBytes));
          if (intervalUs > 0) {
              if (sleepNow <= 1) {
                  boost::this_thread::sleep(boost::posix_time::microseconds(intervalUs));
                  sleepNow = burst;
              }
              else {
                  sleepNow--;
              }
          }
      }
      std::cout << "sent " << client.messages_sent_ << " messages" << std::endl;

      delete [] data;
      }

      if (enableServer) {
      boost::this_thread::sleep(boost::posix_time::milliseconds(timeoutMs));
      std::cout << "received " << server->messages_received_ << " messages" << std::endl;
      running = false;
      server->WriteFile ();
      }

      // stop the listening thread
      io_service.stop();
      ioServiceThread.join();

      delete server;

  }
  catch (std::exception& e)
  {
      std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
