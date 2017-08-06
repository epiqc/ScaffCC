import argparse
import re
from subprocess import call

def process_qasm(fname):

    qgates = ['H','X','CNOT','Y','Z','S','T','Tdag','Sdag','Rz','PrepX','PrepZ','MeasX','MeasZ','Toffoli','Fredkin']    

    qgates_1 = ['H','Y','Z','S','T','Tdag']
    qgates_1a = ['Sdag']
    qgates_2 = ['CNOT']
    qgates_3 = ['Fredkin']    
    qgates_4 = ['PrepX','PrepZ']
    qgates_5 = ['MeasX','MeasZ']
    qgates_6 = ['Rz']
    qgates_7 = ['afree']

    gateNames = {
        'H':'H',
        'X':'X',
        'Y':'Y',
        'Z':'Z',
        'S':'S',
        'T':'T',
        'Sdag':'Sdag',
        'Tdag':'Tdag',
        'PrepX':'PrepX', #'Px',
        'PrepZ':'PrepZ', #'Pz',
        'MeasZ':'MeasZ', #'Mz',
        'MeasX':'MeasX', #'Mx',
        'Rz':'Rz',
        'CNOT':'CNOT', #'CX',
        'Toffoli':'Tof',
        'Fredkin':'Fredkin',
        'afree':'afree'
        }
    
    pattern_qbit_decl = re.compile(r"\{*\s*\{*\bqbit\b\s+(?P<qbit_var>\w+)\s*\[*\s*(?P<array_size>\w*\+*\w*)\s*\]*[\s,]*\}*\s*\}*")
    pattern_cbit_decl = re.compile(r"\{*\s*\{*\bcbit\b\s+(?P<qbit_var>\w+)\s*\[*\s*(?P<array_size>\w*\+*\w*)\s*\]*[\s,]*\}*\s*\}*")
    pattern_zero2garbage_decl = re.compile(r"\s*zero_to_garbage\s+(?P<z_var>\w+)\s*\[*\s*(?P<array_size>\w*\+*\w*)\s*\]*\s*;")
    pattern_qg = re.compile(r"\{*\s*\{*((\w+|\w+\[(.*?)\])\s*\=)*\s*(?P<func_name>\w+)\s*\(\s*(?P<array_size>(.*?))\s*\)\}*\s*\}*;")
    pattern_qbit_arg = re.compile(r"(.*?)\((.*?)\bqbit\b\s*(.*?)\)(.*?)")
    pattern_meas = re.compile(r"\s*(?P<func_ret>(\w+|\w+\[(.*?)\])\s*\=)*\s*(\bqg_MeasX|qg_MeasZ\b)\s*\(\s*(?P<array_size>(.*?))\s*\)\s*;")
    pattern_main = re.compile(r"\s*(\bint|void|module\b)\s+(\bmain|main1\b)\s*\((.*?)\)\s*(\{)*\s*")
    pattern_end_main = re.compile(r"\s*(\breturn\b)\s+(\b\w+\b)\s*;\s*")
    pattern_module = re.compile(r"\s*module\b\s*(.*?)\)(.*?)")
    pattern_rkqc = re.compile(r"\s*rkqc\b\s*(.*?)\)(.*?)")
    pattern_comment = re.compile(r"\s*//--//--(.*?)--//--//\s*")

    fout_name = re.sub('\.scaffold$','_scaffold.c',fname)
    fout = open(fout_name,'w')
    
    f_prebuilt = open("prebuilt.c", 'r')
    line = f_prebuilt.readline()
    while(line != ''):
        fout.write(line)
        line = f_prebuilt.readline()
    f_prebuilt.close()

    f = open(fname,'r')
    b = f.readline()

    inMainFunc = False
    setQbitDecl = []
    setCbitDecl = []
    setZvarDecl = []

    while(b!=''):
        if(b.find('End of QASM generation')!=-1):
            break
        #check for qbit declarations
        m = re.match(pattern_rkqc, b)
        if (m):
            b = re.sub(r"\brkqc\b", "void ",b)
            b = re.sub(r"\bqint\b", "char ", b)

        m = re.match(pattern_main, b)
        if(m):
            inMainFunc = True
            b = re.sub(r"\bvoid|module\b","int ",b)

        m = re.match(pattern_end_main, b)
        if (m):
            if (inMainFunc == True):
                for name, size in setZvarDecl:
                    mystr = "var_display(\"" + name + "\"," + name + "," + size + ");\n"
                    fout.write(mystr)

        m = re.match(pattern_module,b)
        if(m):
            b = re.sub(r"\bmodule\b","void ",b)

        m = re.match(pattern_zero2garbage_decl, b)
        if (m):
            b = re.sub(r"zero_to_garbage", "char", b)
            b = re.sub(r";", " = {0};", b)
            if (inMainFunc == True):
                varName = m.group('z_var')
                varSize = m.group('array_size')
                setZvarDecl.append((varName, varSize))

        m = re.match(pattern_qbit_decl,b)
        if(m): #Matched qbit declaration
            numElem = m.group('array_size')
            var = m.group('qbit_var')
            mystr = b
            mystr = re.sub(r"qbit ","char ",mystr)
            fout.write(mystr)
        else:
            m = re.match(pattern_qg,b)
            if(m):    #Matched qauntum gate call                            
                qstr = m.group('func_name')
                
                if qstr in qgates:
                    rstr = 'qg_'+qstr
                
                    mystr = b.replace(qstr,rstr)

                    #check for Meas gates
                    m1 = re.match(pattern_meas,mystr)
                    if(m1):
                        retStr = m1.group('func_ret')
                        
                        if(retStr):
                            mystr = mystr.replace(retStr,'')

                    fout.write(mystr)                            

                else:
                    fout.write(b)
                
            else:                                                
                #substitute qbit as char* in module definitions                
                m = re.match(pattern_qbit_arg,b)
                                
                if(m):
                    mystr = b
                    mystr = re.sub(r"\bqbit\b","char ",mystr)
                    fout.write(mystr)

                else:
                    m = re.match(pattern_cbit_decl,b)
                    if(m):
                        numElem = m.group('array_size')
                        var = m.group('qbit_var')

                        mystr = b
                        mystr = re.sub(r"\bcbit\b","char ",mystr)
                        fout.write(mystr)
                        
                        
                    else:
                        m = re.match(pattern_comment,b)
                        if(m):
                            subStr = 'printf("'+b.rstrip('\n')+'\\n");'
                            fout.write(subStr)

                        else:
                            #print 'Did not match any pattern:',b
                            fout.write(b)
        b = f.readline()

    f.close()
    fout.close()
    call(["g++", fout_name, "-o", "rkqc_executable"])
    call(["./rkqc_executable", ""])  

parser = argparse.ArgumentParser(description='Convert scaffold code into c code')
parser.add_argument("input")
args = parser.parse_args()

process_qasm(args.input)

