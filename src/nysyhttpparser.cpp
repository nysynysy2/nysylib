#include "nysyhttpparser.h"
#include <sstream>
namespace nysy {
	HttpParser& HttpParser::parse(std::string const& data)& {
		std::stringstream ss{ data };
		std::string buffer{};
		std::getline(ss, buffer);
		if (buffer.ends_with('\r'))buffer.erase(buffer.size() - 1);

		std::smatch res{};

		m_is_validated = std::regex_match(buffer, res, start_line_regex);
		if (!m_is_validated)return *this;

		m_method = res[1];
		m_path = res[2];
		m_http_ver = res[3];

		m_is_validated = false;
		while (std::getline(ss, buffer)) {
			if (buffer.ends_with('\r'))buffer.erase(buffer.size() - 1);
			if (buffer.empty())break;
			if (std::regex_match(buffer, res, header_line_regex)) {
				std::string header{ res[1] };
				std::string value{ res[2] };
				m_headers[header] = value;
				header_processing(header, value);
				m_is_validated = true;
			}
		}
		return *this;
	}
	HttpParser&& HttpParser::parse(std::string const& data)&& {
		this->parse(data);
		return std::move(*this);
	}


	std::optional<std::string> HttpParser::get_method()&& {
		if (m_is_validated)return std::move(m_method);
		else return std::nullopt;
	}
	std::optional<std::string> HttpParser::get_method()const& {
		if (m_is_validated)return m_method;
		else return std::nullopt;
	}

	std::optional<std::string> HttpParser::get_path()&& {
		if (m_is_validated)return std::move(m_path);
		else return std::nullopt;
	}
	std::optional<std::string> HttpParser::get_path()const& {
		if (m_is_validated)return m_path;
		else return std::nullopt;
	}

	std::optional<std::string> HttpParser::get_http_ver()&& {
		if (m_is_validated)return std::move(m_http_ver);
		else return std::nullopt;
	}
	std::optional<std::string> HttpParser::get_http_ver()const& {
		if (m_is_validated)return m_http_ver;
		else return std::nullopt;
	}

	std::optional<std::vector<std::string>> HttpParser::get_content_types()&& {
		if (m_is_validated)return std::move(m_accept_content_types);
		else return std::nullopt;
	}
	std::optional<std::vector<std::string>> HttpParser::get_content_types()const& {
		if (m_is_validated)return m_accept_content_types;
		else return std::nullopt;
	}

	std::optional<std::string> HttpParser::get_header_value(const std::string& header)&& {
		try {
			return std::move(m_headers.at(header));
		}
		catch (const std::out_of_range&) {
			return std::nullopt;
		}
	}
	std::optional<std::string> HttpParser::get_header_value(const std::string& header)const& {
		try {
			return m_headers.at(header);
		}
		catch (const std::out_of_range&) {
			return std::nullopt;
		}
	}

	std::optional<std::unordered_map<std::string, std::string>> HttpParser::get_headers()&& {
		if (m_is_validated)return std::move(m_headers);
		else return std::nullopt;
	}
	std::optional<std::unordered_map<std::string, std::string>> HttpParser::get_headers()const& {
		if (m_is_validated)return m_headers;
		else return std::nullopt;
	}


	void HttpParser::header_processing(const std::string& header, const std::string& value) {

		if (header == "Accept") {
			std::string content_types{ value }, accept_buffer;
			std::stringstream ct_ss{ content_types };
			while (std::getline(ct_ss, accept_buffer, ',')) {
				m_accept_content_types.push_back(accept_buffer);
			}
		}

	}
}