/*
 * Copyright (C) 2021 Icecream95
 * Copyright (C) 2019 Google LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <fcntl.h>
#include <limits.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "drm-shim/drm_shim.h"
#include "util/u_math.h"

#include "drm-uapi/panfrost_drm.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t s32;
typedef int64_t s64;

#include "mali-midgard/mali-ioctl.h"
#include "mali-midgard/mali-ioctl-dvalin.h"

#include "perf.h"

//#define LOG printf
#define LOG while (0) printf

#define PROP_NAME(n) [MALI_GPUPROP_ ## n] = #n

static const char * const prop_names[] = {
   PROP_NAME(PRODUCT_ID),
   PROP_NAME(VERSION_STATUS),
   PROP_NAME(MINOR_REVISION),
   PROP_NAME(MAJOR_REVISION),
   PROP_NAME(GPU_FREQ_KHZ_MAX),
   PROP_NAME(LOG2_PROGRAM_COUNTER_SIZE),
   PROP_NAME(TEXTURE_FEATURES_0),
   PROP_NAME(TEXTURE_FEATURES_1),
   PROP_NAME(TEXTURE_FEATURES_2),
   PROP_NAME(GPU_AVAILABLE_MEMORY_SIZE),

   PROP_NAME(L2_LOG2_LINE_SIZE),
   PROP_NAME(L2_LOG2_CACHE_SIZE),
   PROP_NAME(L2_NUM_L2_SLICES),

   PROP_NAME(TILER_BIN_SIZE_BYTES),
   PROP_NAME(TILER_MAX_ACTIVE_LEVELS),

   PROP_NAME(MAX_THREADS),
   PROP_NAME(MAX_WORKGROUP_SIZE),
   PROP_NAME(MAX_BARRIER_SIZE),
   PROP_NAME(MAX_REGISTERS),
   PROP_NAME(MAX_TASK_QUEUE),
   PROP_NAME(MAX_THREAD_GROUP_SPLIT),
   PROP_NAME(IMPL_TECH),

   PROP_NAME(RAW_SHADER_PRESENT),
   PROP_NAME(RAW_TILER_PRESENT),
   PROP_NAME(RAW_L2_PRESENT),
   PROP_NAME(RAW_STACK_PRESENT),
   PROP_NAME(RAW_L2_FEATURES),
   PROP_NAME(RAW_CORE_FEATURES),
   PROP_NAME(RAW_MEM_FEATURES),
   PROP_NAME(RAW_MMU_FEATURES),
   PROP_NAME(RAW_AS_PRESENT),
   PROP_NAME(RAW_JS_PRESENT),
   PROP_NAME(RAW_JS_FEATURES_0),
   PROP_NAME(RAW_JS_FEATURES_1),
   PROP_NAME(RAW_JS_FEATURES_2),
   PROP_NAME(RAW_JS_FEATURES_3),
   PROP_NAME(RAW_JS_FEATURES_4),
   PROP_NAME(RAW_JS_FEATURES_5),
   PROP_NAME(RAW_JS_FEATURES_6),
   PROP_NAME(RAW_JS_FEATURES_7),
   PROP_NAME(RAW_JS_FEATURES_8),
   PROP_NAME(RAW_JS_FEATURES_9),
   PROP_NAME(RAW_JS_FEATURES_10),
   PROP_NAME(RAW_JS_FEATURES_11),
   PROP_NAME(RAW_JS_FEATURES_12),
   PROP_NAME(RAW_JS_FEATURES_13),
   PROP_NAME(RAW_JS_FEATURES_14),
   PROP_NAME(RAW_JS_FEATURES_15),
   PROP_NAME(RAW_TILER_FEATURES),
   PROP_NAME(RAW_TEXTURE_FEATURES_0),
   PROP_NAME(RAW_TEXTURE_FEATURES_1),
   PROP_NAME(RAW_TEXTURE_FEATURES_2),
   PROP_NAME(RAW_GPU_ID),
   PROP_NAME(RAW_THREAD_MAX_THREADS),
   PROP_NAME(RAW_THREAD_MAX_WORKGROUP_SIZE),
   PROP_NAME(RAW_THREAD_MAX_BARRIER_SIZE),
   PROP_NAME(RAW_THREAD_FEATURES),
   PROP_NAME(RAW_COHERENCY_MODE),

   PROP_NAME(COHERENCY_NUM_GROUPS),
   PROP_NAME(COHERENCY_NUM_CORE_GROUPS),
   PROP_NAME(COHERENCY_COHERENCY),
   PROP_NAME(COHERENCY_GROUP_0),
   PROP_NAME(COHERENCY_GROUP_1),
   PROP_NAME(COHERENCY_GROUP_2),
   PROP_NAME(COHERENCY_GROUP_3),
   PROP_NAME(COHERENCY_GROUP_4),
   PROP_NAME(COHERENCY_GROUP_5),
   PROP_NAME(COHERENCY_GROUP_6),
   PROP_NAME(COHERENCY_GROUP_7),
   PROP_NAME(COHERENCY_GROUP_8),
   PROP_NAME(COHERENCY_GROUP_9),
   PROP_NAME(COHERENCY_GROUP_10),
   PROP_NAME(COHERENCY_GROUP_11),
   PROP_NAME(COHERENCY_GROUP_12),
   PROP_NAME(COHERENCY_GROUP_13),
   PROP_NAME(COHERENCY_GROUP_14),
   PROP_NAME(COHERENCY_GROUP_15),

   PROP_NAME(TEXTURE_FEATURES_3),
   PROP_NAME(RAW_TEXTURE_FEATURES_3),

   PROP_NAME(NUM_EXEC_ENGINES),

   PROP_NAME(RAW_THREAD_TLS_ALLOC),
   PROP_NAME(TLS_ALLOC),
   PROP_NAME(RAW_GPU_FEATURES),
};

#undef PROP_NAME

bool drm_shim_driver_prefers_first_render_node = true;

static int kbase_fd = -1;
static uint8_t *gpuprops;
static unsigned gpuprops_size;

static void
pan_print_gpuprops(void)
{
   int i = 0;
   uint64_t x = 0;
   while (i < gpuprops_size) {
      //endian assumptions...
      x = 0;
      memcpy(&x, gpuprops + i, 4);
      i += 4;

      int size = 1 << (x & 3);
      int name = x >> 2;

      x = 0;
      memcpy(&x, gpuprops + i, size);
      i += size;

      if (name <= MALI_GPUPROP_RAW_GPU_FEATURES)
         printf("u%i %s (%i) = 0x%"PRIx64"\n", size << 3, prop_names[name], name, x);
      else
         printf("u%i UNK (%i) = 0x%"PRIx64"\n", size << 3, name, x);
   }
}

static uint64_t
pan_get_gpuprop(int name)
{
   int i = 0;
   uint64_t x = 0;
   while (i < gpuprops_size) {
      //endian assumptions...
      x = 0;
      memcpy(&x, gpuprops + i, 4);
      i += 4;

      int size = 1 << (x & 3);
      int this_name = x >> 2;

      x = 0;
      memcpy(&x, gpuprops + i, size);
      i += size;

      if (this_name == name)
         return x;
   }

   fprintf(stderr, "Unknown prop %i\n", name);
   return 0;
}

static void
pan_open_kbase(void)
{
   if (kbase_fd != -1)
      return;

   kbase_fd = open("/dev/mali0", O_RDWR);

   close(shim_device.mem_fd);
   shim_device.mem_fd = kbase_fd;

   struct mali_ioctl_get_version ver = {
      .major = 11,
      .minor = 35,
   };
   int ret = drmIoctl(kbase_fd, MALI_IOCTL_GET_VERSION, &ver);
   if (ret == -1)
      perror("ioctl(get_version)");

   LOG("kernel version: %i %i\n", ver.major, ver.minor);

   struct mali_ioctl_set_flags flags = {
      .create_flags = 0
   };

   ret = drmIoctl(kbase_fd, MALI_IOCTL_SET_FLAGS, &flags);
   if (ret == -1)
      perror("ioctl(set_flags)");

   mmap(NULL, 4096, PROT_NONE, MAP_SHARED, kbase_fd, 0x3000);

   struct mali_ioctl_get_gpuprops props = {
      .buffer = 0,
      .size = 0,
      .flags = 0,
   };
   ret = drmIoctl(kbase_fd, MALI_IOCTL_GET_GPUPROPS, &props);
   if (ret == -1)
      perror("ioctl(get_gpuprops size)");
   else if (!ret)
      fprintf(stderr, "size == 0 for get_gpuprops\n");

   gpuprops_size = ret;
   gpuprops = calloc(gpuprops_size, 1);

   props.size = gpuprops_size;
   props.buffer = (u64)(uintptr_t) gpuprops;

   ret = drmIoctl(kbase_fd, MALI_IOCTL_GET_GPUPROPS, &props);
   if (ret == -1)
      perror("ioctl(get_gpuprops)");

   frostwrap_open(kbase_fd);
}

static int
pan_ioctl_noop(int fd, unsigned long request, void *arg)
{
   return 0;
}

static int
pan_ioctl_get_param(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_get_param *gp = arg;

   pan_open_kbase();

   /* TODO: Get these values from kbase */

   switch (gp->param) {
   case DRM_PANFROST_PARAM_GPU_PROD_ID:
      gp->value = pan_get_gpuprop(MALI_GPUPROP_PRODUCT_ID);
      LOG("product id: %"PRIx64"\n", (uint64_t) gp->value);
      return 0;
   case DRM_PANFROST_PARAM_GPU_REVISION:
      gp->value = 0; // TODO
      return 0;
   case DRM_PANFROST_PARAM_SHADER_PRESENT:
      gp->value = pan_get_gpuprop(MALI_GPUPROP_RAW_SHADER_PRESENT);
      LOG("shader present: %"PRIx64"\n", (uint64_t) gp->value);
      return 0;
   case DRM_PANFROST_PARAM_TEXTURE_FEATURES0:
      gp->value = pan_get_gpuprop(MALI_GPUPROP_TEXTURE_FEATURES_0);
      LOG("texture features0: %"PRIx64"\n", (uint64_t) gp->value);
      return 0;
   case DRM_PANFROST_PARAM_THREAD_TLS_ALLOC:
      gp->value = pan_get_gpuprop(MALI_GPUPROP_TLS_ALLOC);
      LOG("tls alloc: %"PRIx64"\n", (uint64_t) gp->value);
      return 0;
   case DRM_PANFROST_PARAM_TILER_FEATURES:
      gp->value = pan_get_gpuprop(MALI_GPUPROP_RAW_TILER_FEATURES);
      LOG("tiler features: %"PRIx64"\n", (uint64_t) gp->value);
      return 0;
   case DRM_PANFROST_PARAM_AFBC_FEATURES:
      gp->value = 0;
      return 0;
   default:
      fprintf(stderr, "Unknown DRM_IOCTL_PANFROST_GET_PARAM %d\n", gp->param);
      return -1;
   }
}

