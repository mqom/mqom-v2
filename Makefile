# Compiler detection
# Detect if we are using clang or gcc
CLANG :=  $(shell $(CC) -v 2>&1 | grep clang)
ifeq ($(CLANG),)
  GCC :=  $(shell $(CC) -v 2>&1 | grep gcc)
endif

ifneq ($(CLANG),)
  # get clang version e.g. 14.1.3
  CLANG_VERSION := $(shell $(CROSS_COMPILE)$(CC) -dumpversion)
  # convert to single number e.g. 14 * 100 + 1
  CLANG_VERSION := $(shell echo $(CLANG_VERSION) | cut -f1-2 -d. | sed -e 's/\./*100+/g')
  # Calculate value - e.g. 1401
  CLANG_VERSION := $(shell echo $$(($(CLANG_VERSION))))
  # Comparison results (true if true, empty if false)
  CLANG_VERSION_GTE_12 := $(shell [ $(CLANG_VERSION) -ge 1200 ]  && echo true)
  CLANG_VERSION_GTE_13 := $(shell [ $(CLANG_VERSION) -ge 1300 ]  && echo true)
  CLANG_VERSION_GTE_16 := $(shell [ $(CLANG_VERSION) -ge 1600 ]  && echo true)
  CLANG_VERSION_GTE_17 := $(shell [ $(CLANG_VERSION) -ge 1700 ]  && echo true)
  CLANG_VERSION_GTE_18 := $(shell [ $(CLANG_VERSION) -ge 1800 ]  && echo true)
  CLANG_VERSION_GTE_19 := $(shell [ $(CLANG_VERSION) -ge 1900 ]  && echo true)
endif

# AR and RANLIB
AR ?= ar
RANLIB ?= ranlib

# Basic CFLAGS
CFLAGS ?= -O3 -march=native -mtune=native -Wall -Wextra -Wshadow -DNDEBUG

# Keccak related stuff, in the form of an external library
LIB_HASH_DIR = sha3
LIB_HASH = $(LIB_HASH_DIR)/libhash.a

# Rinjdael related stuff
RIJNDAEL_DIR = rijndael
RIJNDAEL_INCLUDES = $(RIJNDAEL_DIR)
RIJNDAEL_SRC_FILES = $(RIJNDAEL_DIR)/rijndael_ref.c $(RIJNDAEL_DIR)/rijndael_table.c $(RIJNDAEL_DIR)/rijndael_aes_ni.c $(RIJNDAEL_DIR)/rijndael_ct64.c $(RIJNDAEL_DIR)/rijndael_external.c
ifeq ($(RIJNDAEL_OPT_ARMV7M),1)
  # Force RIJNDAEL optimized assembly usage where possible
  CFLAGS += -DRIJNDAEL_OPT_ARMV7M
  ASMFLAGS += -x assembler-with-cpp
  RIJNDAEL_SRC_FILES += $(RIJNDAEL_DIR)/aes128_table_arvmv7m.s $(RIJNDAEL_DIR)/aes128_fixsliced_arvmv7m.s
endif
RIJNDAEL_OBJS   = $(patsubst %.c,%.o, $(filter %.c,$(RIJNDAEL_SRC_FILES)))
RIJNDAEL_OBJS  += $(patsubst %.s,%.o, $(filter %.s,$(RIJNDAEL_SRC_FILES)))
RIJNDAEL_OBJS  += $(patsubst %.S,%.o, $(filter %.S,$(RIJNDAEL_SRC_FILES)))

# BLC related stuff
BLC_DIR = blc
BLC_INCLUDES = $(BLC_DIR)
BLC_SRC_FILES = $(BLC_DIR)/blc_default.c $(BLC_DIR)/blc_memopt.c $(BLC_DIR)/blc_memopt_x1.c $(BLC_DIR)/blc_memopt_x2.c $(BLC_DIR)/blc_memopt_x4.c
BLC_OBJS   = $(patsubst %.c,%.o, $(filter %.c,$(BLC_SRC_FILES)))
BLC_OBJS  += $(patsubst %.s,%.o, $(filter %.s,$(BLC_SRC_FILES)))
BLC_OBJS  += $(patsubst %.S,%.o, $(filter %.S,$(BLC_SRC_FILES)))

