import sys, json
from subprocess import Popen, PIPE
import subprocess as s

#Read data from stdin
def read_in():
    lines = sys.stdin.readlines()
    
    return lines[0]

def main():
    #get our data as an array from read_in()
    filename = read_in()

    p = Popen(['../cpp/imagesim/Debug/imagesim', filename]) #primer parametro es el ejecutable del programa en c, segundo parametro es el nombre del archivo
    p.wait()
    p = Popen(['../cpp/kmeans/Debug/kmeans', filename]) #primer parametro es el ejecutable del programa en c, segundo parametro es el nombre del archivo
    p.wait()

# Start process
if __name__ == '__main__':
    main()
