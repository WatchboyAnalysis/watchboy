# This script runs through event files looking for corrupt regions
# Once found it leaves a note about the corrupt block, then runs
# simplify.C in an interactive root session on the good blocks
#
# I hate root ...
import sys, os
import math

def main(input_name, output_name):
    # Should be run as: python full_simplify.py input_file output_file
    # run ./corrupt_worker file_name event >& /dev/null > temp_file.txt
    temp_name = input_name + '.temp'
    good_files = [[],[],[]]
    trees_to_explore = ['fastTree', 'orphanTree1', 'orphanTree2']
    print( "Looking for bad data blocks" )

    for tree, index in zip(trees_to_explore, range(3)):
        event_start=0
        # DEBUGGING ::
        # if(tree=='fastTree'):
        #     event_start = 3500000
        # else:
        #     event_start = math.floor(3500000/3)
        # END DEBUG ::
        keep_running = 1
        while keep_running != -1:
            command = './corrupt_worker ' + tree + ' ' + str(input_name) + ' ' + str(event_start) + ' > ' + temp_name #&> log.txt > ' + temp_name
            os.system(command)
            temp_file = open(temp_name, 'r')
            event_end = int((temp_file.readline()).split()[0])
            print("found garbage at event: ", event_end, " in tree ", tree)
            keep_running = int((temp_file.readline()).split()[0])
            if event_start != event_end:
                for i in range(event_start, event_end):
                    good_files[index].append(i)
            event_start = event_end + 1000
        # Done with the first temp file, killing
        os.system('rm ' + temp_name)
        
        # Filling the next temp file with the good blocks
        temp_name_2 = input_name + '.' + tree + '.temp'
        print( 'Writing good blocks to: ' , temp_name_2)
        temp_file = open(temp_name_2, 'w')
        temp_file.write( str(input_name) + '\n' + str(output_name) + '\n' )
        for i in good_files[index]:
            temp_file.write( str(i) + '\n' )
        temp_file.close()

    print( "Simplifying the code format" )
    os.system('root -b -q -l \"simplify.C(\\\"' + str(input_name) + '\\\")\" ')#2> log_simp.txt')
    print( "Finished writing to: ", str(output_name) )

    for tree in trees_to_explore:
        os.system('rm '+input_name+'.'+tree+'.temp')
    #os.system('rm good_block*')

if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
