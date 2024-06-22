#include "zip.h"
#include <miniz.h>
#include <miniz_zip.h>
#include <map>
#include <memory>

namespace frame
{
	struct zip_archive_data
	{
		mz_zip_archive archive;
		std::optional<std::vector<std::string>> files;
		std::map<std::string, mz_zip_archive_file_stat> file_stats;
		std::map<std::string, std::vector<char>> data;
	};

	using zip_archive_data_ptr = std::unique_ptr<zip_archive_data>;

	static struct
	{
		std::map<zip_archive, zip_archive_data_ptr> zips;

		zip_archive zip_counter = 1;

	} zip_data;

	std::optional<zip_archive> open_zip(const std::vector<char>& data)
	{
		auto zdata = std::make_unique<zip_archive_data>();

		if (!mz_zip_reader_init_mem(&zdata->archive, data.data(), data.size(), 0))
			return {};

		auto result = zip_data.zip_counter++;

		zip_data.zips[result] = std::move(zdata);

		return result;
	}

	void close_zip(zip_archive zip)
	{
		mz_zip_reader_end(&zip_data.zips[zip]->archive);
		zip_data.zips.erase(zip);
	}

	const std::vector<std::string>& list_zip_files(zip_archive zip)
	{
		zip_archive_data& archive = *zip_data.zips[zip];

		if (archive.files)
			return *archive.files;

		auto file_count = mz_zip_reader_get_num_files(&archive.archive);

		std::vector<std::string> files;
		for (mz_uint i = 0; i < file_count; ++i)
		{
			mz_zip_archive_file_stat file_stat;
			if (!mz_zip_reader_file_stat(&archive.archive, i, &file_stat))
			{
				continue;
			}

			files.push_back(file_stat.m_filename);
			archive.file_stats[file_stat.m_filename] = file_stat;
		}

		archive.files = std::move(files);

		return *archive.files;
	}

	const std::vector<char>& get_zip_file(zip_archive zip, const std::string& file)
	{
		static const std::vector<char> invalid_file;

		zip_archive_data& archive = *zip_data.zips[zip];

		if (archive.data.count(file))
			return archive.data[file];

		if (!archive.files)
			list_zip_files(zip);

		if (!archive.file_stats.count(file))
			return invalid_file;

		const auto& file_stats = archive.file_stats[file];

		std::vector<char> buffer(file_stats.m_uncomp_size, '\0');

		if (!mz_zip_reader_extract_to_mem(&(archive.archive), file_stats.m_file_index, buffer.data(), buffer.size(), 0))
		{
			auto error = mz_zip_get_last_error(&archive.archive);

			return invalid_file;
		}

		archive.data[file] = std::move(buffer);
	
		return archive.data[file];
	}
}
