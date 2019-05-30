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
#include <boost/asio/signal_set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <mutex>
#include <deque>
#include "elliptic/curve.hpp"
#include "elliptic/point.hpp"
#include "elliptic/utility.hpp"
#include <sstream>

#include "config.hpp"
int debug = 0;
using boost::asio::ip::tcp;
using boost::multiprecision::int1024_t;
using namespace ec;

boost::mutex global_lock;

bool one_run_flag = false;
bool slow_flag = false;
int delay = 2;

class workerThread
{
public:
    static void run(boost::shared_ptr<boost::asio::io_service> io_service)
    {
        {
			global_lock.lock();
			std::cout << "[" << boost::this_thread::get_id() << "]: Thread starts" << std::endl;
			global_lock.unlock();
		}

        io_service->run();

        {
			global_lock.lock();
			std::cout << "[" << boost::this_thread::get_id() << "] Thread ends" << std::endl;
			global_lock.unlock();
		}

    }
};




//--------------------------------SESSION----------------------------------
class Session : public boost::enable_shared_from_this<Session>{

public:
	Session(boost::asio::io_service& io_service):m_socket(io_service), m_io_service(io_service){
		std::cout<<"[" << boost::this_thread::get_id() << "]: ###Constructor: Session###"<<std::endl;
	}

	~Session(){
        std::cout<<"[" << boost::this_thread::get_id() << "]: ###Destructor: Session###"<<std::endl<<std::endl;
    }

	typedef boost::shared_ptr<Session> sess_ptr;
	//make pointer
	static sess_ptr create(boost::asio::io_service& io_service){
		return sess_ptr(new Session(io_service));
	}
	//make socket
	tcp::socket& get_socket(){
		return m_socket;
	}
	//###START ACTION OF SESSION
	void start(){
        std::cout<<"[" << boost::this_thread::get_id() << "]: Connection from: "<<m_socket.remote_endpoint()<<std::endl;
		data_prepare();
		do_read();
	}

	void do_read(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: do_read"<<std::endl;
		if (slow_flag == true) boost::this_thread::sleep( boost::posix_time::seconds(delay));

		boost::asio::async_read(m_socket,
								boost::asio::buffer(msg_in, msg_in.size()),
								boost::bind(&Session::handle_read, shared_from_this(), _1));

		/*
		boost::asio::async_read_until(m_socket, m_input_buffer, delimiter,
										boost::bind(&Session::handle_read, shared_from_this(), _1));
		*/
	}

	void do_write(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: do_write"<<std::endl;
		if (slow_flag == true) boost::this_thread::sleep( boost::posix_time::seconds(delay));

        std::cout<<"[" << boost::this_thread::get_id() << "]:";
		std::cout<<"	Sending :"<<msg_out_que.front().data()<<std::endl;
		boost::asio::async_write(m_socket,
							boost::asio::buffer(msg_out_que.front(), msg_out_que.front().size()),
							boost::bind(&Session::handle_write, shared_from_this(), _1));


	}


private:
	tcp::socket m_socket;
	boost::asio::io_service& m_io_service;
	//control flags
	bool data_prepared = 0;
	bool exchange_done = 0;
    int1024_t private_key_server{0}; //private value for A

	//// DATA TO SEND
	std::deque<std::array<char, MAX_SEGMENT_SIZE>> msg_out_que;;
	std::array<char, MAX_SEGMENT_SIZE> msg_in;
    Point<int1024_t> public_key_server;
    Point<int1024_t> private_key_final;
    std::string last_msg_in;


