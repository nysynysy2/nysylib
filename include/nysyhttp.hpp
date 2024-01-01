#include <fstream>
#include <sstream>
#include <string>
#include <optional>

namespace nysy {

	inline std::optional<std::string> read_file(std::string path) {
		if (path.ends_with("/"))path.append("index.html");
		if (path.starts_with("/"))path.erase(0, 1);
		std::ifstream file{ path, std::ios::binary };
		if (!file.is_open())return std::nullopt;
		std::stringstream ss{};
		ss << file.rdbuf();
		return std::move(ss.str());
	}

}