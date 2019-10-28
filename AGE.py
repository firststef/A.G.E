import getopt
import os
import sys
import threading
import json
from json import encoder
import numpy

project_root = './cpp_proj/AGEProj'
executable_path = './cpp_proj/AGEProj/Release/'
devenv_path = r'"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/"'
supported_functions_age0 = ['rastrigin', 'schwefel', 'rosenbrock', 'sphere']


def generate_output_func(name: str):
    os.environ["Path"] = os.path.abspath(executable_path)
    print('=== run on function ' + name + ' started ===')
    os.system('call AGEProj age0 ' + name)
    print('=== run on function ' + name + ' ended ===')


def main(argv):
    print('================== A.G.E. script ==================')
    print('### 2019 - Petrovici Stefan ###\n')

    if len(sys.argv) == 1:
        print('HELP: pass -b for building the executable from the sln\n')
        print('      pass -r for running the analysis on the available functions\n')
        print('      pass -a to generate statistics\n')

    opts, args = getopt.getopt(sys.argv[1:], "bra", [])

    # build
    if ('-b', '') in opts:
        # move to project to build
        os.environ["Path"] = devenv_path
        os.chdir(project_root)

        try:
            print('=== build started ===')
            proc = os.system('call devenv AGEProj.sln /build Release')
            print('=== build finished ===')
        except:
            print('AGE requires Microsoft Visual Studio 2017 Community to build the project')
            exit(-1)

        # move back
        os.chdir("..")
        os.chdir("..")

    # run code
    if ('-r', '') in opts:
        threads = []
        for func in supported_functions_age0:
            t = threading.Thread(target=generate_output_func, args=(func,))
            t.start()
            threads.append(t)

        for t in threads:
            t.join()

    # generate statistics
    final_table = []
    if ('-a', '') in opts:
        for func in supported_functions_age0:
            with open('output_' + func + '.json', 'r') as f:
                encoder.FLOAT_REPR = lambda o: format(o, '.5f')
                numpy.set_printoptions(formatter={"float_kind": lambda x: "%g" % x})
                numpy.set_printoptions(precision=6)
                data = json.load(f)

                for analysis in data['analysis']:
                    # deterministic
                    d_table_line = {
                        'name': data['function-name'],
                        'algorithm_type': 'deterministic',
                        'dimension': analysis['dimension'],
                        'result': analysis['deterministic_results']['result'],
                        'run_time': analysis['deterministic_results']['runtime'] / 1000000 # converting to seconds
                    }
                    final_table.append(d_table_line)

                    # heuristic
                    results = [float(n) for n in analysis['heuristic_results']['results']]
                    h_table_line = {
                        'name': data['function-name'],
                        'algorithm_type': 'heuristic',
                        'dimension': analysis['dimension'],
                        'mean': numpy.mean(results),
                        'max': numpy.max(results),
                        'min': numpy.min(results),
                        'median' : numpy.median(results),
                        'mean_run_time': numpy.mean(analysis['heuristic_results']['deltas']) / 1000000, # converting to seconds
                        'sd': numpy.std(results)
                    }
                    final_table.append(h_table_line)

        with open('final_result.json', 'w') as final:
            json.dump(final_table, final)

        def sort_type(val):
            if val['algorithm_type'] == 'deterministic':
                return 0
            return 1
        final_table = sorted(final_table, key=sort_type)

        printed_head = False
        for line in final_table:
            if not printed_head:
                print(' & '.join([key.capitalize() for key, val in line.items()]), end=' \\\\\n')
                printed_head = True
                print()

            prettified_strings = []
            for key, val in line.items():
                if type(val) == float or type(val) == numpy.float64:
                    prettified_strings.append('{:.5f}'.format(val))
                else:
                    prettified_strings.append(str(val))
            print(' & '.join([val for val in prettified_strings]), end=' \\\\\n')


if __name__ == "__main__":
    main(sys.argv[1:])
