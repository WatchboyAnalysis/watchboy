import os, sys
import full_simplify

def main():
    # Run as follows: batch.py [list of files] or batch.py *

    for i in range( 1, len(sys.argv) ):
        input_name = sys.argv[i]
        # only process root files
        if input_name.endswith('.root'):
            output_name = input_name.replace('.root','_simplified.root')
            full_simplify.main( input_name, output_name )

    # Clean up this directory a bit
    os.system('rm -rf Auto* __pycache__ good_blocks_of_data.temp');

if __name__ == '__main__':
    main()