# PIOP related stuff
PIOP_DIR = piop
PIOP_INCLUDES = $(PIOP_DIR)
PIOP_SRC_FILES = $(PIOP_DIR)/piop_default.c $(PIOP_DIR)/piop_memopt.c $(PIOP_DIR)/piop_bitslice.c
PIOP_OBJS   = $(patsubst %.c,%.o, $(filter %.c,$(PIOP_SRC_FILES)))
PIOP_OBJS  += $(patsubst %.s,%.o, $(filter %.s,$(PIOP_SRC_FILES)))
PIOP_OBJS  += $(patsubst %.S,%.o, $(filter %.S,$(PIOP_SRC_FILES)))

# Fields related stuff
# TODO
FIELDS_DIR = fields
FIELDS_BITSLICE_DIR = fields_bitsliced
FIELDS_INCLUDES = $(FIELDS_DIR) $(FIELDS_BITSLICE_DIR)

# MQOM2 related elements
MQOM2_DIR = .
MQOM2_INCLUDES = $(MQOM2_DIR)
MQOM2_SRC_FILES = $(MQOM2_DIR)/xof.c $(MQOM2_DIR)/prg.c $(MQOM2_DIR)/ggm_tree.c $(MQOM2_DIR)/expand_mq.c $(MQOM2_DIR)/keygen.c $(MQOM2_DIR)/sign.c $(MQOM2_DIR)/sign_memopt.c $(MQOM2_DIR)/crypto_sign.c
MQOM2_OBJS   = $(patsubst %.c,%.o, $(filter %.c,$(MQOM2_SRC_FILES)))
MQOM2_OBJS  += $(patsubst %.s,%.o, $(filter %.s,$(MQOM2_SRC_FILES)))
MQOM2_OBJS  += $(patsubst %.S,%.o, $(filter %.S,$(MQOM2_SRC_FILES)))

# Extra source files possibly provided by the user
EXTRA_OBJS  =$(patsubst %.c,%.o, $(filter %.c,$(EXTRA_SRC)))
EXTRA_OBJS +=$(patsubst %.s,%.o, $(filter %.s,$(EXTRA_SRC)))
EXTRA_OBJS +=$(patsubst %.S,%.o, $(filter %.S,$(EXTRA_SRC)))

OBJS = $(RIJNDAEL_OBJS) $(BLC_OBJS) $(PIOP_OBJS) $(MQOM2_OBJS) $(EXTRA_OBJS)

ifneq ($(GCC),)
  # Remove gcc's -Warray-bounds and -W-stringop-overflow/-W-stringop-overread as they give many false positives
  CFLAGS += -Wno-array-bounds -Wno-stringop-overflow -Wno-stringop-overread
endif

######## Compilation toggles
## Adjust the optimization targets depending on the platform
ifeq ($(RIJNDAEL_TABLE),1)
  # Table based optimized *non-constant time* Rijndael
  CFLAGS += -DRIJNDAEL_TABLE
endif
ifeq ($(RIJNDAEL_AES_NI),1)
  # AES-NI (requires support on the x86 platform) constant time Rijndael
  CFLAGS += -DRIJNDAEL_AES_NI
endif
ifeq ($(RIJNDAEL_CONSTANT_TIME_REF),1)
  # Reference constant time (slow) Rijndael
  CFLAGS += -DRIJNDAEL_CONSTANT_TIME_REF
endif
ifeq ($(RIJNDAEL_BITSLICE),1)
  # Constant time bitslice Rijndael
  CFLAGS += -DRIJNDAEL_BITSLICE
endif
ifeq ($(RIJNDAEL_CT64_CT_KEYSCHED),1)
  # Force constant time key schedule for the bitsliced ct64 variant
  CFLAGS += -DRIJNDAEL_CT64_CT_KEYSCHED
endif
ifeq ($(RIJNDAEL_TABLE_FORCE_IN_FLASH),1)
  # Force the tables for "rijndael table based" to be in flash
  CFLAGS += -DRIJNDAEL_TABLE_FORCE_IN_FLASH
endif
# External Rijndael: 
ifeq ($(RIJNDAEL_EXTERNAL),1)
  # Externally provided Rijndael
  CFLAGS += -DRIJNDAEL_EXTERNAL
endif

## For fields, we detect if we are on a 64 bit __x86_64__: if this is not the case (32 bits)
## our implementation does not support it (because some intrinsics specifically use 64 bits registers)
DETECT_PLATFORM_X64=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep __x86_64__)
ifeq ($(DETECT_PLATFORM_X64),)
  # On non-x86 and 32 bits x86 platforms, fallback on fields ref
  FIELDS_REF = 1
