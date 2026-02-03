import os, shutil, sys

DESTINATION_PATH = os.path.dirname( __file__ ) + '/release_mupq'
MQOM2_C_SOURCE_CODE_FOLDER = os.path.dirname( __file__ ) + '/../../mqom2_ref'

MQOM2_C_SOURCE_CODE_SUBFOLDERS = ['blc', 'fields', 'fields_bitsliced', 'piop', 'rijndael']
MQOM2_C_SOURCE_CODE_FILES = [
    'api.h',
    'common.h',
    'benchmark.h',
    'enc.h',
    'enc_local.h',
    'enc_mupq.h',
    'enc_liboqs.h',
    'expand_mq.c',
    'expand_mq.h',
    'fields.h',
    'ggm_tree.c',
    'ggm_tree.h',
    'keygen.c',
    'keygen.h',
    'mqom2_parameters.h',
    'prg.h',
    'prg.c',
    'prg_cache.h',
    'sign.c',
    'sign_memopt.c',
    'sign.h',
    'crypto_sign.c',
    'xof.c',
    'xof.h',
    'blc/seed_commit.h',
    'blc/seed_commit_default.h',
    'blc/seed_commit_memopt.h',
    'blc/blc_default.c',
    'blc/blc_default.h',
    'blc/blc_memopt.c',
    'blc/blc_memopt.h',
    'blc/blc_memopt_common.h',
    'blc/blc_memopt_x1.c',
    'blc/blc_memopt_x2.c',
    'blc/blc_memopt_x4.c',
    'blc/blc_common.h',
    'blc/blc.h',
    'fields/fields_handling.h',
    'fields/fields_avx2.h',
    'fields/fields_avx512.h',
    'fields/fields_common.h',
    'fields/fields_ref.h',
    'fields/gf256_mult_table.h',
    'fields_bitsliced.h',
    'fields_bitsliced/fields_bitsliced_branchconst_composite.h',
    'fields_bitsliced/fields_bitsliced_branchconst.h',
    'piop/piop_cache.h',
    'piop/piop_default.c',
    'piop/piop_default.h',
    'piop/piop_memopt.c',
    'piop/piop_memopt.h',
    'piop/piop_bitslice.c',
    'piop/piop_bitslice.h',
    'piop/piop.h',
    'rijndael/rijndael_aes_ni.c',
    'rijndael/rijndael_aes_ni.h',
    'rijndael/rijndael_common.h',
    'rijndael/rijndael_ct64_enc.h',
    'rijndael/rijndael_ct64.c',
    'rijndael/rijndael_ct64.h',
    'rijndael/rijndael_platform.h',
    'rijndael/rijndael_ref.c',
    'rijndael/rijndael_ref.h',
    'rijndael/rijndael_table.c',
    'rijndael/rijndael_table.h',
    'rijndael/rijndael_external.c',
    'rijndael/rijndael_external.h',
    'rijndael/rijndael.h',
    'LICENSE',
]

def copy_folder(src_path, dst_path, only_root=False):
    for root, dirs, files in os.walk(src_path):
        subpath = root[len(src_path)+1:]
        root_created = False
        for filename in files:
            _, file_extension = os.path.splitext(filename)
            if file_extension in ['.h', '.c' ] or filename in ['LICENSE']:
                if not root_created:
                    os.makedirs(os.path.join(dst_path, subpath))
                    root_created = True
                shutil.copyfile(
                    os.path.join(src_path, subpath, filename),
                    os.path.join(dst_path, subpath, filename)
                )


shutil.rmtree(DESTINATION_PATH, ignore_errors=True)
os.makedirs(DESTINATION_PATH)

TARGET_TMPL = "mqom2_cat{}_{}_{}_{}"
LEVELS = [1, 3, 5]
FIELDS = [4, 1, 8]
TRADE_OFFS = ["fast", "short"]
VARIANTS = ["r5", "r3"]

