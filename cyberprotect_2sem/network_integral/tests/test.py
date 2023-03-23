import time
import subprocess
from colorama import Fore, Style

server_executable = 'server'
client_executable = 'client'

def run_client(nthreads):
    exec_string = './' + client_executable
    return subprocess.Popen(args=[exec_string, str(nthreads)], stdout=subprocess.DEVNULL)

def run_server(ncomps):
    exec_string = './' + server_executable
    return subprocess.Popen(args=[exec_string, str(ncomps)], stdout=subprocess.DEVNULL)

def FreezeTest():
    
    return True

def CrashTest():
    return True

def SimpleTest():
    for i in range(1, 4):
        print('Clients: 2, Threads per computer:', i)
        c1 = run_client(i)
        c2 = run_client(i)
        
        s = run_server(2)
        
        c1.wait()
        c2.wait()
        s.wait()
        
        ret_code = s.poll()
        if ret_code != 0:
            return False
    
    return True

tests_list = [
    SimpleTest,
    CrashTest,
    FreezeTest,
]

def main():
    print('Running all tests...')
    for test in tests_list:
        start = time.time()
        result = test()
        end = time.time()
        
        print(test.__name__ + '...', end='')
        if result is True:
            print(Fore.GREEN + ' [PASSED]', end='')
        else:
            print(Fore.RED + ' [FAILED]', end='')
        print(Style.RESET_ALL)
        
        print('Time: ' + str(round(end - start, 2)) + ' seconds')

if __name__ == '__main__':
    main()