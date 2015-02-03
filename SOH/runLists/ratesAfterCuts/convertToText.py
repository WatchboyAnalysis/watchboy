import re

def convert_data(run_name):
    in_name=run_name+'_cutRate.C'
    out_name=run_name+'_cutRate.txt'
    inFile = open(in_name, 'r')
    outFile=open(out_name,'w')
    for line in inFile:
        match = re.search('SetBinContent\((.*)\)',line)
        if match!= None:
            phrase = match.group(1)
            new_phrase=phrase.replace(',',' ')
            new_phrase=new_phrase+'\n'
            outFile.write(new_phrase)

run_list=["y13m09","y13m11","y13m12","y14m01","y14m02","y14m03","y14m04","y14m05","y14m06"]
k=0
while k<len(run_list):
    convert_data(run_list[k])
    k+=1
