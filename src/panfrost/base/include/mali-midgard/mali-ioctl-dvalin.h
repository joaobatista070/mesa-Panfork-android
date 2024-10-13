/*
 * © Copyright 2017-2018 The Panfrost Community
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU license.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */

#ifndef __MALI_IOCTL_DVALIN_H__
#define __MALI_IOCTL_DVALIN_H__

union mali_ioctl_mem_alloc {
	union mali_ioctl_header header;
	/* [in] */
	struct {
		u64 va_pages;
		u64 commit_pages;
		u64 extent;
		u64 flags;
	} in;
	struct {
		u64 flags;
		u64 gpu_va;
	} out;
} __attribute__((packed));

struct mali_ioctl_get_gpuprops {
	u64 buffer;
	u32 size;
	u32 flags;
};

struct mali_ioctl_mem_exec_init {
    u64 va_pages;
};

#define MALI_IOCTL_TYPE_BASE  0x80
#define MALI_IOCTL_TYPE_MAX   0x81

#define MALI_IOCTL_TYPE_COUNT (MALI_IOCTL_TYPE_MAX - MALI_IOCTL_TYPE_BASE + 1)

#define MALI_IOCTL_GET_VERSION             (_IOWR(0x80,  0, struct mali_ioctl_get_version))
#define MALI_IOCTL_SET_FLAGS               ( _IOW(0x80,  1, struct mali_ioctl_set_flags))
#define MALI_IOCTL_JOB_SUBMIT              ( _IOW(0x80,  2, struct mali_ioctl_job_submit))
#define MALI_IOCTL_GET_GPUPROPS            ( _IOW(0x80,  3, struct mali_ioctl_get_gpuprops))
#define MALI_IOCTL_POST_TERM               (  _IO(0x80,  4))
#define MALI_IOCTL_MEM_ALLOC               (_IOWR(0x80,  5, union mali_ioctl_mem_alloc))
#define MALI_IOCTL_MEM_QUERY               (_IOWR(0x80,  6, struct mali_ioctl_mem_query))
#define MALI_IOCTL_MEM_FREE                ( _IOW(0x80,  7, struct mali_ioctl_mem_free))
#define MALI_IOCTL_HWCNT_SETUP             ( _IOW(0x80,  8, __ioctl_placeholder))
#define MALI_IOCTL_HWCNT_ENABLE            ( _IOW(0x80,  9, __ioctl_placeholder))
#define MALI_IOCTL_HWCNT_DUMP              (  _IO(0x80, 10))
#define MALI_IOCTL_HWCNT_CLEAR             (  _IO(0x80, 11))
#define MALI_IOCTL_DISJOINT_QUERY          ( _IOR(0x80, 12, __ioctl_placeholder))
// Get DDK version
// mem jit init
#define MALI_IOCTL_SYNC                    ( _IOW(0x80, 15, struct mali_ioctl_sync))
#define MALI_IOCTL_FIND_CPU_OFFSET         (_IOWR(0x80, 16, __ioctl_placeholder))
#define MALI_IOCTL_GET_CONTEXT_ID          ( _IOR(0x80, 17, struct mali_ioctl_get_context_id))
// TLStream acquire
// TLStream Flush
#define MALI_IOCTL_MEM_COMMIT              ( _IOW(0x80, 20, struct mali_ioctl_mem_commit))
#define MALI_IOCTL_MEM_ALIAS               (_IOWR(0x80, 21, struct mali_ioctl_mem_alias))
#define MALI_IOCTL_MEM_IMPORT              (_IOWR(0x80, 22, struct mali_ioctl_mem_import))
#define MALI_IOCTL_MEM_FLAGS_CHANGE        ( _IOW(0x80, 23, struct mali_ioctl_mem_flags_change))
#define MALI_IOCTL_STREAM_CREATE           ( _IOW(0x80, 24, struct mali_ioctl_stream_create))
#define MALI_IOCTL_FENCE_VALIDATE          ( _IOW(0x80, 25, __ioctl_placeholder))
#define MALI_IOCTL_GET_PROFILING_CONTROLS  ( _IOW(0x80, 26, __ioctl_placeholder))
#define MALI_IOCTL_DEBUGFS_MEM_PROFILE_ADD ( _IOW(0x80, 27, __ioctl_placeholder))
// Soft event update
// sticky resource map
// sticky resource unmap
// Find gpu start and offset
#define MALI_IOCTL_HWCNT_SET               ( _IOW(0x80, 32, __ioctl_placeholder))

