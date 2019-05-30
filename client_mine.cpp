#include <memory>
#include <thread>
#include <array>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <mutex>
#include <deque>
#include <boost/multiprecision/cpp_int.hpp>
#include "elliptic/curve.hpp"
#include "elliptic/point.hpp"
#include "elliptic/utility.hpp"
#include <sstream>

#include "config.hpp"

using boost::asio::ip::tcp;
using boost::multiprecision::int1024_t;
using namespace ec;
int debug = 0;

class Client{
public:
	Client(boost::asio::io_service& io_service, tcp::resolver::iterator endpoint_iterator)
		:m_io_service(io_service), m_socket(io_service){
		std::cout<<"[" << boost::this_thread::get_id() << "]: ###Constructor: Client###"<<std::endl;
		handle_connect();
		boost::asio::async_connect(m_socket,
										endpoint_iterator,
										boost::bind(&Client::do_write, this));

		}
	~Client(){
		std::cout<<"[" << boost::this_thread::get_id() << "]: ###Destructor: Client###"<<std::endl;
		}

		void do_write(){
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: do_write"<<std::endl;
			std::cout<<"[" << boost::this_thread::get_id() << "]: Sending :"<<msg_out_que.front().data()<<std::endl;
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: Size of message being send: "<<msg_out_que.front().size()<<std::endl;
			boost::asio::async_write(m_socket,
									boost::asio::buffer(msg_out_que.front(), msg_out_que.front().size()),
									boost::bind(&Client::handle_write, this));
		}

		void do_read(){
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: do_read"<<std::endl;
			boost::asio::async_read(m_socket,
									boost::asio::buffer(msg_in, msg_in.size()),
									boost::bind(&Client::handle_read, this, _1));
		}

		void do_close(){
			handle_close();
		}
private:
	tcp::socket m_socket;
	boost::asio::io_service& m_io_service;
	//control flags
	bool data_prepared = 0;
	bool exchange_done = 0;
	bool receiving_done = 0;
	int1024_t private_key_client{0};
	Point<int1024_t> public_key_client;
	Point<int1024_t> private_key_final{0};


	//// DATA TO SEND
	std::deque<std::array<char, MAX_SEGMENT_SIZE>> msg_out_que;;
	std::array<char, MAX_SEGMENT_SIZE> msg_in;
	std::string last_msg_in;
	std::string last_msg_check;

	void handle_connect(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle_connect"<<std::endl;
			data_prepare();
			std::cout<<"[" << boost::this_thread::get_id() << "]: Connected"<<std::endl;
	/*
		}else{
			std::cerr << err.message()  << "\n";
			handle_close();
	}*/

	}
	void handle_write(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle write"<<std::endl;
		msg_out_que.pop_front();
		if(!msg_out_que.empty()){

			if(exchange_done == 0){ //exch not done
				if(receiving_done == 0){
					if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: __started receiving__"<<std::endl;
					do_read(); //start receiving
				}else{ //finished receiving
					if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: __ exchange_done = 1___"<<std::endl;
					exchange_done = 1;
					do_write();
				}
			}
			else { //done exch
				do_write();
			}

		}else{ //msg_out_que == empty
			do_read();
		}
	}