for l in LEVELS:
    for field in FIELDS:
        for trade_off in TRADE_OFFS:
            for variant in VARIANTS:
                for impl in ['balanced', 'memopt', 'ref']:
                    instance_path = os.path.join(
                        DESTINATION_PATH, 'crypto_sign',
                        TARGET_TMPL.format(l, f'gf{2**field}', trade_off, variant), impl
                    )
                    shutil.rmtree(instance_path, ignore_errors=True)
                    os.makedirs(instance_path)
                    for filename in MQOM2_C_SOURCE_CODE_FILES:
                        if l == 1 and field == 4 and trade_off == "fast" and variant == "r5":
                            shutil.copyfile(
                                os.path.join(MQOM2_C_SOURCE_CODE_FOLDER, filename),
                                os.path.join(instance_path, os.path.split(filename)[1])
                            )
                        else:
                            # Create symlinks for common files
                            base_path = os.path.join(
                                '../../',
                                TARGET_TMPL.format(1, f'gf16', "fast", "r5"), impl
                            )
                            os.symlink(os.path.join(base_path, os.path.split(filename)[1]), os.path.join(instance_path, os.path.split(filename)[1]))
    
                    shutil.copyfile(
                        os.path.join(MQOM2_C_SOURCE_CODE_FOLDER, 'parameters', f'mqom2_parameters_cat{l}-gf{2**field}-{trade_off}-{variant}.h'),
                        os.path.join(instance_path, f'mqom2_parameters_cat{l}-gf{2**field}-{trade_off}-{variant}.h')
                    )
    
                    # Generate "parameters.h" with the proper parameters
                    parameters = "#ifndef __PARAMETERS_H__\n#define __PARAMETERS_H__\n\n"
                    if l == 1:
                        parameters += "#define MQOM2_PARAM_SECURITY 128\n"
                    elif l == 3:
                        parameters += "#define MQOM2_PARAM_SECURITY 192\n"
                    else:                    
                        parameters += "#define MQOM2_PARAM_SECURITY 256\n"
                    #
                    parameters += ("#define MQOM2_PARAM_BASE_FIELD %d\n" % field)
                    #
                    if trade_off == "short":
                        parameters += "#define MQOM2_PARAM_TRADEOFF 1\n"
                    else:
                        parameters += "#define MQOM2_PARAM_TRADEOFF 0\n"
                    #
                    if variant == "r3":
                        parameters += "#define MQOM2_PARAM_NBROUNDS 3\n\n"
                    else:
                        parameters += "#define MQOM2_PARAM_NBROUNDS 5\n\n"
                    #
                    parameters += "/* Fields conf: ref implementation */\n#define FIELDS_REF\n"
                    parameters += "/* Rijndael conf: bitslice (actually underlying MUPQ implementation for cat1 with the MQOM2_FOR_MUPQ toggle) */\n#define RIJNDAEL_BITSLICE\n"
                    # Opt specific to the implementation
                    if impl == 'balanced':
                        parameters += "/* Options activated for memory optimization */\n#define MEMORY_EFFICIENT_BLC\n#define PIOP_BITSLICE\n#define FIELDS_BITSLICE_COMPOSITE\n#define FIELDS_BITSLICE_PUBLIC_JUMP\n#define BLC_INTERNAL_X2\n#define GGMTREE_NB_ENC_CTX_IN_MEMORY 3\n#define MEMORY_EFFICIENT_KEYGEN\n"
                        parameters += "#define USE_ENC_X8\n#define USE_XOF_X4\n\n"
                        parameters += "/* Specifically target MUPQ */\n#define MQOM2_FOR_MUPQ\n\n/* Do not mess with sections as the PQM4 framework uses them */\n"
                        parameters += "#define NO_EMBEDDED_SRAM_SECTION\n\n#endif /* __PARAMETERS_H__ */\n"
                    elif impl == 'memopt':
                        parameters += "/* Options activated for memory optimization */\n#define MEMORY_EFFICIENT_BLC\n#define MEMORY_EFFICIENT_PIOP\n#define GGMTREE_NB_ENC_CTX_IN_MEMORY 0\n#define MEMORY_EFFICIENT_KEYGEN\n#define VERIFY_MEMOPT\n#define PRG_ONE_RIJNDAEL_CTX\n#define SEED_COMMIT_MEMOPT\n#define PIOP_NB_PARALLEL_REPETITIONS_SIGN 9\n#define PIOP_NB_PARALLEL_REPETITIONS_VERIFY 4\n"
                        parameters += "/* Specifically target MUPQ */\n#define MQOM2_FOR_MUPQ\n\n/* Do not mess with sections as the PQM4 framework uses them */\n"
                        parameters += "#define NO_EMBEDDED_SRAM_SECTION\n\n#endif /* __PARAMETERS_H__ */\n"
                    elif impl == 'ref':
                        # "Reference" implementation, mostly here for "host" side tests of MUPQ
                        parameters += "/* Options activated for memory optimization */\n#define MEMORY_EFFICIENT_BLC\n#define MEMORY_EFFICIENT_PIOP\n#define FIELDS_BITSLICE_PUBLIC_JUMP\n#define MEMORY_EFFICIENT_KEYGEN\n"
                        parameters += "/* Specifically target MUPQ */\n#define MQOM2_FOR_MUPQ\n\n/* Do not mess with sections as the PQM4 framework uses them */\n"
                        parameters += "#define NO_EMBEDDED_SRAM_SECTION\n\n#endif /* __PARAMETERS_H__ */\n"
                    else:
                        print("Error: unknown implementation type %s" % impl)
                        sys.exit(-1)
                    with open(os.path.join(instance_path, 'parameters.h'), 'w') as f:
                        f.write(parameters)
    
                    if l == 1 and field == 4 and trade_off == "fast" and variant == "r5":
                        # Patch the generic parameters to include "parameters.h"
                        with open(os.path.join(instance_path, f'mqom2_parameters.h'), 'r') as f:
                            content = f.read()
                        content = content.replace("#define __MQOM2_PARAMETERS_GENERIC_H__\n", "#define __MQOM2_PARAMETERS_GENERIC_H__\n\n#include \"parameters.h\"\n")
                        content = content.replace("\"parameters/mqom2", "\"mqom2")
                        with open(os.path.join(instance_path, f'mqom2_parameters.h'), 'w') as f:
                            f.write(content)
