# MQOM v2

The repository contains the implementation of the *MQOM-v2* digital signature scheme proposed by the MQOM team. See the [MQOM website](https://mqom.org/) for details. 

## License

The MQOM code on this repository is under the MIT license: please check [LICENSE](LICENSE) for more information.
The repo also contains third party code possibly under other licenses, provided by the header in each concerned file (among other sources, MQOM uses code from [XKCP](https://github.com/XKCP/XKCP) for Keccak,
and [BearSSL](https://bearssl.org/) for C bitsliced AES and Rijndael).

## Dependencies

* To build: `Makefile`
* To use the helper `manage.py`: Python3 (version >= 3.6)

## Quick Usage

The `manage.py` Python script aims to ease the use of the MQOM source code. To compile one or several schemes, you can use

```bash
python3 manage.py compile [schemes ...]
```

where `[schemes ...]` is a non-empty list including all the existing MQOM instances with the format `<category>_<base-field>_<trade-off>_<variant>`:
 * `<category>` can be `cat1`, `cat3` or `cat5`;
 * `<base-field>` can be `gf2`, `gf16` or `gf256`;
 * `<trade-off>` can be `short` or `fast`;
 * `<variant>` can be `r3` or `r5`.

You can also use prefixes to select a set of instances. For example, `cat3` refers to all the instances of Category III, `cat1_gf256` refers to all the instances of Category I with GF(256) as base field and `cat5_gf2_short` refers to all the instances of Category V with GF(2) as base field targeting short communication.

Some optimizations (for Rjindael, Keccak, ...) can be selected using environment variable, see Section "Advanced Usage"

It is also possible to directly compile a chosen variant with the provided `Makefile` and the `MQOM2_VARIANT` environment variable. For instance:

```bash
$ make clean && MQOM2_VARIANT=cat1-gf2-fast-r3 make
```

## Advanced Usage

If you want to specifically tune the instance parameters of a scheme with `Makefile` file, you need to set the following preprocessing variables in the compilation toolchain:
 * `MQOM2_PARAM_SECURITY` is the security level in bits, it may be `128` (for Cat I), `192` (for Cat III) and `256` (for Cat V);
 * `MQOM2_PARAM_BASE_FIELD` is the (log2 of the order of) base field, it may be `1` for the field GF(2), `4` for the field GF(16), and `8` for the field GF(256);
 * `MQOM2_PARAM_TRADEOFF` is the trade-off, it may be `1` for the trade-off "short" and `0` for the trade-off "fast".
 * `MQOM2_PARAM_NBROUNDS` is the variant, it may be `3` for the sigma variant and `5` for the 5-round variant.

Those variables will select the file of the desired parameter set in `parameters/`. You can set them using the `EXTRA_CFLAGS` environment variable. You can get the `EXTRA_CFLAGS` that corresponds to a specific instance using the command
```bash
python3 manage.py env <scheme>
```
where `<scheme>` is an existing instance (in the format `<category>_<base-field>_<trade-off>_<variant>`).

Moreover, you can set environment variable to compile some optimizations:
 * *Rjindael implementation*: `RJINDAEL_TABLE=1` selects a table-based optimized (non-constant time) implementation, while `RIJNDAEL_AES_NI=1` selects a constant-time implementation optimized using the AES-NI instruction set. By default, it uses the portable Rijndael bitslice implementation (adapted from [BearSSL](https://bearssl.org/constanttime.html)), a bit slower than the table-based one (but constant time), corresponding to `RIJNDAEL_BITSLICE=1`. Another implementation is also available using `RIJNDAEL_CONSTANT_TIME_REF=1`: it is here mostly for a readable reference constant time implementation, but it is very slow and hence should be avoided. Specifically for Category I and for the ARMv7-M architecture, we have optimized assembly implementations: for table-based implementation we use the [LUT implementation from SAC2016](https://github.com/Ko-/aes-armcortexm), and for bitslice we use the ["fixsliced" implementation from TCHES2021](https://github.com/aadomn/aes). These ARMv7-M optimized implementations can be activated with the `RIJNDAEL_OPT_ARMV7M=1` compilation toggle.
 * *Field implementation*: `FIELDS_REF=1` selects a reference constant-time implementation, `FIELD_AVX2=1` selects an optimized implementation using AVX2 instruction set and `FIELD_AVX512=1` selects an optimized implementation using AVX-512 instruction set. By default, it select the faster implementation for the current platform. Since MQOM is able to use the GFNI instruction set, it is automatically detected and used in the AVX2 and AVX-512 variants, unless the toggle `NO_GFNI=1` is used. Regarding the reference implementation, by default a constant time x4 SWAR (SIMD Within A Register) is used to optimize performance: if you want to use the slower and simpler x1 computation use the `NO_FIELDS_REF_SWAR_OPT=1` toggle. We also provide non-constant time C implementations for GF(256) multiplication to boost performance on platforms where side-channel attacks are not an issue: please use `USE_GF256_TABLE_MULT=1` to have a large 65kB table for field multiplication, or `USE_GF256_TABLE_LOG_EXP=1` to have smaller log/exp tables (these two options are hence exclusive). For the large 65kB table, one can tune either it is `const` (or not) with `GF256_MULT_TABLE_SRAM=0` - the default - (`GF256_MULT_TABLE_SRAM=1`): this is specifically useful for embedded contexts where one must save SRAM.
 * *Keccak implementation*: the best underlying Keccak implementation should be automatically detected at compilation time. You can force `KECCAK_PLATFORM=avx2` to select a Keccak implementation optimized using the AVX2 instruction set, `KECCAK_PLATFORM=avx512` for AVX-512, `KECCAK_PLATFORM=opt64` for the generic optimized 64-bit C implementation. By default when no optimization is detected, it uses a 64-bit optimized implementation. Specifically for ARMv7-M, when this platform is detected, the optimized assembly implementation [from Adomnicai](https://github.com/aadomn/keccak_armv7m): this can also be forced with `KECCAK_PLATFORM=armv7m`.
 * *XOF*: `USE_XOF_X4=1` (this is the default) activates the usage of x4 XOF implementations, while `USE_XOF_X4=0` deactivates it.
 * *PRG and PIOP caches*: `USE_PRG_CACHE=1` and `USE_PIOP_CACHE=1` (default when compiling) activate the caches usage for PRG and PIOP, which significantly accelerate the computations at the expense of more memory usage. To save memory, these can be
explicitly deactivated with `USE_PRG_CACHE=0` and `USE_PIOP_CACHE=0`.
 * *Memory efficient BLC*: `MEMORY_EFFICIENT_BLC=1` (default is 0, deactivated) activates saving memory for BLC trees computations at the expense of slightly more cycles as these are recomputed. Specifically when the memory optimized variant is selected, it is possible to further tune the number of seed commitments per hash update with `BLC_NB_SEED_COMMITMENTS_PER_HASH_UPDATE=x`, the internal usage of x2 and x4 encryption for BLC (by processing 2 or 4 trees in parallel) with `BLC_INTERNAL_X2=1` or `BLC_INTERNAL_X4=1` (default is 0 for both, i.e. `BLC_INTERNAL_X1` one tree processed at a time), the number of parallel encryption contexts in memory for the GGM trees with `GGMTREE_NB_ENC_CTX_IN_MEMORY=x`. This tuning allows to explore various trade-offs for memory consumption versus performance in terms of cycles. Specifically for `BLC_INTERNAL_X1`, the option `PRG_ONE_RIJNDAEL_CTX=1` allows to reduce the number of contexts used in the PRG, hence reducing the memory footprint. Finally, `SEED_COMMIT_MEMOPT=1` further optimizes memory at the cost of cycles.
 * *Memory efficient PIOP*: `MEMORY_EFFICIENT_PIOP=1` (default is 0, deactivated) activates saving memory for the PIOP computation through the streaming generation of the MQ matrices.
 * *Memory efficient Keygen*: `MEMORY_EFFICIENT_KEYGEN=1` (default is 0, deactivated) activates saving memory for the `ExpandEquations` part of the Keygen using a streaming generation of the MQ matrices.
 * *Forcing platforms profiles*: we provide through the `Makefile` five platforms profiles to explicitly select. The `FORCE_PLATFORM_REF=1` toggle forces the pure C Rijndael bitslice and fields reference implementations, while removing `-march=native -mtune=native` from the `CFLAGS`. The `FORCE_PLATFORM_AVX2=1` toggle forces a typical AVX2 with AES-NI platform with `-maes -mavx2`. The `FORCE_PLATFORM_AVX2_GFNI=1` toggle forces an AVX2 with AES-NI and GFNI platform with `-maes -mgfni -mavx2`. The `FORCE_PLATFORM_AVX512=1` toggle forces an AVX-512 platform with AES-NI and the following instructions subsets: ̀`-mavx512bw -mavx512f -mavx512vl -mavx512vpopcntdq -mavx512vbmi`. Finally, `FORCE_PLATFORM_AVX512_GFNI=1` is the same as the previous platform with GFNI.
 * *Using `weak` low-level APIs*: it is possible to make the Rijndael and Keccak symbols `weak` by using the `USE_WEAK_LOW_LEVEL_API=1` toggle. This can be useful when one wants to override a specific implementation for those two primitives. This can be useful e.g. when one wants to replace the AES-128 implementation with a call to a hardware accelerated backend. When using `weak` symbols, the user has to provide the same symbols without the `weak` attributes (i.e. "strong" symbols) to override the default functions.
 * *Bitsliced PIOP across the repetitions*: `PIOP_BITSLICE=1` activates the bitslice implementation of the PIOP across the `tau` repetitions. This is compatible with low-memory usage, although bitslicing takes a minimal amount of memory (hence extreme memory performance cannot be achieved with this option). This is not compatible (exclusive) with `MEMORY_EFFICIENT_PIOP=1`. When this toggle is selected, four constant-time (wrt secrets) variants of bitsliced fields operations are provided (allowing the usage of jumps or not, or composite fields or not): `FIELDS_BITSLICE_COMPOSITE={0,1}` (regular or composite field usage), `FIELDS_BITSLICE_PUBLIC_JUMP={0,1}` (usage of public conditions or not). These four variants are selectable because their performance in terms of cycles might vary depending on the platform (presence of caches, branch prediction units, etc.).
 * *Memory optimized verification*: `VERIFY_MEMOPT=1` activates dedicated further optimizations of the repetitions in the verification algorithm. Beware that this option **must be used with* `MEMORY_EFFICIENT_BLC=1` and `MEMORY_EFFICIENT_PIOP=1` (the [Makefile](Makefile) activates them when the toggle is selected).
 * *Number of parallel repetitions*: `PIOP_NB_PARALLEL_REPETITIONS_SIGN=x` and `PIOP_NB_PARALLEL_REPETITIONS_VERIFY=y` (with `x` and `y` between 1 and `tau`) allow to group the signature and verification PIOPs by parallel repetitions, hence reducing the memory usage for internal buffers at the cost of extra cycles for some recomputations.
 * *Contexts cleansing*: `USE_ENC_CTX_CLEANSING={0,1}` activates or deactivates the cleansing of some (possible sensitive) variables, which can have impacts on performance on embedded platforms (when such cleansing is called in critical inner loops). Default is `0` for performance, but **set to 1** in sensitive contexts.

Further optimizations for embedded platforms with stringent memory footprints are detailed in the [Breaking the Myth of MPCitH Inefficiency: Optimizing MQOM for Embedded Platforms](https://eprint.iacr.org/2026/078.pdf) paper. We also have a [dedicated repository for embedded experiments](https://github.com/mqom/embedded-experiments) (beware that only the stable elements are included in the current upstream repo, notably one-tree, pre-signature and streaming verification are not part of the current source tree, you will have to fetch the dedicated experiments sources).

