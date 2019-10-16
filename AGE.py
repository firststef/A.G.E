import getopt
import os
import sys
import threading

project_root = './cpp_proj/AGEProj'
executable_path = './cpp_proj/AGEProj/Release/'
devenv_path = r'"C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/Common7/IDE/"'
supported_functions_age1 = ['rastrigin', 'schwefel', 'rosenbrock', 'sphere']


def generate_output_func(name: str):
    os.environ["Path"] = os.path.abspath(executable_path)
    print('=== run on function ' + name + ' started ===')
    os.system('call AGEProj age1 ' + name)
    print('=== run on function ' + name + ' ended ===')


def main(argv):
    print('================== A.G.E. script ==================')
    print('### 2019 - Petrovici Stefan ###\n')

    opts, args = getopt.getopt(sys.argv[1:], "br", [])

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
        for func in supported_functions_age1:
            t = threading.Thread(target=generate_output_func, args=(func,))
            t.start()
            threads.append(t)

        for t in threads:
            t.join()


if __name__ == "__main__":
    main(sys.argv[1:])