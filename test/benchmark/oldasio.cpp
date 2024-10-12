
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <boost/asio.hpp>


namespace SockSrv
{
    using ConnectCallback = std::function<void(const std::string& ip, unsigned short port)>;
    using DisconnectCallback = std::function<void(const std::string& ip, unsigned short port)>;
    using ReceiveCallback = std::function<void(const std::string&)>;

    class SocketSession
        : public std::enable_shared_from_this<SocketSession>
    {
    public:
        SocketSession(const std::string& host, int port, boost::asio::ip::tcp::socket socket, DisconnectCallback&& disconnect_cb=nullptr, ReceiveCallback&& receive_cb=nullptr)
        : m_host(host)
		, m_port(port)
		, m_socket(std::move(socket))
		, m_receivecb(receive_cb)
		, m_disconnectcb(disconnect_cb)
		{
			do_receive();
		}
		void do_write(const std::string& msg)
		{
			boost::asio::async_write(m_socket, boost::asio::buffer(msg),
				boost::asio::detached);
		}

    private:
        void do_receive()
		{
			boost::asio::async_read(
				m_socket,
				boost::asio::buffer(m_data),
				boost::asio::transfer_at_least(1),
				[this](const boost::system::error_code& ec, std::size_t bytes_transferred)
				{
					if (ec)
					{
						m_msg = "";
						if (m_disconnectcb) m_disconnectcb(m_host, m_port);
					}
					else
					{
						do_write(m_data);
						do_receive();
					}
				});
		}

        boost::asio::ip::tcp::socket m_socket;
        boost::asio::streambuf m_buffer;
        ReceiveCallback m_receivecb;
        DisconnectCallback m_disconnectcb;
        std::string m_msg;
        std::string m_host;
		
		char m_data[1024];
        int m_port;
    };
};

class SocketServer
{
public:
    
    SocketServer(boost::asio::io_context& context, const int& port, 
        SockSrv::ConnectCallback&& connect_cb=nullptr, SockSrv::DisconnectCallback&& disconnect_cb=nullptr, SockSrv::ReceiveCallback&& receive_cb=nullptr)
		: m_acceptor(m_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		, m_socket(m_context)
		, m_connectcb(connect_cb)
		, m_disconnectcb(disconnect_cb)
		, m_receivecb(receive_cb)
		, m_selfcontext(true)
	{
		do_accept();
	}
	void send(const char* msg)
	{
		for (const auto& session : m_sessions)
		{
			session->do_write(msg);
		}
	}
    void start()
	{
		if (m_selfcontext)
		{
			std::vector<std::thread> threads;
			auto count =1;// std::thread::hardware_concurrency() ;
			for(int n = 0; n < count; ++n)
			{
				std::thread th(std::bind(static_cast<std::size_t (boost::asio::io_context::*)()>(&boost::asio::io_context::run), &m_context));
				threads.push_back(std::move(th));
			}
			for(auto& thread : threads)
			{
				if(thread.joinable())
				{
					thread.join();
				}
			}
		}
	}

private:
    void do_accept()
	{
		m_acceptor.async_accept(m_socket,
			[this](std::error_code ec)
			{
				if (!ec)
				{
					auto host = m_socket.remote_endpoint().address().to_string();
					auto port = m_socket.remote_endpoint().port();
					if (m_connectcb) m_connectcb(host, port);
					m_sessions.push_back(std::make_shared<SockSrv::SocketSession>(host, port, std::move(m_socket), std::move(m_disconnectcb), std::move(m_receivecb)));
				}
				do_accept();
			});
	}
    
    boost::asio::io_context m_context;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    SockSrv::ConnectCallback m_connectcb;
    SockSrv::DisconnectCallback m_disconnectcb;
    SockSrv::ReceiveCallback m_receivecb;
    std::vector<std::shared_ptr<SockSrv::SocketSession>> m_sessions;
    bool m_selfcontext;
};

int main()
{
	boost::asio::io_context context;
	SocketServer ss(context, 8888);
	std::thread x(&SocketServer::start, std::ref(ss));
	x.detach();
	while (true) sleep(1);

	return 0;
}
