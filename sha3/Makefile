CC?=gcc
WARNING_FLAGS?=-Wall -Wextra -Wpedantic -Wno-unused-function

CFLAGS?= -O3 $(CFLAGS_PLATFORM) $(WARNING_FLAGS) -std=gnu99 
CFLAGS+=$(EXTRA_CFLAGS) $(EXTRA_WARNING_FLAGS)

# Do we have a C++ compiler instead of a C compiler?
GPP := $(shell $(CROSS_COMPILE)$(CC) -v 2>&1 | grep g++)
CLANGPP := $(shell echo $(CROSS_COMPILE)$(CC) | grep clang++)
ifneq ($(GPP),)
CFLAGS := $(patsubst -std=gnu99, -std=gnu++2a, $(CFLAGS))
endif
ifneq ($(CLANGPP),)
CFLAGS := $(patsubst -std=gnu99, -std=gnu++2a, $(CFLAGS))
CFLAGS += -Wno-deprecated
endif

CFLAGS += -DNO_MISALIGNED_ACCESSES

COMMON_SOURCES=KeccakHash.c KeccakSponge.c KeccakSpongetimes4.c KeccakHashtimes4.c 
COMMON_OBJECTS=KeccakHash.o KeccakSponge.o KeccakSpongetimes4.o KeccakHashtimes4.o 

ifeq ($(KECCAK_PLATFORM),avx2)
	INCLUDE_PATHS=avx2
	SPECIFIC_SOURCES=avx2/KeccakP-1600-AVX2.S avx2/KeccakP-1600-times4-SIMD256.c
	SPECIFIC_OBJECTS=avx2/KeccakP-1600-AVX2.o avx2/KeccakP-1600-times4-SIMD256.o
	PLATFORM=1
	CFLAGS_PLATFORM += -mavx2
endif
ifeq ($(KECCAK_PLATFORM),opt64)
	INCLUDE_PATHS=opt64
	SPECIFIC_SOURCES=opt64/KeccakP-1600-opt64.c opt64/KeccakP-1600-times4-on1.c
	SPECIFIC_OBJECTS=opt64/KeccakP-1600-opt64.o opt64/KeccakP-1600-times4-on1.o
	PLATFORM=1
endif
ifeq ($(KECCAK_PLATFORM),plain32)
	INCLUDE_PATHS=plain32
	SPECIFIC_SOURCES=plain32/KeccakP-1600-inplace32BI.c plain32/KeccakP-1600-times4-on1.c
	SPECIFIC_OBJECTS=plain32/KeccakP-1600-inplace32BI.o plain32/KeccakP-1600-times4-on1.o
	PLATFORM=1
endif

# Use the sanitizers
ifeq ($(USE_SANITIZERS),1)
CFLAGS += -fsanitize=undefined -fsanitize=address -fsanitize=leak
  # XXX: NOTE: we must remove the signed integers overflows from the
  # sanitizers since we "abuse" them in the arithmetic library
  CFLAGS += -fno-sanitize=signed-integer-overflow
endif
ifeq ($(WERROR), 1)
  # Sometimes "-Werror" might be too much, we only use it when asked to
  CFLAGS += -Werror
endif

HEADERS_INCLUDE=$(patsubst %,-I./%,$(INCLUDE_PATHS))
SOURCES=$(COMMON_SOURCES) $(SPECIFIC_SOURCES)
OBJECTS=$(COMMON_OBJECTS) $(SPECIFIC_OBJECTS)
SHA3LIB=libhash.a

%.o : %.c
ifneq ($(PLATFORM),1)
	$(error "Sorry: no platform specified ... Please select KECCAK_PLATFORM=avx2, or opt64, or plain32")
endif
	$(CC) $(CFLAGS) -c -I. $(HEADERS_INCLUDE) $< -o $@

all: $(SOURCES) $(SHA3LIB)

$(SHA3LIB): $(OBJECTS)
	ar rcs $@ $^

%.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@

headers:
	@echo HEADERS=$(INCLUDE_PATHS)

clean:
	rm $(COMMON_OBJECTS) $(SPECIFIC_OBJECTS) 2>/dev/null || true
	rm $(SHA3LIB) 2>/dev/null || true