// https://github.com/ianmacd/d2s/blob/master/drivers/gpu/arm/b_r16p0/mali_kbase_core_linux.c
#define MALI_IOCTL_MEM_EXEC_INIT           ( _IOW(0x80, 38, struct mali_ioctl_mem_exec_init))
// gwt start
// gwt stop
// gwt dump
/// Begin TEST type region
/// End TEST type region

/**********************************
 * Definitions for GPU properties *
 **********************************/
#define MALI_GPUPROP_VALUE_SIZE_U8	(0x0)
#define MALI_GPUPROP_VALUE_SIZE_U16	(0x1)
#define MALI_GPUPROP_VALUE_SIZE_U32	(0x2)
#define MALI_GPUPROP_VALUE_SIZE_U64	(0x3)

#define MALI_GPUPROP_PRODUCT_ID			1
#define MALI_GPUPROP_VERSION_STATUS			2
#define MALI_GPUPROP_MINOR_REVISION			3
#define MALI_GPUPROP_MAJOR_REVISION			4
/* 5 previously used for GPU speed */
#define MALI_GPUPROP_GPU_FREQ_KHZ_MAX			6
/* 7 previously used for minimum GPU speed */
#define MALI_GPUPROP_LOG2_PROGRAM_COUNTER_SIZE		8
#define MALI_GPUPROP_TEXTURE_FEATURES_0		9
#define MALI_GPUPROP_TEXTURE_FEATURES_1		10
#define MALI_GPUPROP_TEXTURE_FEATURES_2		11
#define MALI_GPUPROP_GPU_AVAILABLE_MEMORY_SIZE		12

#define MALI_GPUPROP_L2_LOG2_LINE_SIZE			13
#define MALI_GPUPROP_L2_LOG2_CACHE_SIZE		14
#define MALI_GPUPROP_L2_NUM_L2_SLICES			15

#define MALI_GPUPROP_TILER_BIN_SIZE_BYTES		16
#define MALI_GPUPROP_TILER_MAX_ACTIVE_LEVELS		17

#define MALI_GPUPROP_MAX_THREADS			18
#define MALI_GPUPROP_MAX_WORKGROUP_SIZE		19
#define MALI_GPUPROP_MAX_BARRIER_SIZE			20
#define MALI_GPUPROP_MAX_REGISTERS			21
#define MALI_GPUPROP_MAX_TASK_QUEUE			22
#define MALI_GPUPROP_MAX_THREAD_GROUP_SPLIT		23
#define MALI_GPUPROP_IMPL_TECH				24