static int
pan_ioctl_create_bo(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_create_bo *create = arg;

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = calloc(1, sizeof(*bo));
   size_t size = ALIGN(create->size, 4096);

   union mali_ioctl_mem_alloc kcreate = {
      .in = {
         .va_pages = DIV_ROUND_UP(size, 4096),
         .commit_pages = DIV_ROUND_UP(size, 4096),
         .extent = 0,
         .flags = MALI_MEM_PROT_CPU_RD | MALI_MEM_PROT_CPU_WR | MALI_MEM_PROT_GPU_RD | MALI_MEM_PROT_GPU_WR | MALI_MEM_COHERENT_LOCAL | MALI_MEM_CACHED_CPU | MALI_MEM_SAME_VA,
      }
   };

   if (create->flags & PANFROST_BO_HEAP) {
      size_t align_size = 2 * 1024 * 1024 / 4096; /* 2 MB */

      kcreate.in.va_pages = ALIGN_POT(kcreate.in.va_pages, align_size);
      kcreate.in.commit_pages = 0;
      kcreate.in.extent = align_size;
      kcreate.in.flags |= MALI_MEM_GROW_ON_GPF;
   }

   if (!(create->flags & PANFROST_BO_NOEXEC)) {
      kcreate.in.flags |= MALI_MEM_PROT_GPU_EX;
      kcreate.in.flags &= ~MALI_MEM_PROT_GPU_WR;
   }

   LOG("create %x %x %x %x ", (int)kcreate.in.va_pages, (int)kcreate.in.commit_pages, (int)kcreate.in.extent, (int)kcreate.in.flags);

   int ret = drmIoctl(kbase_fd, MALI_IOCTL_MEM_ALLOC, &kcreate);
   LOG("0x%"PRIx64" 0x%"PRIx64"\n", kcreate.out.flags, kcreate.out.gpu_va);

   uint64_t gpu_va = kcreate.out.gpu_va;

   if (kcreate.out.flags & MALI_MEM_SAME_VA) {
      gpu_va = (uintptr_t)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, kbase_fd, gpu_va);
      LOG("va %"PRIx64"\n", gpu_va);
      if (gpu_va == (uintptr_t)MAP_FAILED)
         perror("mmap");
   } else {
      fprintf(stderr, "same_va not set!\n");
   }

   if (ret == -1)
      perror("ioctl(mem_alloc)");

   bo->mem_addr = gpu_va;
   bo->size = size;

   create->handle = drm_shim_bo_get_handle(shim_fd, bo);
   create->offset = gpu_va;

   drm_shim_bo_put(bo);

   return 0;
}

