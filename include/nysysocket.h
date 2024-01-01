#ifndef _NYSY_TCP_SOCKET_
#define _NYSY_TCP_SOCKET_
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif//_WIN32

#include <string>
#include <string_view>
#include <numeric>
#include <limits>
#include <expected>

namespace nysy {
#ifdef _WIN32
using FileDescription = std::size_t;
constexpr inline std::size_t TCP_ERROR{ static_cast<std::size_t>(-1) }
	, CHAR_BUFFER_SIZE{ 4096 };
#else
using FileDescription = int;
constexpr inline int TCP_ERROR{ -1 };
constexpr inline std::size_t CHAR_BUFFER_SIZE{ 4096 };
#endif
	
	enum class SocketErrorType : unsigned short{
		InitError = 0
		, SetPortReuseError
		, InvalidIPAddress
		, BindError
		, ListenError
		, AcceptError
		, SendDataError
		, ReceiveDataError
		, SetNonBlockingError
		, SetBlockingError
		, ConnectError
		, UnknownError
        , Disconnected
	};

	class SocketError : std::exception{
	public:
		explicit SocketError() = default;
		explicit SocketError(std::string error_message
			, SocketErrorType error_type = SocketErrorType::UnknownError);
		inline virtual const char* what()const noexcept override {
			return m_error_message.c_str();
		}
		inline SocketErrorType get_error_type()const {
			return m_error_type;
		}
	private:
		std::string m_error_message{"Unknown Tcp Socket Error"};
		SocketErrorType m_error_type{ SocketErrorType::UnknownError };
	};

	class TcpSocket {
	public:
		
#ifdef _WIN32
		static inline void wsa_init()noexcept {
			WSADATA data{};
			::WSAStartup(MAKEWORD(2, 2), &data);
		}

		static inline void wsa_cleanup()noexcept {
			::WSACleanup();
		}
#endif

		/*
		*@throws SocketError
		*/
		TcpSocket();
		
		TcpSocket(const TcpSocket&) = default;

		void bind(unsigned short port, std::string ip = "0.0.0.0");
		
		void listen(int back_log);
		
		[[nodiscard]] std::expected<TcpSocket, SocketErrorType> accept();
		
		void send(std::string_view data);
		
		[[nodiscard]] std::expected<std::string, SocketErrorType> receive();

		void connect(unsigned short port, std::string ip);

		void close_socket();
	private:
		sockaddr_in m_server_address{};
		sockaddr_in m_client_address{};
		FileDescription m_file_description{ TCP_ERROR };
	};

}//namespace nysy
#endif