#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "mali-midgard/mali_kbase_hwcnt_reader.h"
#include "perf.h"

//#define LOG printf
#define LOG while (0) printf

#define PRFCNT_SIZE 2952
#define PRFCNT_BUFFERS 32

#define PRFCNT_BUFFER_SIZE ALIGN_POT(PRFCNT_SIZE *PRFCNT_BUFFERS, 4096)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_POT(x, pot_align) (((x) + (pot_align) - 1) & ~((pot_align) - 1))

static int counter_fd = -1;
static uint8_t *counter_area = MAP_FAILED;

static FILE *counter_output;

union kbase_ioctl_kinstr_prfcnt_setup {
    struct {
        uint32_t request_item_count;
        uint32_t request_item_size;
        uint64_t requests_ptr;
    } in;
    struct {
        uint32_t prfcnt_metadata_item_size;
        uint32_t prfcnt_mmap_size_bytes;
    } out;
};

#define KBASE_IOCTL_KINSTR_PRFCNT_SETUP \
    _IOWR(0x80, 57, union kbase_ioctl_kinstr_prfcnt_setup)

void frostwrap_open(int fd)
{
   struct prfcnt_request_item requests[] = {
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_MODE,
         .u.req_mode = {
            .mode = PRFCNT_MODE_MANUAL,
         },
      },
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_ENABLE,
         .u.req_enable = {
            .block_type = PRFCNT_BLOCK_TYPE_FE,
            .set = 0,
            .enable_mask = { ~0ULL, ~0ULL },
         },
      },
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_ENABLE,
         .u.req_enable = {
            .block_type = PRFCNT_BLOCK_TYPE_TILER,
            .set = 0,
            .enable_mask = { ~0ULL, ~0ULL },
         },
      },
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_ENABLE,
         .u.req_enable = {
            .block_type = PRFCNT_BLOCK_TYPE_MEMORY,
            .set = 0,
            .enable_mask = { ~0ULL, ~0ULL },
         },
      },
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_ENABLE,
         .u.req_enable = {
            .block_type = PRFCNT_BLOCK_TYPE_SHADER_CORE,
            .set = 0,
            .enable_mask = { ~0ULL, ~0ULL },
         },
      },
      {
         .hdr.item_type = PRFCNT_REQUEST_TYPE_SCOPE,
         .u.req_scope = {
            .scope = PRFCNT_SCOPE_GLOBAL,
         },
      },
      {
         .hdr.item_type = FLEX_LIST_TYPE_NONE,
      },
   };

   union kbase_ioctl_kinstr_prfcnt_setup hwc = {
      .in = {
         .request_item_count = ARRAY_SIZE(requests),
         .request_item_size = sizeof(requests[0]),
         .requests_ptr = (uintptr_t) &requests,
      },
   };

   counter_fd = ioctl(fd, KBASE_IOCTL_KINSTR_PRFCNT_SETUP, &hwc);
   if (counter_fd < 0)
      perror("ioctl(kinstr_prfcnt_setup)");

   counter_area = mmap(NULL, PRFCNT_BUFFER_SIZE, PROT_READ, MAP_SHARED, counter_fd, 0);
   if (counter_area == MAP_FAILED)
      perror("mmap(counter_fd)");

   struct prfcnt_control_cmd enable_perf = {
      .cmd = PRFCNT_CONTROL_CMD_START,
   };

   int ret = ioctl(counter_fd, KBASE_IOCTL_KINSTR_PRFCNT_CMD, &enable_perf);
   if (ret == -1)
      perror("ioctl(perfcnt_cmd(start))");

   counter_output = fopen("/tmp/counters.csv", "w");
   if (!counter_output)
      perror("fopen(/tmp/counters.csv)");
}

static void
counter_dump(uint8_t *base, uint8_t *dump, unsigned size)
{
   uint8_t *end = dump + size;

   uint64_t seq = 0;

   while (dump < end) {
      struct prfcnt_metadata *item = (struct prfcnt_metadata *)dump;
      switch (item->hdr.item_type) {
      case PRFCNT_SAMPLE_META_TYPE_SAMPLE:
         LOG("time %llu-%llu seq %llu\n",
             item->u.sample_md.timestamp_start,
             item->u.sample_md.timestamp_end,
             item->u.sample_md.seq);
         seq = item->u.sample_md.seq;
         break;
      case PRFCNT_SAMPLE_META_TYPE_CLOCK:
         for (unsigned i = 0; i < item->u.clock_md.num_domains; ++i)
            LOG("domain %i cycles: %llu\n", i, item->u.clock_md.cycles[i]);
         break;
      case PRFCNT_SAMPLE_META_TYPE_BLOCK: {
         struct prfcnt_block_metadata *b = &item->u.block_md;
         LOG("type %i index %i set %i state %x values %i\n",
             b->block_type, b->block_idx, b->set,
             b->block_state, b->values_offset);
         uint64_t *data = (uint64_t *)(base + b->values_offset);
         for (unsigned i = 0; i < 64; ++i)
            fprintf(counter_output, "%"PRIu64",%u,%u,%u,%"PRIu64"\n",
                    seq, b->block_type, b->block_idx, i, data[i]);
         break;
      }
      case FLEX_LIST_TYPE_NONE:
          return;

      default:
         printf("Unknown type %i subtype %i\n", item->hdr.item_type >> 12,
                item->hdr.item_type & 0xfff);
         return;
      }

      dump += sizeof(*item);
   }
}

void frostwrap_dump(void)
{
    int ret;

    struct prfcnt_control_cmd sample_perf = {
        .cmd = PRFCNT_CONTROL_CMD_SAMPLE_SYNC,
    };

    ret = ioctl(counter_fd, KBASE_IOCTL_KINSTR_PRFCNT_CMD, &sample_perf);
    if (ret == -1)
        perror("ioctl(perfcnt_cmd(sample))");

    struct prfcnt_sample_access sample = {0};
    ioctl(counter_fd, KBASE_IOCTL_KINSTR_PRFCNT_GET_SAMPLE, &sample);
    if (ret == -1)
        perror("ioctl(perfcnt_cmd(get_sample))");

    LOG("sample at offset %"PRIu64"\n", (uint64_t) sample.sample_offset_bytes);

    counter_dump(counter_area, counter_area + sample.sample_offset_bytes, PRFCNT_SIZE);

    ioctl(counter_fd, KBASE_IOCTL_KINSTR_PRFCNT_PUT_SAMPLE, &sample);
    if (ret == -1)
        perror("ioctl(perfcnt_cmd(pit_sample))");
}
