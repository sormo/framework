#pragma once
#include <sokol_gfx.h>
#include <vector>
#include <unordered_map>
#include <xxh3.h>
#include <cassert>

class buffer_sg
{
public:

	using range_id = size_t;

	buffer_sg() = default;
	buffer_sg(sg_usage usage, sg_buffer_type type);

	operator bool() const;

	void apply(range_id range_id, sg_bindings& bindings, size_t vertex_bindings_index = 0);

	range_id append(const char* data, size_t size);

	std::vector<range_id> append(const char* data, size_t size, size_t count);

	size_t get_data_size(range_id range_id);

	// provide data pointer for in-place update
	void update_inplace(range_id range_id, char** data_ptr);

	void update(range_id range_id, const char* data);

	void remove(range_id range_id)
	{
		// TODO
	}

	void flush();

private:
	sg_buffer buffer_id = {};
	sg_usage usage = sg_usage::_SG_USAGE_DEFAULT;
	sg_buffer_type type = sg_buffer_type::_SG_BUFFERTYPE_DEFAULT;

	struct buffer_range
	{
		int offset;
		size_t size;
	};
	std::vector<buffer_range> ranges;
	std::unordered_map<XXH64_hash_t, range_id> ranges_cache;

	std::vector<char> buffer_data;

	bool is_appended = false; // TODO can't update buffer while appending to it in the same frame

	struct
	{
		int offset = 0;
		size_t size = 0;

	} update_range;
	bool is_dirty = false;

	int append_buffer_data(const char* data, size_t size);

	std::vector<int> append_buffer_data(const char* data, size_t size, size_t count);

	void create_buffer(const char* data, size_t size);

	void merge_update_range(int offset, size_t size);
};