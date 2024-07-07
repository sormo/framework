#include "buffer_sg.h"

int buffer_sg::append_buffer_data(char* data, size_t size)
{
	size_t required_size = buffer_data.size() + size;
	int offset = (int)buffer_data.size();

	buffer_data.resize(required_size);
	memcpy(&buffer_data[offset], data, size);

	return offset;
}

std::vector<int> buffer_sg::append_buffer_data(char* data, size_t size, size_t count)
{
	int offset = (int)buffer_data.size();

	size_t required_size = buffer_data.size() + size * count;
	buffer_data.resize(required_size);

	std::vector<int> result;

	for (size_t i = 0; i < count; i++)
	{
		memcpy(&buffer_data[offset], data, size);
		result.push_back(offset);

		offset += (int)size;
	}

	return result;
}

void buffer_sg::create_buffer(char* data, size_t size)
{
	if (buffer_id.id != 0)
		sg_destroy_buffer(buffer_id);

	sg_buffer_desc desc{};
	desc.data = { data, size };
	desc.size = size;
	desc.type = type;
	desc.usage = usage;

	buffer_id = sg_make_buffer(desc);
}

void buffer_sg::merge_update_range(int offset, size_t size)
{
	// Calculate the end points of the intervals
	size_t end1 = offset + size;
	size_t end2 = update_range.offset + update_range.size;

	// Find the new offset as the minimum of the current and updated offsets
	int new_offset = std::min(offset, update_range.offset);

	// Find the new size as the difference between the maximum end points and the new offset
	size_t new_size = std::max(end1, end2) - new_offset;

	// Update the update_range with the new merged interval
	update_range.offset = new_offset;
	update_range.size = new_size;
}

buffer_sg::buffer_sg(sg_usage usage, sg_buffer_type type)
	: usage(usage), type(type)
{}

buffer_sg::operator bool() const
{
	return buffer_id.id != 0;
}

void buffer_sg::apply(range_id range_id, sg_bindings& bindings, size_t vertex_bindings_index)
{
	if (type == SG_BUFFERTYPE_INDEXBUFFER)
	{
		bindings.index_buffer = buffer_id;
		bindings.index_buffer_offset = ranges[range_id].offset;
	}
	else
	{
		bindings.vertex_buffers[vertex_bindings_index] = buffer_id;
		bindings.vertex_buffer_offsets[vertex_bindings_index] = ranges[range_id].offset;
	}
}

buffer_sg::range_id buffer_sg::append(char* data, size_t size)
{
	buffer_range range{ -1, size };

	auto data_hash = XXH64(data, size, 0);
	if (ranges_cache.count(data_hash))
	{
		return ranges_cache[data_hash];
	}

	if (usage == SG_USAGE_IMMUTABLE)
	{
		range.offset = append_buffer_data(data, size);

		create_buffer(buffer_data.data(), buffer_data.size());
	}
	else
	{
		bool is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;

		if (is_over_capacity)
		{
			if (buffer_data.capacity() == 0)
			{
				buffer_data.reserve(256);
				is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;
			}

			while (is_over_capacity)
			{
				buffer_data.reserve(buffer_data.capacity() * 2);
				is_over_capacity = buffer_data.capacity() < buffer_data.size() + size;
			}

			range.offset = append_buffer_data(data, size);

			create_buffer(nullptr, buffer_data.capacity());

			sg_append_buffer(buffer_id, { buffer_data.data(), buffer_data.size() });
		}
		else
		{
			range.offset = append_buffer_data(data, size);
			int sg_offset = sg_append_buffer(buffer_id, { data, size });

			assert(sg_offset == range.offset);
		}
	}

	range_id result = ranges.size();
	ranges.push_back(std::move(range));

	is_appended = true;

	ranges_cache[data_hash] = result;

	return result;
}

std::vector<buffer_sg::range_id> buffer_sg::append(char* data, size_t size, size_t count)
{
	std::vector<range_id> result;

	auto append_buffer_data_and_create_ranges = [this](char* data, size_t size, size_t count)
		{
			std::vector<range_id> result;
			for (auto offset : append_buffer_data(data, size, count))
			{
				result.push_back(ranges.size());
				ranges.push_back({ offset, size });
			}
			return result;
		};

	if (usage == SG_USAGE_IMMUTABLE)
	{
		result = append_buffer_data_and_create_ranges(data, size, count);

		create_buffer(buffer_data.data(), buffer_data.size());
	}
	else
	{
		bool is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;

		if (is_over_capacity)
		{
			if (buffer_data.capacity() == 0)
			{
				buffer_data.reserve(256);
				is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;
			}

			while (is_over_capacity)
			{
				buffer_data.reserve(buffer_data.capacity() * 2);
				is_over_capacity = buffer_data.capacity() < buffer_data.size() + size * count;
			}

			result = append_buffer_data_and_create_ranges(data, size, count);

			create_buffer(nullptr, buffer_data.capacity());

			sg_append_buffer(buffer_id, { buffer_data.data(), buffer_data.size() });
		}
		else
		{
			result = append_buffer_data_and_create_ranges(data, size, count);
		}
	}

	is_appended = true;

	return result;
}

size_t buffer_sg::get_data_size(range_id range_id)
{
	return ranges[range_id].size;
}

// provide data pointer for in-place update

void buffer_sg::update_inplace(range_id range_id, char** data_ptr)
{
	assert(usage != SG_USAGE_IMMUTABLE);

	auto [offset, size] = ranges[range_id];

	*data_ptr = &buffer_data[offset];

	is_dirty = true;

	merge_update_range(offset, size);
}

void buffer_sg::update(range_id range_id, char* data)
{
	assert(usage != SG_USAGE_IMMUTABLE);

	auto [offset, size] = ranges[range_id];

	memcpy(&buffer_data[offset], data, size);

	is_dirty = true;

	merge_update_range(offset, size);
}

void buffer_sg::flush()
{
	if (is_dirty && !is_appended)
	{
		sg_update_buffer(buffer_id, { &buffer_data[update_range.offset], update_range.size });
		is_dirty = false;
	}
	is_appended = false;
}
