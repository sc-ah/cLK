#include <stdio.h>
#include <stdlib.h>

#define HDR_NAME "lk_template"
#define XIP_NAME "tinboot"
#define OUT_NAME "lk.nb"
#define SECTOR 0x800
#define TARGET_SIZE (0x40000 / 0x800)

static int block = 0;

void sign(FILE* fout) {
	fwrite(&block, 1, sizeof(block), fout);
	++block;
	fwrite("\xfd\xff\xfb\xff", 1, 4, fout);
}

void concat(FILE *fin, FILE *fout) {
	char buf[SECTOR];
	size_t n_read;
	while ((n_read = fread(buf, 1, SECTOR, fin))) {
		fwrite(buf, 1, n_read, fout);
		if (n_read % 0x200) {
			fseek(fout, 0x200 - (n_read % 0x200), SEEK_CUR);
		}
		sign(fout);
	}
}

int main() {
	int rc = EXIT_SUCCESS;
	FILE *fhdr, *fxip, *fout;
	
	fout = fopen(OUT_NAME, "wb");
	if (!fout) {
		perror("unable to open output file");
		rc = EXIT_FAILURE;
		goto cleanup;
	}
	
	fhdr = fopen(HDR_NAME, "rb");
	if (!fhdr) {
		perror("unable to open template");
		rc = EXIT_FAILURE;
		goto cleanup;
	}

	fxip = fopen(XIP_NAME, "rb");
	if (!fxip) {
		perror("unable to open xip file");
		rc = EXIT_FAILURE;
		goto cleanup;
	}
	
	concat(fhdr, fout);
	concat(fxip, fout);
	while (block < TARGET_SIZE) {
		fseek(fout, 0x800, SEEK_CUR);
		sign(fout);
	}

cleanup:
	if (fout) {
		fflush(fout);
		fclose(fout);
	}
	if (fhdr) {
		fclose(fhdr);
	}
	if (fxip) {
		fclose(fxip);
	}

	return rc;
}
