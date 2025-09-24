import argparse
import time
import os, json, sys, signal

CATEGORIES = {'cat1': '128', 'cat3': '192', 'cat5': '256'}
BASE_FIELDS = {'gf2': '1', 'gf16': '4', 'gf256': '8'}
TRADE_OFFS = {'short': '1', 'fast': '0'}
VARIANTS = {'r3': '3', 'r5': '5'}

# List all choices
choices_scheme_sets = ['all']
choices_schemes = []
for cat in CATEGORIES:
    choices_scheme_sets.append(f'{cat}')
    for field in BASE_FIELDS:
        choices_scheme_sets.append(f'{cat}_{field}')
        for tradeoff in TRADE_OFFS:
            choices_scheme_sets.append(f'{cat}_{field}_{tradeoff}')
            for variant in VARIANTS:
                choices_scheme_sets.append(f'{cat}_{field}_{tradeoff}_{variant}')
                choices_schemes.append(f'{cat}_{field}_{tradeoff}_{variant}')

# Define the argument parsing
parser = argparse.ArgumentParser()
subparsers = parser.add_subparsers(dest='command', help='sub-command help')

parser_compile = subparsers.add_parser('compile', help='compile scheme')
parser_compile.add_argument('schemes', nargs='+', choices=choices_scheme_sets, help='schemes to compile')
parser_compile.add_argument('--no-kat', action='store_true', dest='b_no_kat', help='Avoid compiling the "kat_gen" and "kat_check" executables')
parser_compile.add_argument('--no-bench', action='store_true', dest='b_no_bench', help='Avoid compiling the "bench" executable')
parser_compile.add_argument('--verbose', action='store_true', dest='b_verbose', help='Activate verbose compilation')
parser_compile.add_argument('-p', '--parallel-jobs', dest='parallel_jobs', type=int, default=0, help='Number of parallel jobs (-1 means max, 0 means monojob)')
parser_compile.add_argument('-o', '--only-print', action='store_true', dest='b_compile_only_print', help='Do not compile, only print the compilation invocation to be used')

parser_set = subparsers.add_parser('env', help='get environment variables')
parser_set.add_argument('scheme', choices=choices_schemes, help='scheme to get')

parser_set = subparsers.add_parser('clean', help='clean compilation objects and build folder')

parser_bench = subparsers.add_parser('bench', help='bench')
parser_bench.add_argument('schemes', nargs='+', choices=choices_scheme_sets, help='schemes to benchmark')
parser_bench.add_argument('-n', '--nb-repetitions', dest='nb_repetitions', type=int, default=100, help='Number of repetitions')
parser_bench.add_argument('-p', '--parallel-jobs', dest='parallel_jobs', type=int, default=0, help='Number of parallel jobs (-1 means max, 0 means monojob)')
parser_bench.add_argument('--verbose', action='store_true', dest='b_verbose', help='Activate verbose benchmarks')
parser_bench.add_argument('--memory', action='store_true', dest='b_bench_memory', help='Bench also memory usage')
parser_bench.add_argument('-o', '--output', dest='b_bench_file_name', help='Specify the output json benchmarking filename')
parser_bench.add_argument('-f', '--build-folder', dest='b_bench_build_folder_name', help='Specify the build folder (default is "build/")')

parser_test = subparsers.add_parser('test', help='test')
parser_test.add_argument('schemes', nargs='+', choices=choices_scheme_sets, help='schemes to test')
parser_test.add_argument('-n', '--nb-repetitions', dest='nb_repetitions', type=int, default=10, help='Number of repetitions')
parser_test.add_argument('-c', '--compare-kat', dest='compare_kat', default=None, help='Compare KAT (provide a ZIP file representing a submission package)')
parser_test.add_argument('--no-kat-check', action='store_true', dest='b_no_kat_check', help='Avoid executing the KAT check (only the gen is executed)')
parser_test.add_argument('--no-valgrind', action='store_true', dest='b_no_valgrind', help='Avoid using valgrind')
parser_test.add_argument('-p', '--parallel-jobs', dest='parallel_jobs', type=int, default=0, help='Number of parallel jobs (-1 means max, 0 means monojob)')
parser_test.add_argument('--verbose', action='store_true', dest='b_verbose', help='Activate verbose tests')
parser_test.add_argument('-f', '--build-folder', dest='b_test_build_folder_name', help='Specify the build folder (default is "build/")')

