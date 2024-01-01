#include "nysysocket.h"
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif//_WIN32
#include <format>
#include <string_view>
#include <expected>
namespace nysy {
	template<typename... Args>
	inline std::string get_error_message(Args... args) {
		return std::format("Line: {}, Function : `{}`, Error: {}", args...);
	}
	SocketError::SocketError(std::string error_message,
		SocketErrorType error_type)
		: m_error_message{ std::format("Tcp Socket Error:\n{}", error_message) }
		, m_error_type{ error_type } {}

	TcpSocket::TcpSocket() {

		m_file_description = ::socket(AF_INET, SOCK_STREAM, 0);
		
		if (m_file_description == TCP_ERROR) {
			throw SocketError{ 
				get_error_message(__LINE__, __func__, "Socket Init Error"),
				SocketErrorType::InitError
			};
		}

	}

	void TcpSocket::bind(unsigned short port, std::string ip) {
		int _on{ 1 };
		if (::setsockopt(m_file_description
			, SOL_SOCKET
			, SO_REUSEADDR
#ifdef _WIN32
			, reinterpret_cast<const char*>(&_on)
#else
			, &_on
#endif
			, sizeof(_on))
			== TCP_ERROR) {
			throw SocketError{ 
				get_error_message(__LINE__, __func__, "Set Port Reuse Error"),
				SocketErrorType::SetPortReuseError
			};
		}

		m_server_address.sin_family	= AF_INET;
		m_server_address.sin_port	= ::htons(port);
		if (::inet_pton(AF_INET, ip.c_str()
			, reinterpret_cast<char*>(&m_server_address.sin_addr))
			== TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Invalid IP Adress"),
				SocketErrorType::InvalidIPAddress
			};
		}
		
		if (::bind(m_file_description
			, reinterpret_cast<sockaddr*>(&m_server_address)
			, sizeof(m_server_address)) == TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Bind Error"),
				SocketErrorType::BindError
			};
		}
	}

	void TcpSocket::listen(int back_log) {
		if (::listen(m_file_description, back_log) == TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Listen Error")
				, SocketErrorType::ListenError
			};
		}
	}

	[[nodiscard]] std::expected<TcpSocket, SocketErrorType> TcpSocket::accept() {
		TcpSocket clientSocket{};
		socklen_t clientLen{ static_cast<socklen_t>(sizeof(clientSocket.m_client_address)) };
		clientSocket.m_file_description = ::accept(m_file_description
			, reinterpret_cast<sockaddr*>(&clientSocket.m_client_address)
			, &clientLen);
		if (clientSocket.m_file_description == TCP_ERROR) {
			return std::unexpected{ SocketErrorType::AcceptError };
		}
		return clientSocket;
	}

	void TcpSocket::send(std::string_view data) {
		if (::send(m_file_description,
			data.data()
			, static_cast<int>(data.size()), 0) == TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Send Data Error")
				, SocketErrorType::SendDataError
			};
		}
	}


	[[nodiscard]] std::expected<std::string, SocketErrorType> TcpSocket::receive() {
		std::string buffer{};
		char char_buffer[CHAR_BUFFER_SIZE]{ 0 };
		long int received_len{ ::recv(m_file_description, char_buffer, CHAR_BUFFER_SIZE, 0) };
		if (received_len == 0)return std::unexpected{ SocketErrorType::Disconnected };
		if (received_len < 0) {
#ifdef _WIN32
			auto error_code{ ::WSAGetLastError() };
			if (error_code != WSAECONNRESET) {
#else
			if (errno != ECONNRESET) {
#endif
				return std::unexpected{ SocketErrorType::ReceiveDataError };
			}
			return std::unexpected{ SocketErrorType::Disconnected };
		}

		buffer.append(char_buffer, received_len);

#ifdef _WIN32
		unsigned long non_blocking{ 1 };
		if (::ioctlsocket(m_file_description, FIONBIO, &non_blocking) == TCP_ERROR) {
			return std::unexpected{ SocketErrorType::SetNonBlockingError };
		}

		while (received_len > 0) {
			received_len = ::recv(m_file_description, char_buffer, CHAR_BUFFER_SIZE, 0);
			if (received_len < 0) {
				if (WSAGetLastError() == WSAEWOULDBLOCK)break;
				else return std::unexpected{ SocketErrorType::ReceiveDataError };
			}
			buffer.append(char_buffer, received_len);
		}

		non_blocking = 0;
		if (::ioctlsocket(m_file_description, FIONBIO, &non_blocking) == TCP_ERROR) {
			return std::unexpected{ SocketErrorType::SetBlockingError };
		}

#else
		while (received_len > 0) {
			received_len = ::recv(m_file_description, char_buffer, CHAR_BUFFER_SIZE, MSG_DONTWAIT);
			if (received_len > 0) {
				buffer.append(char_buffer, received_len);
			}
		}
#endif//_WIN32
		return buffer;
	}

	void TcpSocket::connect(unsigned short port, std::string ip) {
		m_client_address.sin_family = AF_INET;
		m_client_address.sin_port = ::htons(port);
		if (::inet_pton(AF_INET, ip.c_str()
			, reinterpret_cast<char*>(&m_client_address.sin_addr)) == TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Invalid IP Adress")
				, SocketErrorType::InvalidIPAddress
			};
		}

		if (::connect(m_file_description
			, reinterpret_cast<sockaddr*>(&m_client_address)
			, sizeof(m_client_address)) == TCP_ERROR) {
			throw SocketError{
				get_error_message(__LINE__, __func__, "Connect Error")
				, SocketErrorType::ConnectError
			};
		}
	}

	void nysy::TcpSocket::close_socket() {
		if (m_file_description != TCP_ERROR) {
#ifdef _WIN32
			::closesocket(m_file_description);
#else
			::close(m_file_description);
#endif
			m_file_description = TCP_ERROR;
		}
	}

}//namespace nysy