static int
pan_ioctl_mmap_bo(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_mmap_bo *mmap_bo = arg;

   struct shim_fd *shim_fd = drm_shim_fd_lookup(fd);
   struct shim_bo *bo = drm_shim_bo_lookup(shim_fd, mmap_bo->handle);

   mmap_bo->offset = drm_shim_bo_get_mmap_offset(shim_fd, bo);

   return 0;
}

static int
pan_ioctl_madvise(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_madvise *madvise = arg;

   madvise->retained = 1;

   return 0;
}

static int
pan_ioctl_submit(int fd, unsigned long request, void *arg)
{
   struct drm_panfrost_submit *submit = arg;

   static mali_atom_id atom_number;

   struct mali_jd_atom_v2 atom = {
      .jc = submit->jc,
      .atom_number = atom_number++,
   };

   if (submit->requirements & PANFROST_JD_REQ_FS)
      atom.core_req = 1;
   else
      atom.core_req = 2;

   struct mali_ioctl_job_submit ksubmit = {
      .nr_atoms = 1,
      .stride = sizeof(struct mali_jd_atom_v2),
      .addr = &atom,
   };

   LOG("submit %"PRIx64"\n", (uint64_t) submit->jc);
   int ret = drmIoctl(kbase_fd, MALI_IOCTL_JOB_SUBMIT, &ksubmit);
   LOG("submit done\n");
   if (ret == -1)
      perror("ioctl(job_submit)");

   struct pollfd pollfd = {
       .fd = kbase_fd,
       .events = POLLIN,
   };

   poll(&pollfd, 1, -1);

   if (pollfd.revents != POLLIN)
       fprintf(stderr, "poll on kbase_fd didn't return POLLIN!\n");

   struct mali_jd_event_v2 event = {0};
   read(kbase_fd, &event, sizeof(event));

   if (event.event_code != 1)
       fprintf(stderr, "Event 0x%x for atom %i\n", event.event_code, event.atom_number);

   /* Assume the event was because our job finished, don't bother
    * looping */

   if (atom.atom_number != event.atom_number)
      LOG("atom: %i -> %i\n", atom.atom_number, event.atom_number);

   frostwrap_dump();

   return 0;
}