arguments = parser.parse_args()

# Utility to execute command
def run_command(command, cwd, shell=False):
    import subprocess
    if shell:
        process = subprocess.Popen(command, cwd=cwd, shell=shell,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )    
    else:
        process = subprocess.Popen(command.split(), cwd=cwd,
            stdout=subprocess.PIPE, stderr=subprocess.PIPE
        )
    stdout, stderr = process.communicate()
    stdout = stdout.decode('utf8') if stdout is not None else None
    stderr = stderr.decode('utf8') if stdout is not None else None
    return stdout, stderr

# Utility to get the selected schemes
class MQOMInstance:
    def __init__(self, scheme, dst_path):
        self.scheme = scheme
        self.dst_path = dst_path / self.get_label()
        self.dst_path.mkdir(parents=True, exist_ok=True) 

        (cat, field, tradeoff, variant) = self.scheme
        extra_cflags  = os.getenv('EXTRA_CFLAGS', '')
        lst = extra_cflags.split(' ')
        new_lst = [item for item in lst if '-DMQOM2_PARAM_' not in item]
        new_lst.append('-DMQOM2_PARAM_SECURITY='+CATEGORIES[cat])
        new_lst.append('-DMQOM2_PARAM_BASE_FIELD='+BASE_FIELDS[field])
        new_lst.append('-DMQOM2_PARAM_TRADEOFF='+TRADE_OFFS[tradeoff])
        new_lst.append('-DMQOM2_PARAM_NBROUNDS='+VARIANTS[variant])
        extra_cflags = ' '.join(new_lst)
        self.compilation_prefix = {
            'EXTRA_CFLAGS': extra_cflags,
        }

    def get_label(self):
        (cat, field, tradeoff, variant) = self.scheme
        return f'{cat}_{field}_{tradeoff}_{variant}'

    def clean(self):
        run_command('make clean', CWD, shell=True)

    def compile_bench(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (bench) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench', folder, shell=True)

    def compile_bench_mem_keygen(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (bench write) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_keygen')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_keygen', folder, shell=True)
        
    def compile_bench_mem_sign(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (bench write) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_sign')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_sign', folder, shell=True)

    def compile_bench_mem_open(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (bench read) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_open')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make bench_mem_open', folder, shell=True)

    def compile_kat_gen(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (kat gen) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make kat_gen')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make kat_gen', folder, shell=True)

    def compile_kat_check(self, folder):
        extra_cflags = self.compilation_prefix['EXTRA_CFLAGS']
        prefix_exec = self.get_label()
        dst_path = self.dst_path
        if arguments.b_compile_only_print:
            print("=== Compilation (kat check) of %s" % prefix_exec)
            print(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make kat_check')
            return "",""
        else:
            return run_command(f'EXTRA_CFLAGS="{extra_cflags}" DESTINATION_PATH="{dst_path}" PREFIX_EXEC="{prefix_exec}" make kat_check', folder, shell=True)

    def run_bench(self, nb_experiments):
        scheme_label = self.get_label()
        dst_path = self.dst_path
        stdout, stderr = run_command(f'{dst_path}/{scheme_label}_bench {nb_experiments}', cwd=CWD)
        assert (not stderr), stderr
        if arguments.b_verbose:
            print(stdout)

        # check that the score is maximal
        data = {
            'path': scheme_label,
            'name':             get_info(stdout, r'\[API\] Algo Name: (.+)'),
            'version':          get_info(stdout, r'\[API\] Algo Version: (.+)'),
            'instruction_sets': get_info(stdout, r'Instruction Sets:\s*(\S.+)?'),
            'compilation'     : ' '.join(f'{key}="{value}"' for key, value in self.compilation_prefix.items()),
            'debug':            get_info(stdout, r'Debug: (.+)'),
            'correctness':      get_info(stdout, r'Correctness: (.+)/{}'.format(nb_experiments)),
            'keygen': get_info(stdout, r' - Key Gen: (.+) ms \(std=(.+)\)'),
            'sign':   get_info(stdout, r' - Sign:    (.+) ms \(std=(.+)\)'),
            'verif':  get_info(stdout, r' - Verify:  (.+) ms \(std=(.+)\)'),
            'pk_size':      get_info(stdout, r' - PK size: (.+) B'),
            'sk_size':      get_info(stdout, r' - SK size: (.+) B'),
            'sig_size_max': get_info(stdout, r' - Signature size \(MAX\): (.+) B'),
            'sig_size':     get_info(stdout, r' - Signature size: (.+) B \(std=(.+)\)'),
            'timestamp' : time.time(),
        }
        try:
            keygen_cycles = get_info(stdout, r' - Key Gen: (.+) cycles')
            sign_cycles = get_info(stdout, r' - Sign:    (.+) cycles')
            verif_cycles = get_info(stdout, r' - Verify:  (.+) cycles')
            data['keygen_cycles'] = keygen_cycles
            data['sign_cycles'] = sign_cycles
            data['verif_cycles'] = verif_cycles
            # Try to extract the detailed elements
            blc_commit_dict = {
                'total' : 'BLC.Commit',
                'expand_trees' : r'\[BLC.Commit\] Expand Trees',
                'seed_commit' : r'\[BLC.Commit\] Seed Commit',
                'prg' : r'\[BLC.Commit\] PRG',
                'xof' : r'\[BLC.Commit\] XOF',
                'arithm' : r'\[BLC.Commit\] Arithm',
            }
            piop_compute_dict = {
                'total' : 'PIOP.Compute',
                'expand_mq' : r'\[PIOP.Compute\] ExpandMQ',
                'expand_batching_mat' : r'\[PIOP.Compute\] Expand Batching Mat',
                'matrix_mult_ext' : r'\[PIOP.Compute\] Matrix Mul Ext',
                'compute_t1' : r'\[PIOP.Compute\] Compute t1',
                'compute_p_zi' : r'\[PIOP.Compute\] Compute P_zi',
                'batch_and_mask' : r'\[PIOP.Compute\] Batch and Mask',
            }
            sample_challenge_dict = {
                'total' : 'Sample Challenge',
            }
            blc_open_dict = {
                'total' : 'BLC.Open',
            }
            for d in blc_commit_dict:
                data['detailed_' + 'blc_commit_' + d] = (float(get_info(stdout, r'.*- %s: (.+) cycles\)' % blc_commit_dict[d]).split("ms")[0]), float(get_info(stdout, r'.*- %s: (.+) cycles\)' % blc_commit_dict[d]).split("(")[1])) 
            for d in piop_compute_dict:
                data['detailed_' + 'piop_compute_' + d] = (float(get_info(stdout, r'.*- %s: (.+) cycles\)' % piop_compute_dict[d]).split("ms")[0]), float(get_info(stdout, r'.*- %s: (.+) cycles\)' % piop_compute_dict[d]).split("(")[1])) 
            for d in sample_challenge_dict:
                data['detailed_' + 'sample_challenge_' + d] = (float(get_info(stdout, r'.*- %s: (.+) cycles\)' % sample_challenge_dict[d]).split("ms")[0]), float(get_info(stdout, r'.*- %s: (.+) cycles\)' % sample_challenge_dict[d]).split("(")[1])) 
            for d in blc_open_dict:
                data['detailed_' + 'blc_open_' + d] = (float(get_info(stdout, r'.*- %s: (.+) cycles\)' % blc_open_dict[d]).split("ms")[0]), float(get_info(stdout, r'.*- %s: (.+) cycles\)' % blc_open_dict[d]).split("(")[1])) 
        except ValueError:
            pass
        if arguments.b_verbose:
            print(data)
        return data
    
    def run_bench_memory(self):
        import re
        scheme_label = self.get_label()
        dst_path = self.dst_path
        data = {}
        for cmd, algo_label in [('keygen', 'Key Generation'), ('sign', 'Signing'), ('open', 'Verification')]:
            _, stderr = run_command(f'valgrind --max-stackframe=10000000 --tool=massif --stacks=yes --log-file=massif.read.log --massif-out-file=massif.read.out {dst_path}/{scheme_label}_bench_mem_{cmd}', cwd=CWD)
            assert (not stderr), stderr
            stats = None
            with open('massif.read.out') as _file:
                stats = _file.readlines()
            current_snapshot = None
            snapshots = []
            reg_title = re.compile(r'snapshot=(\d+)')
            reg_stats = [
                ('time', int, re.compile(r'time=(\d+)')),
                ('mem_heap_B', int, re.compile(r'mem_heap_B=(\d+)')),
                ('mem_heap_extra_B', int, re.compile(r'mem_heap_extra_B=(\d+)')),
                ('mem_stacks_B', int, re.compile(r'mem_stacks_B=(\d+)')),
                ('heap_tree', str, re.compile(r'heap_tree=(.*)')),
            ]
            for line in stats:
                line = line.strip()
                res = reg_title.fullmatch(line)
                if res:
                    if current_snapshot is not None:
                        current_snapshot['total'] = 0
                        if 'mem_heap_B' in current_snapshot:
                            current_snapshot['total'] += current_snapshot['mem_heap_B']
                        if 'mem_heap_extra_B' in current_snapshot:
                            current_snapshot['total'] += current_snapshot['mem_heap_extra_B']
                        if 'mem_stacks_B' in current_snapshot:
                            current_snapshot['total'] += current_snapshot['mem_stacks_B']
                        snapshots.append(current_snapshot)
                    current_snapshot = {'snapshot': int(res.group(1))}
                else:
                    for label, dtype, reg in reg_stats:
                        res = reg.fullmatch(line)
                        if res:
                            current_snapshot[label] = dtype(res.group(1))
            data[cmd] = snapshots
            if arguments.b_verbose:
                peak_memory_snapshot = max(snapshots, key=lambda x: x['total'])
                print(f' - {algo_label}: {peak_memory_snapshot['total']} B')
        if arguments.b_verbose:
            print()
        return data
    
    def run_kat_gen(self):
        scheme_label = self.get_label()
        dst_path = self.dst_path
        return run_command(f'cd {dst_path} && ./{scheme_label}_kat_gen', cwd=CWD, shell=True)

    def run_kat_check(self):
        scheme_label = self.get_label()
        dst_path = self.dst_path
        return run_command(f'cd {dst_path} && ./{scheme_label}_kat_check', cwd=CWD, shell=True)

    def run_valgrind_bench(self):
        scheme_label = self.get_label()
        dst_path = self.dst_path
        _, stderr = run_command(f'valgrind --max-stackframe=10000000 --leak-check=yes {dst_path}/{scheme_label}_bench 1', cwd=CWD)
        summary = [line for line in stderr.split('\n') if 'ERROR SUMMARY' in line][0]
        return summary

    @classmethod
    def get_schemes(cls, schemes_arg, *args, **kwargs):
        schemes = []
        include_all = ('all' in schemes_arg)
        for cat in CATEGORIES:
            include_cat = (f'{cat}' in schemes_arg)
            for field in BASE_FIELDS:
                include_field = (f'{cat}_{field}' in schemes_arg)
                for tradeoff in TRADE_OFFS:
                    include_tradeoff = (f'{cat}_{field}_{tradeoff}' in schemes_arg)
                    for variant in VARIANTS:
                        include_variant = (f'{cat}_{field}_{tradeoff}_{variant}' in schemes_arg)
                        include_scheme = include_all or include_cat or include_field or include_tradeoff or include_variant
                        if include_scheme:
                            schemes.append(cls((cat, field, tradeoff, variant), *args, **kwargs))
        return schemes
    
    @classmethod
    def get_scheme(cls, scheme_arg, *args, **kwargs):
        scheme = None
        for cat in CATEGORIES:
            for field in BASE_FIELDS:
                for tradeoff in TRADE_OFFS:
                    for variant in VARIANTS:
                        scheme_label = f'{cat}_{field}_{tradeoff}_{variant}'
                        if scheme_arg == scheme_label:
                            scheme = cls((cat, field, tradeoff, variant), *args, **kwargs)
        return scheme

def get_info(lines, regex_str):
    import re
    lines = lines if type(lines) is list else lines.split('\n')
    regex = re.compile(regex_str)
    for line in lines:
        res = regex.fullmatch(line)
        if res:
            values = []
            for v in res.groups():
                if v is None:
                    values.append('')
                    continue
                try:
                    # Test if integer
                    values.append(int(v))
                except ValueError:
                    try:
                        # Test if decimal
                        values.append(float(v))
                    except ValueError:
                        # So, it is a string
                        values.append(v)
            if len(values) == 1:
                return values[0]
            else:
                return tuple(values)
    raise ValueError('This regex does not match with any line.')

import pathlib, os
CWD = pathlib.Path(__file__).absolute().parent
BUILD_PATH = CWD.joinpath('build')

if arguments.command == 'compile':
    import contextlib, atexit, tempfile, shutil

    tempdir_names = []

    def cleanup_stuff():
        global tempdir_names
        for t in tempdir_names:
            shutil.rmtree(t, ignore_errors=True)

    def signal_handler(sig, frame):
        print('You pressed Ctrl+C! Exiting')
        cleanup_stuff()
        sys.exit(0)

    def copy_folder(src_path, dst_path, only_root=False):
        for root, dirs, files in os.walk(src_path):
            subpath = root[len(src_path)+1:]
            root_created = False
            for filename in files:
                _, file_extension = os.path.splitext(filename)
                if file_extension in ['.h', '.c', '.inc', '.macros', '.S'] or filename in ['Makefile', '.gitignore']:
                    if not root_created:
                        os.makedirs(os.path.join(dst_path, subpath),  exist_ok = True)
                        root_created = True
                    shutil.copyfile(
                        os.path.join(src_path, subpath, filename),
                        os.path.join(dst_path, subpath, filename)
                    )

    # Register the signal handler
    signal.signal(signal.SIGINT, signal_handler)

    # Register At exist handler
    atexit.register(cleanup_stuff)

    def handle_scheme_compile(scheme):
        print(f'[+] {scheme.get_label()}')
        scheme.clean()
        tname = '/tmp/' + scheme.get_label()
        tempdir_names.append(tname)
        # Copy the source tree to the temporary folder
        copy_folder(str(CWD), tname)
        if not arguments.b_no_bench:
            stdout, stderr = scheme.compile_bench(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
            stdout, stderr = scheme.compile_bench_mem_keygen(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
            stdout, stderr = scheme.compile_bench_mem_sign(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
            stdout, stderr = scheme.compile_bench_mem_open(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
        if not arguments.b_no_kat:
            stdout, stderr = scheme.compile_kat_gen(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
            stdout, stderr = scheme.compile_kat_check(tname)
            if arguments.b_verbose or stderr:
                print(stdout, stderr)
    # Register the signal handler
    signal.signal(signal.SIGINT, signal_handler)
    # Create build folder
    BUILD_PATH.mkdir(parents=True, exist_ok=True) 
    # Compile all the selected schemes
    schemes = MQOMInstance.get_schemes(arguments.schemes, BUILD_PATH)
    if arguments.parallel_jobs != 0:
        from joblib import Parallel, delayed
        results = Parallel(n_jobs=arguments.parallel_jobs, backend="threading")(map(delayed(handle_scheme_compile), schemes))
    else:
        for scheme in schemes:
            handle_scheme_compile(scheme)

elif arguments.command == 'env':
    # Get the selected schemes
    scheme = MQOMInstance.get_scheme(arguments.scheme, BUILD_PATH)
    extra_cflags = scheme.compilation_prefix['EXTRA_CFLAGS']
    print(f'export EXTRA_CFLAGS="{extra_cflags}"')

elif arguments.command == 'clean':
    run_command('make clean', CWD, shell=True)
    import shutil
    shutil.rmtree(BUILD_PATH, ignore_errors=True)

elif arguments.command == 'bench':
    import threading
    STATS_PATH = CWD.joinpath('stats')
    STATS_PATH.mkdir(parents=True, exist_ok=True) 
    # This is a threading lock to handle parallelism for writing into a file
    lock = threading.Lock()

    # Override build folder if asked to
    if arguments.b_bench_build_folder_name is not None:
        BUILD_PATH = CWD.joinpath(arguments.b_bench_build_folder_name)
    if not os.path.isdir(BUILD_PATH):
        print("Error: build path %s does not exist ..." % BUILD_PATH)
        sys.exit(-1)

    # Open json file
    if arguments.b_bench_file_name is not None:
        stats_file_name = arguments.b_bench_file_name
    else:
        stats_file_name = STATS_PATH.joinpath('%s.json' % time.strftime("%Y%m%d_%H%M%S"))
    stats_file = open(stats_file_name, 'w')
    stats_file.write("[")

    def signal_handler(sig, frame):
        print('You pressed Ctrl+C! Exiting')
        # Gracefully close the file
        try:
            stats_file.write("]")
            stats_file.close()
            print('Stats written in stats file %s' % stats_file_name)
        except:
            pass
        sys.exit(0)

    def handle_scheme_bench(scheme):
        print(f'[+] {scheme.get_label()}')
        data = scheme.run_bench(nb_experiments)
        assert data['correctness'] == nb_experiments, (data['correctness'], nb_experiments)
        #assert data['debug'].lower() == 'off', data['debug']
        if bench_memory:
            data_mem = scheme.run_bench_memory()
            data['memory'] = {
                'keygen':  max(data_mem['keygen'], key=lambda x: x['total'])['total'],
                'sign':  max(data_mem['sign'], key=lambda x: x['total'])['total'],
                'verif':  max(data_mem['open'], key=lambda x: x['total'])['total'],
            }
        with lock:
            if len(all_data) != 0:
                stats_file.write(',')
            stats_file.write(json.dumps(data))
            # Flush and sync data in file
            stats_file.flush()
            os.fsync(stats_file)
            all_data.append(data)
            

    # Register the signal handler
    signal.signal(signal.SIGINT, signal_handler)

    nb_experiments = arguments.nb_repetitions
    bench_memory = arguments.b_bench_memory
    print(f'Nb repetitions: {nb_experiments}')
    schemes = MQOMInstance.get_schemes(arguments.schemes, BUILD_PATH)
    all_data = []
    if arguments.parallel_jobs != 0:
        from joblib import Parallel, delayed
        results = Parallel(n_jobs=arguments.parallel_jobs, backend="threading")(map(delayed(handle_scheme_bench), schemes))
    else:
        for scheme in schemes:
            handle_scheme_bench(scheme)

    try:
        stats_file.write("]")
        stats_file.close()
        print('Stats written in stats file %s' % stats_file_name.relative_to(CWD))
    except:
        pass

elif arguments.command == 'test':
    import contextlib, atexit, tempfile, filecmp, shutil

    KAT_Folder = None
    tempdir_obj = None
    tempdir_name = None

    # Override build folder if asked to
    if arguments.b_test_build_folder_name is not None:
        BUILD_PATH = CWD.joinpath(arguments.b_test_build_folder_name)
    if not os.path.isdir(BUILD_PATH):
        print("Error: build path %s does not exist ..." % BUILD_PATH)
        sys.exit(-1)

    def signal_handler(sig, frame):
        print('You pressed Ctrl+C! Exiting')
        sys.exit(0)

    @contextlib.contextmanager
    def cd(newdir, cleanup=lambda: True):
        prevdir = os.getcwd()
        os.chdir(os.path.expanduser(newdir))
        try:
            yield
        finally:
            os.chdir(prevdir)
            cleanup()

    @contextlib.contextmanager
    def tempdir():
        dirpath = tempfile.mkdtemp()
        def cleanup():
            shutil.rmtree(dirpath)
        with cd(dirpath, cleanup):
            yield dirpath

    def cleanup_stuff():
        global tempdir_name
        if tempdir_name is not None:
            shutil.rmtree(tempdir_name)
            tempdir_name = None

    # Register the signal handler
    signal.signal(signal.SIGINT, signal_handler)

    # Register At exit handler
    atexit.register(cleanup_stuff)

    compare_kat_zip = False
    if arguments.compare_kat != None:
        # Is it a zip file or a folder?
        if ".zip" in arguments.compare_kat:
            import zipfile, re
            try:
                zf = zipfile.ZipFile(arguments.compare_kat)
                tempdir_obj = tempdir()
                tempdir_name = str(tempdir_obj)
                zf.extractall(tempdir_name)
            except:
                print("Error: cannot handle provided ZIP package %s" % arguments.compare_kat)
                sys.exit(-1)
            # The uncompressed folder should contain a KAT folder, find it
            subm_pack = None
            for root, dirs, files in os.walk(tempdir_name):
                for d in dirs:
                    subm_pack = re.search(r'submission_package_v2.*', d)
                    print(d)
                    if subm_pack is not None:
                        subm_pack = subm_pack[0]
                        break
                if subm_pack is not None:
                    break
            if subm_pack is None:
                print("Error: cannot find submission package in provided ZIP package %s" % arguments.compare_kat)
                sys.exit(-1)
            KAT_Folder = tempdir_name + '/' + subm_pack + "/KAT"
            print(KAT_Folder)
            if not os.path.isdir(KAT_Folder):
                print("Error: no KAT folder in the uncompressed ZIP package %s" % arguments.compare_kat)
                sys.exit(-1)
            print("[+] KAT folder found in the provided ZIP package %s" % arguments.compare_kat)
            compare_kat_zip = True
        else:
            # See if the folder exists
            if not os.path.isdir(arguments.compare_kat):
                print("Error: KAT folder %s does not exist" % arguments.compare_kat)
                sys.exit(-1)
            KAT_Folder = arguments.compare_kat

    def handle_scheme_test(scheme):
        print(f'[+] {scheme.get_label()}')
        nb_experiments = arguments.nb_repetitions
        data = scheme.run_bench(nb_experiments)
        assert data['correctness'] == nb_experiments, (data['correctness'], nb_experiments)
        if arguments.b_verbose:
            print(data)
        # Generate KAT
        stdout, stderr = scheme.run_kat_gen()
        assert (not stderr), stderr
        if arguments.b_verbose:
            print(stdout)
        file_req = 'PQCsignKAT_{}.req'.format(data['sk_size'])
        file_rsp = 'PQCsignKAT_{}.rsp'.format(data['sk_size'])
        has_file_req = os.path.exists(os.path.join(scheme.dst_path, file_req))
        has_file_rsp = os.path.exists(os.path.join(scheme.dst_path, file_rsp))
        if has_file_req and has_file_rsp:
            print(' - KAT generation: ok (for %s)' % scheme.get_label())
        else:
            print(' - KAT generation: ERROR! (for %s)' % scheme.get_label())
            sys.exit(-1)
        if not arguments.b_no_kat_check:
            # Check KAT
            stdout, stderr = scheme.run_kat_check()
            assert (not stderr), stderr
            if arguments.b_verbose:
                print(stdout)
            assert ('Everything is fine!' in stdout), stdout
            print(' - KAT check: ok (for %s)' % scheme.get_label())
        # Compare the KAT with an existing reference one?
        if KAT_Folder is not None:
            # Check that we indeed have the KAT for the tested scheme
            if compare_kat_zip:
                scheme_name = 'mqom2_%s' % scheme.get_label()
            else:
                scheme_name = '%s' % scheme.get_label()
                CHECK_KAT_file_name_scheme = KAT_Folder + "/" + scheme_name + "/" + "PQCsignKAT_" + str(data['sk_size']) + ".rsp"
                if not os.path.exists(CHECK_KAT_file_name_scheme):
                    scheme_name = 'mqom2_%s' % scheme.get_label()
            # Check that our KAT files exist
            KAT_file_name_scheme = KAT_Folder + "/" + scheme_name + "/" + "PQCsignKAT_" + str(data['sk_size']) + ".rsp"
            KAT_file_name_scheme_generated = str(scheme.dst_path) + "/" + "PQCsignKAT_" + str(data['sk_size']) + ".rsp"
            if not os.path.exists(KAT_file_name_scheme):
                print("Error: KAT file %s not found ..." % KAT_file_name_scheme)
                sys.exit(-1)
            # Now make a bin diff between the two files
            if filecmp.cmp(KAT_file_name_scheme, KAT_file_name_scheme_generated) is True:
                print(' - KAT check with reference KAT: ok (for %s)' % scheme.get_label())
            else:
                print(' - KAT check with reference KAT: ERROR! (for %s)' % KAT_file_name_scheme_generated)
                sys.exit(-1)
        if not arguments.b_no_valgrind:
            summary = scheme.run_valgrind_bench()
            print(f' - Valgrind: "{summary}" (for %s)' % scheme.get_label())

    schemes = MQOMInstance.get_schemes(arguments.schemes, BUILD_PATH)
    if arguments.parallel_jobs != 0:
        from joblib import Parallel, delayed
        results = Parallel(n_jobs=arguments.parallel_jobs, backend="threading")(map(delayed(handle_scheme_test), schemes))
    else:
        for scheme in schemes:
            handle_scheme_test(scheme)
