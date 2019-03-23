#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "oggvorbis.h"

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Usage: %s [input.ogg] [output.mogg]\n", argv[0]);
		return 0;
	}
	FILE *fi, *fo;
	fopen_s(&fi, argv[1], "rb");
	if (!fi)
	{
		printf("Could not open %s for reading\n", argv[1]);
		return 1;
	}

	fseek(fi, 0, SEEK_SET);
	vorbis_state* vs;
	err e;
	const size_t units = 0x8000;
	size_t next_filepos = units;
	std::vector<std::pair<int, int>> map_entries;
	map_entries.push_back(std::make_pair(0, 0));

	if (e = vorbis_init(fi, &vs), e == OK)
	{
		while (e = vorbis_next(vs), e == OK)
		{
			if (vs->cur_packet_start >= next_filepos)
			{
				map_entries.push_back(std::make_pair((int)vs->cur_packet_start & (~0xFFF), (int)vs->next_sample));
				next_filepos += units;
			}
		}
		printf("Probably ok, last error: %s", str_of_err(e));
	}
	else
	{
		printf("Could not init vorbis (%s)\n", str_of_err(e));
	}

	vorbis_free(vs);

	fopen_s(&fo, argv[2], "wb");
	if (!fo)
	{
		printf("Could not open %s for writing\n", argv[2]);
		return 1;
	}

	int num_entries = map_entries.size();

	int oggVersion = 0xA;
	int fileOffset = 20 + (8 * num_entries);
	int oggMapVersion = 0xF;
	int bufsize = 20000;

	fwrite(&oggVersion, sizeof(int), 1, fo);
	fwrite(&fileOffset, sizeof(int), 1, fo);
	fwrite(&oggMapVersion, sizeof(int), 1, fo);
	fwrite(&bufsize, sizeof(int), 1, fo);
	fwrite(&num_entries, sizeof(int), 1, fo);
	for (int i = 0; i < num_entries; i++)
	{
		fwrite(&map_entries[i].first, sizeof(int), 1, fo);
		fwrite(&map_entries[i].second, sizeof(int), 1, fo);
	}
	// Copy the audio data
	fseek(fi, 0, SEEK_SET);
	size_t read = 0;
	char copyBuf[8192];
	do {
		read = fread_s(copyBuf, 8192, 1, 8192, fi);
		fwrite(copyBuf, 1, read, fo);
	} while (read > 0);

	fclose(fo);
	fclose(fi);

	return 0;
}