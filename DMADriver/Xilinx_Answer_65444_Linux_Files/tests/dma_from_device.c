#define _BSD_SOURCE
#define _XOPEN_SOURCE 500
#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static int verbosity = 0;
static int read_back = 0;
static int no_write = 0;
static int allowed_accesses = 1;

static struct option const long_opts[] =
{
  {"device", required_argument, NULL, 'd'},
  {"address", required_argument, NULL, 'a'},
  {"size", required_argument, NULL, 's'},
  {"offset", required_argument, NULL, 'o'},
  {"count", required_argument, NULL, 'c'},
  {"file", required_argument, NULL, 'f'},
  {"verbose", no_argument, NULL, 'v'},
  {"help", no_argument, NULL, 'h'},
  {0, 0, 0, 0}
};

static int test_dma(char *devicename, uint32_t addr, uint32_t size, uint32_t offset, uint32_t count, char *filename);

#define DEVICE_NAME_DEFAULT "/dev/xdma/card0/c2h0"
#define SIZE_DEFAULT (32)
#define COUNT_DEFAULT (1)

static void usage(const char* name)
{
  int i = 0;
  printf("%s\n\n", name);
  printf("usage: %s [OPTIONS]\n\n", name);
  printf("Read using SGDMA, optionally write output to a binary input file\n\n");

  printf("  -%c (--%s) device (defaults to %s)\n", long_opts[i].val, long_opts[i].name, DEVICE_NAME_DEFAULT); i++;
  printf("  -%c (--%s) address of the start address on the AXI bus\n", long_opts[i].val, long_opts[i].name); i++;
  printf("  -%c (--%s) size of a single transfer in bytes, default is %d.\n", long_opts[i].val, long_opts[i].name, SIZE_DEFAULT); i++;
  printf("  -%c (--%s) page offset of transfer\n", long_opts[i].val, long_opts[i].name); i++;
  printf("  -%c (--%s) number of transfers, default is %d.\n", long_opts[i].val, long_opts[i].name, COUNT_DEFAULT); i++;
  printf("  -%c (--%s) filename to read/write the data of the transfers\n", long_opts[i].val, long_opts[i].name); i++;
  printf("  -%c (--%s) be more verbose during test\n", long_opts[i].val, long_opts[i].name); i++;
  printf("  -%c (--%s) print usage help and exit\n", long_opts[i].val, long_opts[i].name); i++;
}

static uint32_t getopt_integer(char *optarg)
{
  int rc;
  uint32_t value;
  rc = sscanf(optarg, "0x%x", &value);
  if (rc <= 0)
    rc = sscanf(optarg, "%ul", &value);
  printf("sscanf() = %d, value = 0x%08x\n", rc, (unsigned int)value);
  return value;
}

int main(int argc, char* argv[])
{
  int cmd_opt;
  char *device = DEVICE_NAME_DEFAULT;
  uint32_t address = 0;
  uint32_t size = SIZE_DEFAULT;
  uint32_t offset = 0;
  uint32_t count = COUNT_DEFAULT;
  char *filename = NULL;

  while ((cmd_opt = getopt_long(argc, argv, "vhxc:f:d:a:s:o:", long_opts, NULL)) != -1)
  {
    switch (cmd_opt)
    {
      case 0:
        /* long option */
        break;
      case 'v':
        verbosity++;
        break;
      /* device node name */
      case 'd':
        device = strdup(optarg);
        break;
      /* RAM address on the AXI bus in bytes */
      case 'a':
        address = getopt_integer(optarg);
        break;
      /* RAM size in bytes */
      case 's':
        size = getopt_integer(optarg);
        break;
      case 'o':
        offset = getopt_integer(optarg) & 4095;
        break;
      /* count */
      case 'c':
        count = getopt_integer(optarg);
        break;
      /* count */
      case 'f':
        filename = strdup(optarg);
        break;
      /* No write to file */
      case 'x':
        no_write++;
        break;
      /* print usage help and exit */
      case 'h':
      default:
        usage(argv[0]);
        exit(0);
        break;
    }
  }
  printf("device = %s, address = 0x%08x, size = 0x%08x, offset = 0x%08x, count = %u\n", device, address, size, offset, count);
  test_dma(device, address, size, offset, count, filename);
}

/* Subtract timespec t2 from t1
 *
 * Both t1 and t2 must already be normalized
 * i.e. 0 <= nsec < 1000000000 */
static void timespec_sub(struct timespec *t1, const struct timespec *t2)
{
  assert(t1->tv_nsec >= 0);
  assert(t1->tv_nsec < 1000000000);
  assert(t2->tv_nsec >= 0);
  assert(t2->tv_nsec < 1000000000);
  t1->tv_sec -= t2->tv_sec;
  t1->tv_nsec -= t2->tv_nsec;
  if (t1->tv_nsec >= 1000000000)
  {
    t1->tv_sec++;
    t1->tv_nsec -= 1000000000;
  }
  else if (t1->tv_nsec < 0)
  {
    t1->tv_sec--;
    t1->tv_nsec += 1000000000;
  }
}

static int test_dma(char *devicename, uint32_t addr, uint32_t size, uint32_t offset, uint32_t count, char *filename)
{
  int rc;
  char *buffer = NULL;
  char *allocated = NULL;
  struct timespec ts_start, ts_end;

  posix_memalign((void **)&allocated, 4096/*alignment*/, size + 4096);
  assert(allocated);
  buffer = allocated + offset;
  printf("host memory buffer = %p\n", buffer);

  int file_fd = -1;
  int fpga_fd = open(devicename, O_RDWR | O_NONBLOCK);
  assert(fpga_fd >= 0);

  /* create file to write data to */
  if (filename) {
    file_fd = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_SYNC, 0666);
    assert(file_fd >= 0);
  }

  while (count--) {
    memset(buffer, 0x00, size);
    /* select AXI MM address */
    off_t off = lseek(fpga_fd, addr, SEEK_SET);
    rc = clock_gettime(CLOCK_MONOTONIC, &ts_start);
    /* read data from AXI MM into buffer using SGDMA */
    rc = read(fpga_fd, buffer, size);
    if ((rc > 0) && (rc < size)) {
      printf("Short read of %d bytes into a %d bytes buffer, could be a packet read?\n", rc, size);
    }
    rc = clock_gettime(CLOCK_MONOTONIC, &ts_end);
    /* file argument given? */
    if ((file_fd >= 0) & (no_write == 0)) {
      /* write buffer to file */
      rc = write(file_fd, buffer, size);
      assert(rc == size);
    }
  }
  /* subtract the start time from the end time */
  timespec_sub(&ts_end, &ts_start);
  /* display passed time, a bit less accurate but side-effects are accounted for */
  printf("CLOCK_MONOTONIC reports %ld.%09ld seconds (total) for last transfer of %d bytes\n", ts_end.tv_sec, ts_end.tv_nsec, size);

  close(fpga_fd);
  if (file_fd >=0) {
    close(file_fd);
  }
  free(allocated);
}