endif
#
ifeq ($(FIELDS_REF),1)
  # Reference implementation for fields
  CFLAGS += -DFIELDS_REF
endif
ifeq ($(FIELDS_AVX2),1)
  # Force AVX2 implementation for fields
  CFLAGS += -DFIELDS_AVX2
endif
ifeq ($(FIELDS_AVX512),1)
  # Force AVX512 implementation for fields
  CFLAGS += -DFIELDS_AVX512
endif
ifeq ($(USE_GF256_TABLE_MULT),1)
  # Use non-constant time GF(256) 65 kB table (in flash/ROM) multiplication
  CFLAGS += -DUSE_GF256_TABLE_MULT
  ifeq ($(GF256_MULT_TABLE_SRAM),1)
    # Large multiplication table forced to be in SRAM instead of flash/ROM
    CFLAGS += -DGF256_MULT_TABLE_SRAM
  endif
endif
ifeq ($(USE_GF256_TABLE_LOG_EXP),1)
  # Use log/exp tables in SRAM for GF(256) multiplication: should be constant
  # time on platforms with no cache to SRAM
  CFLAGS += -DUSE_GF256_TABLE_LOG_EXP
endif
ifeq ($(NO_FIELDS_REF_SWAR_OPT),1)
  # Force the fact that we do NOT use the SWAR (SIMD within a register) optimization for
  # fields reference implementation
  CFLAGS += -DNO_FIELDS_REF_SWAR_OPT
endif

ifeq ($(NO_GFNI),1)
  # Force NO GFNI automatic usage when detected
  CFLAGS += -DNO_GFNI
endif
# Adjust the benchmarking mode: by default we measure time unless stated
# otherwise
ifneq ($(NO_BENCHMARK_TIME),1)
  CFLAGS += -DBENCHMARK_TIME
endif
# Activate benchmarking
ifeq ($(BENCHMARK),1)
  CFLAGS += -DBENCHMARK -DBENCHMARK_CYCLES
endif

# Disable the PRG cache for time / memory trade-off optimization
# The cache is activated by default
ifneq ($(USE_PRG_CACHE),0)
  CFLAGS += -DUSE_PRG_CACHE
endif
ifeq ($(NO_EXPANDMQ_PRG_CACHE),1)
  CFLAGS += -DNO_EXPANDMQ_PRG_CACHE
endif
ifeq ($(NO_BLC_PRG_CACHE),1)
  CFLAGS += -DNO_BLC_PRG_CACHE
endif

# Force the usage of only one Rijndael context for PRG and PRG_pub
# (only true for x1 variants, obviously nonsense for x2, x4 and x8 variants)
ifeq ($(PRG_ONE_RIJNDAEL_CTX),1)
  CFLAGS += -DPRG_ONE_RIJNDAEL_CTX
endif
# Memory optimized SeedCommit, only using one Rijndael context
ifeq ($(SEED_COMMIT_MEMOPT),1)
  CFLAGS += -DSEED_COMMIT_MEMOPT
endif

# Disable the PIOP cache for time / memory trade-off optimization
# The cache is activated by default
ifneq ($(USE_PIOP_CACHE),0)
  ifeq ($(MEMORY_EFFICIENT_PIOP),1)
    # Error: USE_PIOP_CACHE and MEMORY_EFFICIENT_PIOP are not compatible
    $(error Error: USE_PIOP_CACHE and MEMORY_EFFICIENT_PIOP are not compatible!)
  endif
  CFLAGS += -DUSE_PIOP_CACHE
endif
# Use the XOF x4 acceleration by default
ifneq ($(USE_XOF_X4),0)
  CFLAGS += -DUSE_XOF_X4
endif
# Activate optimizing memory for the seed trees
ifeq ($(MEMORY_EFFICIENT_BLC),1)
  CFLAGS += -DMEMORY_EFFICIENT_BLC
endif
# Useful parameters for memopt BLC
ifneq ($(BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE),)
  CFLAGS += -DBLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE=$(BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE)
endif
ifeq ($(BLC_INTERNAL_X4),1)
  CFLAGS += -DBLC_INTERNAL_X4
endif
ifeq ($(BLC_INTERNAL_X2),1)
  CFLAGS += -DBLC_INTERNAL_X2