	void data_prepare(){
		// TO DO BETTTERRRRRRR, CREATE MESSAGE QUE
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: data_prepare"<<std::endl;
		data_prepared = 1;

        //temp for conversion of multi precision
        std::stringstream temp_multiprecision;

        //Creation of string containing generating_point
        temp_multiprecision << generating_point;
		std::string s_m_P = temp_multiprecision.str();

        //generation of server public key
        do {
            private_key_server = randomBigInt(160);
        } while (private_key_server > group_size);

        public_key_server = generating_point * private_key_server; //g^a  key to send from A
        temp_multiprecision.str("");
        temp_multiprecision << public_key_server;

        std::string s_m_Q = temp_multiprecision.str();

        //control messages
		std::string s_ACK = "start transmission";
		std::string s_end = "end transmission";
		//COMPRESS INTO ONE FUNCTION

        add_msg(s_ACK);
		add_msg(s_m_P);
		add_msg(s_m_Q);
		add_msg(s_end);

		data_prepared = 1;
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


	void handle_read(const boost::system::error_code& err){
		if(!err){
			if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle read"<<std::endl;
            std::cout<<"[" << boost::this_thread::get_id() << "]:";
			std::cout<<"	Received string:"<<msg_in.data()<<std::endl;
			std::string check = msg_in.data();
			check = check.substr(0,3);
				//usefull for checks
			//if (debug == 1) std::cout<<"	sub string: "<<check<<std::endl;

			if( check == "ACK"){
				do_write();
			}
			else if(check == "FIN"){

				//KEY CREATION
				std::string s_public_key_client = last_msg_in;
				if (debug == 1) std::cout<<"public from client: "<<s_public_key_client<<std::endl;

				std::string X = s_public_key_client.substr(1, s_public_key_client.find(",")-1);
				std::string Y = s_public_key_client.substr(s_public_key_client.find(",")+1, s_public_key_client.find(")"));
				Y.pop_back();
				if (debug == 1) std::cout<< "X: "<< X<<std::endl<< " Y: " <<Y<<std::endl;
				Point<int1024_t> public_key_client{int1024_t{X}, int1024_t{Y}, curve_ptr};
                if (debug == 1) std::cout<<"public key client: "<<public_key_client<<std::endl;
                //final private key
				private_key_final = public_key_client*private_key_server;
				std::cout<<"FINAL KEY: "<<private_key_final<<std::endl;

				std::string s_FIN_ACK = "FIN_ACK";
				add_msg(s_FIN_ACK);
				exchange_done = 1;
				do_write();
			}else{
                last_msg_in = msg_in.data();
                do_read();
			}
		}else{
            std::cout<<"[" << boost::this_thread::get_id() << "]:";
			std::cout<<"Error: "<<err.message()<<std::endl;
			//do_write();
		}
	}


	void handle_write(const boost::system::error_code& err){
		if(exchange_done == 1){
                std::cout<<"[" << boost::this_thread::get_id() << "]:";
				std::cout<< " EXCHANGE FINISHED SUCESSFULLY !!!!!!!!!!!!!!!!!!"<<std::endl;
				handle_close();
		}else{
			if(!err){
				if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle write"<<std::endl; //to rm
				msg_out_que.pop_front();
				if(!msg_out_que.empty()){
                    std::cout<<"[" << boost::this_thread::get_id() << "]:";
					std::cout<<"	Sending :"<<msg_out_que.front().data()<<std::endl;
					boost::asio::async_write(m_socket,
										boost::asio::buffer(msg_out_que.front(), msg_out_que.front().size()),
										boost::bind(&Session::handle_write, shared_from_this(), _1));
				}else{
                    std::cout<<"[" << boost::this_thread::get_id() << "]:";
					std::cout<<"Ended exchange"<<std::endl;
					do_read();
				}
			}else{
                std::cout<<"[" << boost::this_thread::get_id() << "]:";
				std::cout<<"Error: "<<err.message()<<std::endl;
				//do_write();
			}
		}
	}

	void handle_close(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: handle _close"<<std::endl;
		m_socket.close();
		if (one_run_flag == 1) m_io_service.stop(); // if you wish to stop thread after finnishing exchange
	}
};

//------------------SERVER-------------------------------------------------
class Server
{
public:
	Server(boost::shared_ptr<boost::asio::io_service> io_service)
		:m_acceptor(*io_service, tcp::endpoint(tcp::v4(), SERVER_PORT)),
		 m_socket(*io_service), m_io_service(io_service){
		 try
		 {
			 std::cout<<"[" << boost::this_thread::get_id() << "]: ###Constructor: Server###"<<std::endl;
             std::cout<<"[" << boost::this_thread::get_id() << "]: ###Server running on adress"<<" on port: "<<SERVER_PORT<<" ###"<<std::endl;
             start_Accept();
		 }
		 catch (std::exception& err)
		 {
            std::cout<<"[" << boost::this_thread::get_id() << "]:";
			std::cerr <<"exception caught: "<<err.what()  << std::endl;
		 }
	 }

	 ~Server(){
         std::cout<<"[" << boost::this_thread::get_id() << "]: ###Destructor: Server###"<<std::endl;
     }



private:
	tcp::acceptor m_acceptor;
	tcp::socket m_socket;
    boost::shared_ptr<boost::asio::io_service> m_io_service;


	void start_Accept(){
		if (debug == 1) std::cout<<"[" << boost::this_thread::get_id() << "]: start accept"<<std::endl;

		//get pointer to new session
		Session::sess_ptr connection = Session::create(m_acceptor.get_io_service());
		//use acceptor to accept

		m_acceptor.async_accept(connection->get_socket(),
					boost::bind(&Server::handle_Accept, this, connection,
					boost::asio::placeholders::error));
	}

	void handle_Accept(Session::sess_ptr connection, const boost::system::error_code &err){
		if(!err){
            connection ->start();
		} else {
            std::cout<<"[" << boost::this_thread::get_id() << "]:";
			std::cerr<<"Exception: "<< err.message() <<std::endl;
		}
		start_Accept();
	}


};


int main(int argc, char* argv[])
{
    if (argc > 1){
        debug = std::stoi(argv[1]);
        std::cout<<"Debug: "<<debug<<std::endl;
        if (argc > 2){
            slow_flag = true;
            delay = std::stoi(argv[2]);
            std::cout<<"Delay: "<<delay<<std::endl;
        }
    }
	//Initialization of io_service, and other members for asynchronous work
	boost::shared_ptr<boost::asio::io_service> io_service(new boost::asio::io_service);
	boost::shared_ptr< boost::asio::io_service::work > work(new boost::asio::io_service::work( *io_service ));
	boost::shared_ptr< boost::asio::io_service::strand > strand(new boost::asio::io_service::strand( *io_service ));

	//tcp::endpoint endpoint(tcp::v4(), 8001);
    //Server init
	Server new_server(io_service);

	boost::thread_group workers;
    boost::asio::signal_set signals( *io_service, SIGINT, SIGTERM );

    //sig handling lambda
    signals.async_wait(
        [&io_service]( const boost::system::error_code& error, int signal_number){
            std::cout << "[" << boost::this_thread::get_id()  << "]: Got signal " << signal_number << " stopping io_service." << std::endl;
            io_service->stop();
    });

    //worker creation and io_service assign
    workers.create_thread(boost::bind(&workerThread::run, io_service));

    std::string command;
    int done = 0;
    std::cout << "[" << boost::this_thread::get_id()  << "]: ### Enter the word \"exit\" to exit ###"<<std::endl;
    while (!done && std::cin >> command) {
        if (command == "exit") {
            done = 1;
            io_service->stop();
        }
    }

	workers.join_all();



	return 0;
}
