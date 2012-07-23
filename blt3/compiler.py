import parser
import sys

def main():
    fname = sys.argv[1]
    data = open(fname).read()
    tree = parser.parse(data)
    print(tree)

if __name__ == '__main__':
    if len(sys.argv) >= 2:
        main()
    else:
        print('usage: %s filename.blt3'%sys.argv[0])
