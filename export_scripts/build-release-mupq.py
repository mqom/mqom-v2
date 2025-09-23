import os, shutil

DESTINATION_PATH = os.path.dirname( __file__ ) + '/release_mupq'
MQOM2_C_SOURCE_CODE_FOLDER = os.path.dirname( __file__ ) + '/../'

MQOM2_C_SOURCE_CODE_SUBFOLDERS = ['blc', 'fields', 'piop', 'rijndael']
MQOM2_C_SOURCE_CODE_FILES = [
    'api.h',
    'common.h',
    'benchmark.h',
    'enc.h',
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
    'sign.h',
    'xof.c',
    'xof.h',
    'blc/blc_default.c',
    'blc/blc_default.h',
    'blc/blc_memopt.c',
    'blc/blc_memopt.h',
    'blc/blc.h',
    'fields/fields_handling.h',
    'fields/fields_avx2.h',
    'fields/fields_avx512.h',
    'fields/fields_common.h',
    'fields/fields_ref.h',
    'fields/gf256_mult_table.h',
    'piop/piop_cache.h',
    'piop/piop_default.c',
    'piop/piop_default.h',
    'piop/piop_memopt.c',
    'piop/piop_memopt.h',
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
    'rijndael/rijndael.h',
]

def copy_folder(src_path, dst_path, only_root=False):
    for root, dirs, files in os.walk(src_path):
        subpath = root[len(src_path)+1:]
        root_created = False
        for filename in files:
            _, file_extension = os.path.splitext(filename)
            if file_extension in ['.h', '.c' ]:
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
                instance_path = os.path.join(
                    DESTINATION_PATH, 'crypto_sign',
                    TARGET_TMPL.format(l, f'gf{2**field}', trade_off, variant), 'ref'
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
                            TARGET_TMPL.format(1, f'gf16', "fast", "r5"), 'ref'
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
                parameters += "/* Options activated for memory optimization */\n#define MEMORY_EFFICIENT_BLC\n#define MEMORY_EFFICIENT_PIOP\n#define MEMORY_EFFICIENT_KEYGEN\n"
                parameters += "#define USE_ENC_X8\n#define USE_XOF_X4\n\n"
                parameters += "/* Specifically target MUPQ */\n#define MQOM2_FOR_MUPQ\n\n/* Do not mess with sections as the PQM4 framework uses them */\n"
                parameters += "#define NO_EMBEDDED_SRAM_SECTION\n\n#endif /* __PARAMETERS_H__ */\n"
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