#define MALI_GPUPROP_RAW_SHADER_PRESENT		25
#define MALI_GPUPROP_RAW_TILER_PRESENT			26
#define MALI_GPUPROP_RAW_L2_PRESENT			27
#define MALI_GPUPROP_RAW_STACK_PRESENT			28
#define MALI_GPUPROP_RAW_L2_FEATURES			29
#define MALI_GPUPROP_RAW_CORE_FEATURES			30
#define MALI_GPUPROP_RAW_MEM_FEATURES			31
#define MALI_GPUPROP_RAW_MMU_FEATURES			32
#define MALI_GPUPROP_RAW_AS_PRESENT			33
#define MALI_GPUPROP_RAW_JS_PRESENT			34
#define MALI_GPUPROP_RAW_JS_FEATURES_0			35
#define MALI_GPUPROP_RAW_JS_FEATURES_1			36
#define MALI_GPUPROP_RAW_JS_FEATURES_2			37
#define MALI_GPUPROP_RAW_JS_FEATURES_3			38
#define MALI_GPUPROP_RAW_JS_FEATURES_4			39
#define MALI_GPUPROP_RAW_JS_FEATURES_5			40
#define MALI_GPUPROP_RAW_JS_FEATURES_6			41
#define MALI_GPUPROP_RAW_JS_FEATURES_7			42
#define MALI_GPUPROP_RAW_JS_FEATURES_8			43
#define MALI_GPUPROP_RAW_JS_FEATURES_9			44
#define MALI_GPUPROP_RAW_JS_FEATURES_10		45
#define MALI_GPUPROP_RAW_JS_FEATURES_11		46
#define MALI_GPUPROP_RAW_JS_FEATURES_12		47
#define MALI_GPUPROP_RAW_JS_FEATURES_13		48
#define MALI_GPUPROP_RAW_JS_FEATURES_14		49
#define MALI_GPUPROP_RAW_JS_FEATURES_15		50
#define MALI_GPUPROP_RAW_TILER_FEATURES		51
#define MALI_GPUPROP_RAW_TEXTURE_FEATURES_0		52
#define MALI_GPUPROP_RAW_TEXTURE_FEATURES_1		53
#define MALI_GPUPROP_RAW_TEXTURE_FEATURES_2		54
#define MALI_GPUPROP_RAW_GPU_ID			55
#define MALI_GPUPROP_RAW_THREAD_MAX_THREADS		56
#define MALI_GPUPROP_RAW_THREAD_MAX_WORKGROUP_SIZE	57
#define MALI_GPUPROP_RAW_THREAD_MAX_BARRIER_SIZE	58
#define MALI_GPUPROP_RAW_THREAD_FEATURES		59
#define MALI_GPUPROP_RAW_COHERENCY_MODE		60

#define MALI_GPUPROP_COHERENCY_NUM_GROUPS		61
#define MALI_GPUPROP_COHERENCY_NUM_CORE_GROUPS		62
#define MALI_GPUPROP_COHERENCY_COHERENCY		63
#define MALI_GPUPROP_COHERENCY_GROUP_0			64
#define MALI_GPUPROP_COHERENCY_GROUP_1			65
#define MALI_GPUPROP_COHERENCY_GROUP_2			66
#define MALI_GPUPROP_COHERENCY_GROUP_3			67
#define MALI_GPUPROP_COHERENCY_GROUP_4			68
#define MALI_GPUPROP_COHERENCY_GROUP_5			69
#define MALI_GPUPROP_COHERENCY_GROUP_6			70
#define MALI_GPUPROP_COHERENCY_GROUP_7			71
#define MALI_GPUPROP_COHERENCY_GROUP_8			72
#define MALI_GPUPROP_COHERENCY_GROUP_9			73
#define MALI_GPUPROP_COHERENCY_GROUP_10		74
#define MALI_GPUPROP_COHERENCY_GROUP_11		75
#define MALI_GPUPROP_COHERENCY_GROUP_12		76
#define MALI_GPUPROP_COHERENCY_GROUP_13		77
#define MALI_GPUPROP_COHERENCY_GROUP_14		78
#define MALI_GPUPROP_COHERENCY_GROUP_15		79

#define MALI_GPUPROP_TEXTURE_FEATURES_3		80
#define MALI_GPUPROP_RAW_TEXTURE_FEATURES_3		81

#define MALI_GPUPROP_NUM_EXEC_ENGINES			82

#define MALI_GPUPROP_RAW_THREAD_TLS_ALLOC		83
#define MALI_GPUPROP_TLS_ALLOC				84
#define MALI_GPUPROP_RAW_GPU_FEATURES			85

#endif /* __MALI_IOCTL_DVALIN_H__ */
