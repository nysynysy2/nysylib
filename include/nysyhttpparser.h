#include <vector>
#include <unordered_map>
#include <string>
#include <optional>
#include <regex>

namespace nysy {
	class HttpParser {
	public:
		std::regex start_line_regex{ R"---(([A-Z]{3,4}) (.*) HTTP/([0-9][.][0-9]))---" };
		std::regex accept_line_regex{ R"---(Accept: (.*))---" };
		std::regex header_line_regex{ R"---((.*): (.*))---" };

		HttpParser() = default;
		HttpParser& parse(std::string const& data)&;
		HttpParser&& parse(std::string const& data)&&;


		std::optional<std::string> get_method()&&;
		std::optional<std::string> get_method()const&;

		std::optional<std::string> get_path()&&;
		std::optional<std::string> get_path()const&;


		std::optional<std::string> get_http_ver()&&;
		std::optional<std::string> get_http_ver()const&;

		std::optional<std::vector<std::string>> get_content_types()&&;
		std::optional<std::vector<std::string>> get_content_types()const&;

		std::optional<std::string> get_header_value(const std::string& header)&&;
		std::optional<std::string> get_header_value(const std::string& header)const&;
		std::optional<std::unordered_map<std::string, std::string>> get_headers()&&;
		std::optional<std::unordered_map<std::string, std::string>> get_headers()const&;

		inline bool is_validated()const { return m_is_validated; }
	private:
		void header_processing(const std::string& header, const std::string& value);

		bool m_is_validated{ true };
		std::string m_method{}, m_path{}, m_http_ver{};
		std::vector<std::string> m_accept_content_types{};
		std::unordered_map<std::string, std::string> m_headers{};
	};
}