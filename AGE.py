import getopt
import os
import sys
import threading
import json
from json import encoder
import numpy
import datetime
import sys

sys.setrecursionlimit(100)

project_root = './cpp_proj/AGEProj'
executable_path = './cpp_proj/AGEProj/Release/'
devenv_path = r'"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/"'
supported_functions_age0 = ['rastrigin', 'schwefel', 'rosenbrock', 'sphere']
supported_functions_age1 = ['rastrigin', 'schwefel', 'rosenbrock', 'sphere']
supported_functions_age2 = ['rastrigin', 'schwefel', 'rosenbrock', 'sphere']


def generate_output_func(an_type: str, name: str):
    os.environ["Path"] = os.path.abspath(executable_path)
    start = datetime.datetime.now()
    print('=== run on function ' + name + ' started ===' + ' at ' + str(start))
    os.system('call AGEProj ' + an_type + ' ' + name)
    end = datetime.datetime.now()
    print('=== run on function ' + name + ' ended ===' + ' at ' + str(start))
    print('total time on ' + name + ' is ' + str(end - start))


def main(argv):
    print('================== A.G.E. script ==================')
    print('### 2019 - Petrovici Stefan ###\n')

    if len(sys.argv) == 1:
        print('HELP: pass -b for building the executable from the sln')
        print('      pass -r for running the analysis on the available functions + age0/age1 type analyzer')
        print('      pass -a to generate statistics + age0/age1 type analyzer')
        print('      => Analysers : age0 - generates output for two algorithms:')
        print('      =>                    a deterministic binary search algorithm')
        print('      =>                    a heuristic algorithm')
        print('      =>             age1 - generates output for three algorithms:')
        print('      =>                    hill_climbing best improve')
        print('      =>                    hill_climbing best improve')
        print('      =>                    simulated_annealing')

    opts, args = getopt.getopt(sys.argv[1:], "br:a:", ["analyze=", "run="])

    for opt, arg in opts:

        # build
        if opt in ('-b', ''):
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
        if opt in ('-r', '--run'):
            threads = []
            if arg == 'age0':
                for func in supported_functions_age0:
                    t = threading.Thread(target=generate_output_func, args=(arg, func,))
                    t.start()
                    threads.append(t)
            if arg == 'age1':
                for func in supported_functions_age1:
                    t = threading.Thread(target=generate_output_func, args=(arg, func,))
                    t.start()
                    threads.append(t)

            if arg == 'age2':
                for func in supported_functions_age2:
                    t = threading.Thread(target=generate_output_func, args=(arg, func,))
                    t.start()
                    threads.append(t)

            for t in threads:
                t.join()

        # generate statistics
        if opt in ('-a', '--analyze'):
            if arg == 'age0':
                final_table = []
                for func in supported_functions_age0:
                    with open('output_age0_' + func + '.json', 'r') as f:
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
                                'run_time': analysis['deterministic_results']['runtime'] / 1000000
                                # converting to seconds
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
                                'median': numpy.median(results),
                                'mean_run_time': numpy.mean(analysis['heuristic_results']['deltas']) / 1000000,
                                # converting to seconds
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
                    for cand, val in line.items():
                        if type(val) == float or type(val) == numpy.float64:
                            prettified_strings.append('{:.5f}'.format(val))
                        else:
                            prettified_strings.append(str(val))
                    print(' & '.join([val for val in prettified_strings]), end=' \\\\\n')

            if arg == 'age1':
                final_table = []
                for func in supported_functions_age0:
                    with open('output_age1_' + func + '.json', 'r') as f:
                        encoder.FLOAT_REPR = lambda o: format(o, '.5f')
                        numpy.set_printoptions(formatter={"float_kind": lambda x: "%g" % x})
                        numpy.set_printoptions(precision=6)
                        data = json.load(f)

                        for analysis in data['analysis']:
                            results = [float(n) for n in analysis['results']]
                            run_time = sum(analysis['deltas']) / 1000000
                            h_table_line = {
                                'algorithm': analysis['algorythm-name'],
                                'function_name': func,
                                'dimension': analysis['dimension'],
                                'mean': numpy.mean(results),
                                'max': numpy.max(results),
                                'min': numpy.min(results),
                                'median': numpy.median(results),
                                'mean_run_time': numpy.mean(analysis['deltas']) / 1000000,  # converting to seconds
                                'run_time': run_time / 60,
                                'sd': numpy.std(results)
                            }
                            final_table.append(h_table_line)

                with open('final_result.json', 'w') as final:
                    json.dump(final_table, final)

                printed_head = False
                for line in final_table:
                    if not printed_head:
                        print(' & '.join([key.capitalize() for key, val in line.items()]), end=' \\\\\n')
                        printed_head = True
                        print()

                    prettified_strings = []
                    for cand, val in line.items():
                        if type(val) == float or type(val) == numpy.float64:
                            prettified_strings.append('{:.5f}'.format(val))
                        else:
                            prettified_strings.append(str(val))
                    print(' & '.join([val for val in prettified_strings]), end=' \\\\\n')

            if arg == 'age1p':
                with open('output_trace_hc.json', 'r') as f:
                    data = json.load(f)

                    res_dict = dict()

                    def add_to_dict(key):
                        tr = dict()
                        for neigh in candidates['improvements']:
                            if key in [''.join(coord_s) for coord_s in candidates['improvements'][neigh]['next_neighbors_coordinates_string']]:
                                new_key = ''.join(neigh)
                                tr[new_key] = dict()
                                tr[new_key]["children"] = add_to_dict(new_key)
                                tr[new_key]["value"] = candidates['improvements'][neigh]['value']
                        return tr

                    for func, candidates in data.items():

                        candidate_tree = dict()
                        for cand, value in candidates['improvements'].items():
                            if not candidates['improvements'][cand]['next_neighbors_coordinates_string']:
                                candidate_tree[cand] = dict()
                                candidate_tree[cand]["children"] = add_to_dict(cand)
                                candidate_tree[cand]["value"] = candidates['improvements'][cand]['value']

                        res_dict[func] = candidate_tree

                        print('Json loaded.')

                    with open('candidate_tree.json', 'w') as j:
                        json.dump(res_dict, j)

            if arg == 'age2':
                final_table = []
                for func in supported_functions_age0:
                    with open('output_age2_' + func + '.json', 'r') as f:
                        encoder.FLOAT_REPR = lambda o: format(o, '.5f')
                        numpy.set_printoptions(formatter={"float_kind": lambda x: "%g" % x})
                        numpy.set_printoptions(precision=6)
                        data = json.load(f)

                        for analysis in data['analysis']:
                            if 'results' not in analysis:
                                continue
                            results = [float(n) for n in analysis['results']]
                            run_time = sum(analysis['deltas']) / 1000000
                            h_table_line = {
                                'algorithm': analysis['algorythm-name'],
                                'function_name': func,
                                'dimension': analysis['dimension'],
                                'mean': numpy.mean(results),
                                'max': numpy.max(results),
                                'min': numpy.min(results),
                                'median': numpy.median(results),
                                'mean_run_time': numpy.mean(analysis['deltas']) / 1000000,  # converting to seconds
                                'run_time': run_time / 60,
                                'sd': numpy.std(results)
                            }
                            final_table.append(h_table_line)

                with open('final_result.json', 'w') as final:
                    json.dump(final_table, final)

                printed_head = False
                for line in final_table:
                    if not printed_head:
                        print(' & '.join([key.capitalize() for key, val in line.items()]), end=' \\\\\n')
                        printed_head = True
                        print()

                    prettified_strings = []
                    for cand, val in line.items():
                        if type(val) == float or type(val) == numpy.float64:
                            prettified_strings.append('{:.5f}'.format(val))
                        else:
                            prettified_strings.append(str(val))
                    print(' & '.join([val for val in prettified_strings]), end=' \\\\\n')


if __name__ == "__main__":
    main(sys.argv[1:])
