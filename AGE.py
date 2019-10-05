import subprocess

root = 'D:/facultate/anul_2/A.G.E/'
compiler_path = 'D:/facultate/anul_2/A.G.E/mingw/bin/'

proc = subprocess.Popen(['g++', root + "age1.cpp", "-o", root + "age1.exe"], cwd=compiler_path, stdout=open('age1.txt', 'w'), shell=True)
proc.wait()
