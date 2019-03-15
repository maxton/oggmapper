#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
	size_t next_filepos = 0;
	if (e = vorbis_init(fi, &vs), e == OK)
	{
		while (e = vorbis_next(vs), e == OK)
		{
			if (vs->cur_packet_start >= next_filepos)
			{
				printf("pos: %x, granulepos: %llx, sample: %llx\n",
					vs->cur_packet_start,
					vs->cur_page.granule_pos,
					vs->next_sample);
				next_filepos += 0x8000;
			}
		}
		printf("Probably ok, last error: %s", str_of_err(e));
	}
	else
	{
		printf("Could not init vorbis (%s)\n", str_of_err(e));
	}

	fopen_s(&fo, argv[2], "wb");
	if (!fo)
	{
		printf("Could not open %s for writing\n", argv[2]);
		return 1;
	}
	return 0;
}