#pragma once
#include <vector>
#include <string>
#include <optional>

namespace frame
{
	using zip_archive = int32_t;

	std::optional<zip_archive> open_zip(const std::vector<char>& data);
	void close_zip(zip_archive zip);

	const std::vector<std::string>& list_zip_files(zip_archive zip);
	const std::vector<char>& get_zip_file(zip_archive zip, const std::string& file);
}