endif
ifneq ($(GGMTREE_NB_ENC_CTX_IN_MEMORY),)
  CFLAGS += -DGGMTREE_NB_ENC_CTX_IN_MEMORY=$(GGMTREE_NB_ENC_CTX_IN_MEMORY)
endif
# Activate optimizing memory for PIOP
ifeq ($(MEMORY_EFFICIENT_PIOP),1)
  CFLAGS += -DMEMORY_EFFICIENT_PIOP
endif
ifneq ($(PIOP_NB_PARALLEL_REPETITIONS_SIGN),)
  CFLAGS += -DPIOP_NB_PARALLEL_REPETITIONS_SIGN=$(PIOP_NB_PARALLEL_REPETITIONS_SIGN)
endif
ifneq ($(PIOP_NB_PARALLEL_REPETITIONS_VERIFY),)
  CFLAGS += -DPIOP_NB_PARALLEL_REPETITIONS_VERIFY=$(PIOP_NB_PARALLEL_REPETITIONS_VERIFY)
endif
# Activate optimizing memory for Keygen
ifeq ($(MEMORY_EFFICIENT_KEYGEN),1)
  CFLAGS += -DMEMORY_EFFICIENT_KEYGEN
endif
ifeq ($(VERIFY_MEMOPT),1)
  CFLAGS += -DVERIFY_MEMOPT -DMEMORY_EFFICIENT_BLC -DMEMORY_EFFICIENT_PIOP
endif
# Activate optimizing memory for PIOP with bitslicing
ifeq ($(PIOP_BITSLICE),1)
  CFLAGS += -DPIOP_BITSLICE
endif
# Fields bitslice dedicated options
ifeq ($(FIELDS_BITSLICE_COMPOSITE),1)
  CFLAGS += -DFIELDS_BITSLICE_COMPOSITE
endif
ifeq ($(FIELDS_BITSLICE_PUBLIC_JUMP),1)
  CFLAGS += -DFIELDS_BITSLICE_PUBLIC_JUMP
endif

ifneq ($(USE_ENC_X8),0)
  CFLAGS += -DUSE_ENC_X8
endif
# Contexts cleansing
ifeq ($(USE_ENC_CTX_CLEANSING),1)
  CFLAGS += -DUSE_ENC_CTX_CLEANSING
endif

# Do not use allocation probes
ifeq ($(NO_ALLOC_PROBE),1)
  CFLAGS += -DNO_ALLOC_PROBE
endif

# Use the signature buffer as temporary variable storage
ifeq ($(USE_SIGNATURE_BUFFER_AS_TEMP),1)
  CFLAGS += -DUSE_SIGNATURE_BUFFER_AS_TEMP
endif

