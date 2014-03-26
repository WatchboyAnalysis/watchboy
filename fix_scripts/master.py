# This script will run the full progression of simplifying scripts
# The order is:
# full_simplify.py -> gate_builder -> gate_builder_organize -> charge_balance
# simpfix and gate_builder_addon DO NOT need to run anymore

import os, sys
import full_simplify
    

def main():
    # Run as: master.py [list of files]
    
    # First, make sure ALL of the programs are compiled
    programs = ['./gate_builder', './gate_builder_organize', './charge_balance']
    if False in [os.path.isfile(x) for x in programs]:
        print('Cannot locate all files')
        return None

    for fname in sys.argv:
        if fname.endswith('.root'):
            output_name = fname.replace('.root', '_simplified.root')
            full_simplify.main( fname, output_name )
            for exe in programs:
                os.system(programs+' '+output_name)

    return

if __name__ == '__main__':
    main()
