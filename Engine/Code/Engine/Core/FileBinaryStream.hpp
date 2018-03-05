#pragma once

#include "Engine/Core/BinaryStream.hpp"
#include <stdio.h>

class FileBinaryStream : public BinaryStream
{
public:
	const char* m_filename;
	FILE* m_file;

public:
	FileBinaryStream();
	~FileBinaryStream();

	bool is_open();
	bool open_for_write(const char* filename);
	bool open_for_read(const char* filename);
	void close();

	virtual u32 write_bytes(void* bytes, size_t count) override;
	virtual u32 read_bytes(void* out_bytes, size_t count) override;
};