ifeq ($(NO_NATIVE_TUNE),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
endif

## Toggles to force the platform compilation flags
ifeq ($(FORCE_PLATFORM_REF),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
  # Ref platform uses pure C fields implementation and Rijndael implementation
  CFLAGS += -DFIELDS_REF -DRIJNDAEL_BITSLICE
endif
ifeq ($(FORCE_PLATFORM_AVX2),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
  CFLAGS += -maes -mavx2
endif
ifeq ($(FORCE_PLATFORM_AVX2_GFNI),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
  CFLAGS += -maes -mgfni -mavx2
endif
ifeq ($(FORCE_PLATFORM_AVX512),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
  CFLAGS += -maes -mavx512bw -mavx512f -mavx512vl -mavx512vpopcntdq -mavx512vbmi
endif
ifeq ($(FORCE_PLATFORM_AVX512_GFNI),1)
  CFLAGS := $(subst -march=native,,$(CFLAGS))
  CFLAGS := $(subst -mtune=native,,$(CFLAGS))
  CFLAGS += -maes -mgfni -mavx512bw -mavx512f -mavx512vl -mavx512vpopcntdq -mavx512vbmi
endif

## Togles for various analysis and other useful stuff
# Static analysis of gcc
ifeq ($(FANALYZER),1)
  CFLAGS += -fanalyzer
endif
# Force Link Time Optimizations
# XXX: warning, this can be agressive and yield wrong results with -O3
ifeq ($(FLTO),1)
  CFLAGS += -flto
endif
# Use the sanitizers
ifeq ($(USE_SANITIZERS),1)
CFLAGS += -fsanitize=address -fsanitize=leak -fsanitize=undefined
  ifeq ($(USE_SANITIZERS_IGNORE_ALIGN),1)
    CFLAGS += -fno-sanitize=alignment
  endif
endif
ifeq ($(WERROR), 1)
  # Sometimes "-Werror" might be too much, we only use it when asked to
  CFLAGS += -Werror
endif
ifneq ($(PARAM_SECURITY),)
  # Adjust the security parameters as asked to
  CFLAGS += -DMQOM2_PARAM_SECURITY=$(PARAM_SECURITY)
endif

# "Weak" low-level API
ifeq ($(USE_WEAK_LOW_LEVEL_API),1)
  CFLAGS += -DUSE_WEAK_LOW_LEVEL_API
endif

ifneq ($(DESTINATION_PATH),)
  DESTINATION_PATH := $(DESTINATION_PATH)/
endif
ifneq ($(PREFIX_EXEC),)
PREFIX_EXEC := $(PREFIX_EXEC)_
endif

### Keccak library specific platfrom related flags
# If no platform is specified for Keccak, try to autodetect it
ifeq ($(KECCAK_PLATFORM),)
  KECCAK_DETECT_PLATFORM_AVX512VL=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep AVX512VL)
  KECCAK_DETECT_PLATFORM_AVX512F=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep AVX512F)
  KECCAK_DETECT_PLATFORM_SUB_AVX2=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep AVX2)
  KECCAK_DETECT_PLATFORM_X64=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep __x86_64__)
  KECCAK_DETECT_PLATFORM_AVX512=
  # NOTE: we detect __x86_64__ as our Keccak implementation specifically uses 64 bit registers in assembly
  # (while x86 32 bit platforms might support AVX2 or AVX-512)
  ifneq ($(KECCAK_DETECT_PLATFORM_X64),)
    ifneq ($(KECCAK_DETECT_PLATFORM_AVX512VL),)
      ifneq ($(KECCAK_DETECT_PLATFORM_AVX512F),)
        KECCAK_DETECT_PLATFORM_AVX512=1
      endif
    endif
  endif
  ifneq ($(KECCAK_DETECT_PLATFORM_X64),)
    ifneq ($(KECCAK_DETECT_PLATFORM_SUB_AVX2),)
        KECCAK_DETECT_PLATFORM_AVX2=1
    endif
  endif
  #
  ifneq ($(KECCAK_DETECT_PLATFORM_AVX512),)
      KECCAK_PLATFORM=avx512
  else
    ifneq ($(KECCAK_DETECT_PLATFORM_AVX2),)
      KECCAK_PLATFORM=avx2
    else
      # Possibly detect ARMv7M to use the assembly optimized version
      KECCAK_DETECT_PLATFORM_ARMV7M=$(shell $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -dM -E - < /dev/null 2> /dev/null |egrep -e __ARM_ARCH_7M__ -e __ARM_ARCH_7EM__)
      ifneq ($(KECCAK_DETECT_PLATFORM_ARMV7M),)
          KECCAK_PLATFORM=armv7m
      else
          # No specific platform detected, fallback to opt64
          KECCAK_PLATFORM=opt64
      endif
    endif
  endif
endif
CFLAGS += -DKECCAK_PLATFORM="$(KECCAK_PLATFORM)"
# Adjust the include dir depending on the target platform
LIB_HASH_INCLUDES = $(LIB_HASH_DIR) $(LIB_HASH_DIR)/$(KECCAK_PLATFORM)

# Include the necessary headers
CFLAGS += $(foreach DIR, $(LIB_HASH_INCLUDES), -I$(DIR))
CFLAGS += $(foreach DIR, $(RIJNDAEL_INCLUDES), -I$(DIR))
CFLAGS += $(foreach DIR, $(BLC_INCLUDES), -I$(DIR))
CFLAGS += $(foreach DIR, $(PIOP_INCLUDES), -I$(DIR))
CFLAGS += $(foreach DIR, $(FIELDS_INCLUDES), -I$(DIR))
CFLAGS += $(foreach DIR, $(MQOM2_INCLUDES), -I$(DIR))

# MQOM Variant selection
MQOM2_VARIANT_CFLAGS=
### Cat 1
ifeq ($(MQOM2_VARIANT),cat1-gf2-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf2-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf2-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf2-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf4-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf4-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf4-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf4-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf16-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf16-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf16-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf16-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf256-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf256-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat1-gf256-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat1-gf256-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=128
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
### Cat 3
ifeq ($(MQOM2_VARIANT),cat3-gf2-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf2-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf2-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf2-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf4-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf4-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf4-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf4-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf16-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf16-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf16-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf16-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf256-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf256-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat3-gf256-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat3-gf256-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=192
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
### Cat 5
ifeq ($(MQOM2_VARIANT),cat5-gf2-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf2-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf2-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf2-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf4-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf4-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf4-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf4-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=2
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf16-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf16-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf16-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf16-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=4
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf256-fast-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf256-fast-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=0
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
ifeq ($(MQOM2_VARIANT),cat5-gf256-short-r3)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=3
endif
ifeq ($(MQOM2_VARIANT),cat5-gf256-short-r5)
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_SECURITY=256
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_BASE_FIELD=8
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_TRADEOFF=1
  MQOM2_VARIANT_CFLAGS+=-DMQOM2_PARAM_NBROUNDS=5
endif
################
CFLAGS += $(MQOM2_VARIANT_CFLAGS)

# FLTO (link time optimizations) usage
ifeq ($(FLTO),1)
CFLAGS += -flto
LDFLAGS += -flto
endif

# Possibly append user provided extra CFLAGS
CFLAGS += $(EXTRA_CFLAGS)

KECCAK_OBJS_=$(shell cd sha3/ && CC="$(CC)" KECCAK_PLATFORM="$(KECCAK_PLATFORM)" make --no-print-directory print_objects)
KECCAK_OBJS=$(foreach OBJ, $(KECCAK_OBJS_),sha3/$(OBJ))

all: libhash $(OBJS)

libhash:
	@echo "[+] Compiling libhash"
	cd $(LIB_HASH_DIR) && KECCAK_PLATFORM="$(KECCAK_PLATFORM)" make

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.s.o:
	$(CC) $(CFLAGS) $(ASMFLAGS) -c -o $@ $<

.S.o:
	$(CC) $(CFLAGS) $(ASMFLAGS) -c -o $@ $<

sign: libhash $(OBJS)
	$(CC) $(CFLAGS) generator/PQCgenKAT_sign.c generator/rng.c $(OBJS) $(LIB_HASH) -lcrypto -o $(DESTINATION_PATH)$(PREFIX_EXEC)sign

kat_gen: libhash $(OBJS)
	$(CC) $(CFLAGS) generator/PQCgenKAT_sign.c generator/rng.c $(OBJS) $(LIB_HASH) -lcrypto -o $(DESTINATION_PATH)$(PREFIX_EXEC)kat_gen

kat_check: libhash $(OBJS)
	$(CC) $(CFLAGS) generator/PQCgenKAT_check.c generator/rng.c $(OBJS) $(LIB_HASH) -lcrypto -o $(DESTINATION_PATH)$(PREFIX_EXEC)kat_check

bench: libhash $(OBJS)
	$(CC) $(CFLAGS) benchmark/bench.c benchmark/timing.c $(OBJS) $(LIB_HASH) -lm -o $(DESTINATION_PATH)$(PREFIX_EXEC)bench

bench_mem_keygen: libhash $(OBJS)
	$(CC) $(CFLAGS) benchmark/bench_mem_keygen.c $(OBJS) $(LIB_HASH) -lm -o $(DESTINATION_PATH)$(PREFIX_EXEC)bench_mem_keygen

bench_mem_sign: libhash $(OBJS)
	$(CC) $(CFLAGS) benchmark/bench_mem_sign.c $(OBJS) $(LIB_HASH) -lm -o $(DESTINATION_PATH)$(PREFIX_EXEC)bench_mem_sign

bench_mem_open: libhash $(OBJS)
	$(CC) $(CFLAGS) benchmark/bench_mem_open.c $(OBJS) $(LIB_HASH) -lm -o $(DESTINATION_PATH)$(PREFIX_EXEC)bench_mem_open

test_field_bitslice: libhash $(OBJS)
	$(CC) $(CFLAGS) tests/matmul/test_field_bitslice.c benchmark/timing.c $(OBJS) $(LIB_HASH) -o $(DESTINATION_PATH)$(PREFIX_EXEC)test_field_bitslice

print_objects:
	@echo $(OBJS) && echo $(KECCAK_OBJS)

clean:
	@cd $(LIB_HASH_DIR) && make clean
	@find . -name "*.o" -type f -delete
	@rm -f kat_gen kat_check bench bench_mem_keygen bench_mem_sign bench_mem_open sign mupq_kat_gen test_embedded_KAT