static ioctl_fn_t driver_ioctls[] = {
   [DRM_PANFROST_SUBMIT] = pan_ioctl_submit,
   [DRM_PANFROST_WAIT_BO] = pan_ioctl_noop,
   [DRM_PANFROST_CREATE_BO] = pan_ioctl_create_bo,
   [DRM_PANFROST_MMAP_BO] = pan_ioctl_mmap_bo,
   [DRM_PANFROST_GET_PARAM] = pan_ioctl_get_param,
   [DRM_PANFROST_GET_BO_OFFSET] = pan_ioctl_noop,
   [DRM_PANFROST_PERFCNT_ENABLE] = pan_ioctl_noop,
   [DRM_PANFROST_PERFCNT_DUMP] = pan_ioctl_noop,
   [DRM_PANFROST_MADVISE] = pan_ioctl_madvise,
};

void
drm_shim_driver_init(void)
{
   shim_device.bus_type = DRM_BUS_PLATFORM;
   shim_device.driver_name = "panfrost";
   shim_device.driver_ioctls = driver_ioctls;
   shim_device.driver_ioctl_count = ARRAY_SIZE(driver_ioctls);

   /* panfrost uses the DRM version to expose features, instead of getparam. */
   shim_device.version_major = 1;
   shim_device.version_minor = 1;
   shim_device.version_patchlevel = 0;

   drm_shim_override_file("DRIVER=panfrost\n"
                          "OF_FULLNAME=/soc/mali\n"
                          "OF_COMPATIBLE_0=arm,mali-t860\n"
                          "OF_COMPATIBLE_N=1\n",
                          "/sys/dev/char/%d:%d/device/uevent", DRM_MAJOR,
                          render_node_minor);
}
