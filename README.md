# MQOM v2

The repository contains the implementation of the *MQOM-v2* digital signature scheme proposed by the MQOM team. See the [MQOM website](https://mqom.org/) for details. 

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
 * `<base-field>` can be `gf2` or `gf256`;
 * `<trade-off>` can be `short` or `fast`;
 * `<variant>` can be `r3` or `r5`.

You can also use prefixes to select a set of instances. For example, `cat3` refers to all the instances of Category III, `cat1_gf256` refers to all the instances of Category I with GF(256) as base field and `cat5_gf2_short` refers to all the instances of Category V with GF(2) as base field targeting short communication.

Some optimizations (for Rjindael, Keccak, ...) can be selected using environment variable, see Section "Advanced Usage"

## Advanced Usage

To compile a specific instance of a scheme with `Makefile` file, you need to set the preprocessing variables in the compilation toolchain:
 * `MQOM2_PARAM_SECURITY` is the security level in bits, it may be `128` (for Cat I), `192` (for Cat III) and `256` (for Cat V);
 * `MQOM2_PARAM_BASE_FIELD` is the (log2 of the order of) base field, it may be `1` for the field GF(2) and `8` for the field GF(256);
 * `MQOM2_PARAM_TRADEOFF` is the trade-off, it may be `1` for the trade-off "short" and `0` for the trade-off "fast".
 * `MQOM2_PARAM_NBROUNDS` is the variant, it may be `3r` for the sigma variant and `5r` for the 5-round variant.
Those variables will select the file of the desired parameter set in `parameters/`. You can set them using the `EXTRA_CFLAGS` environment variable. You can get the `EXTRA_CFLAGS` that corresponds to a specific instance using the command
```bash
python3 manage.py env <scheme>
```
where `<scheme>` is an existing instance (in the format `<category>_<base-field>_<trade-off>_<variant>`).

Moreover, you can set environment variable to compile some optimizations:
 * *Rjindael implementation*: `RJINDAEL_TABLE=1` selects a table-based optimized (non-constant time) implementation, while `RIJNDAEL_AES_NI=1` selects a constant-time implementation optimized using the AES-NI instruction set. By default, it uses a (slow) reference constant time implementation.
 * *Field implementation*: `FIELDS_REF=1` selects a reference constant-time implementation, `FIELD_AVX2=1` selects an optimized implementation using AVX2 instruction set and `FIELD_GFNI=1` selects an optimized implementation using GFNI instruction set. By default, it select the faster implementation for the current platform.
 * *Keccak implementation*: `KECCAK_AVX2=1` select a Keccak implementation optimized using the AVX2 instruction set. By default, it uses a 64-bit optimized implementation.

