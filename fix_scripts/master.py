# This script will run the full progression of simplifying scripts
# The order is:
# full_simplify.py -> gate_builder -> gate_builder_organize -> charge_balance
# simpfix and gate_builder_addon DO NOT need to run anymore

import os, sys, time
import full_simplify
    

def main():
    # Run as: master.py [list of files]

    runtimes = []
    
    # First, make sure ALL of the programs are compiled
    programs = ['./gate_builder', './gate_builder_organize', './charge_balance', './organize']
    if False in [os.path.isfile(x) for x in programs]:
        print('Cannot locate all files')
        return None

    for fname in sys.argv:
        if fname.endswith('.root'):
            output_name = fname.replace('.root', '_simplified.root')
            ti=time.time()
            full_simplify.main( fname, output_name )
            runtimes.append(time.time()-ti)
            for exe in programs:
                print('running', exe, 'on', output_name)
                ti=time.time()
                os.system(exe+' '+output_name + ' 2>/dev/null')
                runtimes.append(time.time()-ti)
    print('Run times:', runtimes)

if __name__ == '__main__':
    main()