	void handle_read(const boost::system::error_code& err){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle_read"<<std::endl;
		if (msg_in.data() == last_msg_check){
			std::cout<<"[" << boost::this_thread::get_id() << "]: ###Please make sure server is online, and please restart client "<<std::endl;
			std::cout<<"[" << boost::this_thread::get_id() << "]: ###Issue: Received same message twice"<<std::endl;
			handle_close();
			exit(1);
		}
		std::cout<<"[" << boost::this_thread::get_id() << "]: Received: "<< msg_in.data()<<std::endl;

		std::string check = msg_in.data();
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: Number of received chars "<< check.length()<<std::endl;
		check = check.substr(0,3);
		last_msg_check = msg_in.data();
		//usefull for checks
		//	std::cout<<"	sub string: "<<check<<std::endl;

		if( check == "sta"){
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: __started exchange__"<<std::endl;
			do_read();
		}else if(check == "end"){
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: __receiving done__"<<std::endl;

				//KEY CREATION
				std::string s_public_key_server = last_msg_in;
				if (debug == 1) std::cout<<"PUBLIC FROM SERVER: "<<s_public_key_server<<std::endl;


				std::string X = s_public_key_server.substr(1, s_public_key_server.find(",")-1);
				std::string Y = s_public_key_server.substr(s_public_key_server.find(",")+1, s_public_key_server.find(")"));
				Y.pop_back();
				if (debug == 1) std::cout<< "X: "<< X<<std::endl<< " Y: " <<Y<<std::endl;
				Point<int1024_t> public_key_server{int1024_t{X}, int1024_t{Y}, curve_ptr};
				if (debug == 1) std::cout<<"Public key server: "<<public_key_server<<std::endl;

				private_key_final = public_key_server*private_key_client;
				std::cout<<"FINAL KEY: "<<private_key_final<<std::endl;


			do_write();
			receiving_done = 1;
		}else if(receiving_done == 0){
			last_msg_in = msg_in.data();
			do_read();
		}else if(exchange_done == 1 && check == "FIN"){
			std::cout<<"[" << boost::this_thread::get_id() << "]: EXCHANGE FINISHED SUCESSFULLY !!!!!!!!!!!!!!!!!!"<<std::endl;
			handle_close();
		}
	}

	void handle_close(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle _close"<<std::endl;
		m_socket.close();
		m_io_service.stop();
		exit(1);
	}

	void data_prepare(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: data_prepare"<<std::endl;
		std::stringstream temp_multiprecision;

		//private and public key generation
		do {
            private_key_client = randomBigInt(160);
        } while (private_key_client > group_size);
		//public part of client key
		public_key_client = generating_point*private_key_client;

		temp_multiprecision <<public_key_client;
		std::string s_m_R = temp_multiprecision.str();

		std::string s_ACK = "ACK";
		std::string s_FIN_ACK = "FIN_ACK";

		//COMPRESS INTO ONE FUNCTION
		add_msg(s_ACK);
		add_msg(s_m_R);
		add_msg(s_FIN_ACK);
		data_prepared = 1;
		last_msg_in = '0';
	}

	void add_msg(std::string msg_to_add){

		std::array<char, MAX_SEGMENT_SIZE> new_msg;
		std::fill(std::begin(new_msg), std::end(new_msg) , 0);

		int i;
    	for (i = 0; i < msg_to_add.length(); i++) {
        	new_msg[i] = msg_to_add[i];
    	}

		msg_out_que.push_back(new_msg);


	}


};



int main(int argc, char* argv[])
{
	try{
        if (argc < 3){
            std::cerr << "Please as argv place <host> <port> <debug>\n";
            return 1;
        }
		if (argc > 3){
			debug = std::stoi(argv[3]);
			std::cout<<"Debug: "<<debug<<std::endl;
		}
		//init
		boost::asio::io_service io_service;
	    tcp::resolver resolver(io_service);
	    tcp::resolver::query query(argv[1], argv[2]);
	    tcp::resolver::iterator iterator = resolver.resolve(query);
		// Client init

		std::cout<<"[" << boost::this_thread::get_id() << "]: Connecting to: "<< argv[1] << " ,on port: "<< argv[2] <<std::endl;

		Client cli(io_service, iterator);

		//std::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

		boost::asio::signal_set signals( io_service, SIGINT, SIGTERM );

	    //sig handling lambda
	    signals.async_wait(
	        [&io_service]( const boost::system::error_code& error, int signal_number){
	            std::cout << "[" << boost::this_thread::get_id()  << "]: Got signal " << signal_number << "; stopping io_service." << std::endl;
	            io_service.stop();
	    });

		//t.join();
		io_service.run();

	} catch (std::exception& e){
    	std::cerr << "Exception: " << e.what() << "\n";
    }

	    return 0;
}
