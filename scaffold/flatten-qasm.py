import argparse
import re

def process_qasm(fname):

    qgates = ['H','X','CNOT','Y','Z','S','T','Tdag','Sdag','Rz','PrepX','PrepZ','MeasX','MeasZ','Toffoli','Fredkin']    

    qgates_1 = ['H','X','Y','Z','S','T','Tdag']
    qgates_1a = ['Sdag']
    qgates_2 = ['CNOT']
    qgates_3 = ['Toffoli','Fredkin']    
    qgates_4 = ['PrepX','PrepZ']
    qgates_5 = ['MeasX','MeasZ']
    qgates_6 = ['Rz']
    

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
        'Fredkin':'Fredkin'
        }
    
    pattern_qbit_decl = re.compile(r"\s*\bqbit\b\s+(?P<qbit_var>\w+)\s*\[\s*(?P<array_size>\d+)\s*\]\s*;")
    pattern_cbit_decl = re.compile(r"\s*\bcbit\b\s+(?P<qbit_var>\w+)\s*\[\s*(?P<array_size>\d+)\s*\]\s*;")

    pattern_qg = re.compile(r"\s*((\w+|\w+\[(.*?)\])\s*\=)*\s*(?P<func_name>\w+)\s*\(\s*(?P<array_size>(.*?))\s*\)\s*;")
    pattern_qbit_arg = re.compile(r"(.*?)\((.*?)\bqbit\b\s*(.*?)\)(.*?)")
    pattern_meas = re.compile(r"\s*(?P<func_ret>(\w+|\w+\[(.*?)\])\s*\=)*\s*(\bqg_MeasX|qg_MeasZ\b)\s*\(\s*(?P<array_size>(.*?))\s*\)\s*;")
    pattern_main = re.compile(r"\s*(\bvoid|module\b)\s+(\bmain\b)\s*\((.*?)\)\s*(\{)*\s*")
    pattern_comment = re.compile(r"\s*//--//--(.*?)--//--//\s*")

    fout_name = re.sub('\.qasmh$','_qasm.scaffold',fname)
    fout = open(fout_name,'w')

    fout.write('#include<stdio.h>\n')

    #add instrumentation functions
    for q in qgates_1:
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a){ printf("' +gateNames[q] +' %s\\n",a); }\n'
        fout.write(fstr)

    fout.write('\n')

    for q in qgates_1a: #Sdag = S^3
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a){ printf("S %s\\n",a); printf("S %s\\n",a); printf("S %s\\n",a); }\n'
        fout.write(fstr)

    fout.write('\n')


    for q in qgates_2: #CNOT => CX (target,control)
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a, char* b){ printf("'+gateNames[q]+' %s,%s\\n",a,b); }\n'
        fout.write(fstr)

    fout.write('\n')

    for q in qgates_3:
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a, char* b, char* c){ printf("' +gateNames[q] +' %s,%s,%s\\n",a,b,c); }\n'
        fout.write(fstr)

    fout.write('\n')

    for q in qgates_4: #PrepZ, PrepX
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a, int i){ printf("' +gateNames[q] +' %s\\n",a); '
        fout.write(fstr)
        fstr = 'if(i==1){ printf("X %s\\n",a); } }\n'
        fout.write(fstr)

    fout.write('\n')

    for q in qgates_5: #MeasX, MeasZ
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a){ printf("' +gateNames[q] +' %s\\n",a); }\n'
        fout.write(fstr)

    fout.write('\n')

    for q in qgates_6:
        instFnName = 'qg_'+q
        fstr = 'void '+instFnName+'(char* a, double b){ printf("' +gateNames[q] +' %s,%f\\n",a,b); }\n'
        fout.write(fstr)

    fout.write('\n')


    #ignore contents until QASM Generation Pass                                                                         
    f = open(fname,'r')
    b = 'Dummy Line'
    while(b!=''):
        if(b.find('QASM Generation Pass:')!=-1):
            break
        b = f.readline()

    b = f.readline()

    inMainFunc = False
    setQbitDecl = []
    setCbitDecl = []


    while(b!=''):
        if(b.find('End of QASM generation')!=-1):
            break

        #check for qbit declarations                
        m = re.match(pattern_main,b)
        if(m):
            inMainFunc = True
            b = re.sub(r"\bvoid|module\b","int ",b)

        m = re.match(pattern_qbit_decl,b)

        if(m): #Matched qbit declaration
            numElem = int(m.group('array_size'))
            var = m.group('qbit_var')

            addAlphabet=''
            if(not inMainFunc):
                addAlphabet='a' #add 'a' at end of ancilla declaration
    
            subStr = "char* "+m.group('qbit_var')+'['+m.group('array_size')+'] = {'
            fout.write(subStr)
                        
            for i in range(numElem-1):                
                varName = var+str(i)+addAlphabet
                tmp = '"'+varName+'",'
                if varName not in setQbitDecl:
                    setQbitDecl.append(varName)
                fout.write(tmp)
                
            varName = var+str(numElem-1)+addAlphabet
            tmp = '"'+varName+'"'            
            if varName not in setQbitDecl:
                setQbitDecl.append(varName)
            fout.write(tmp)
            fout.write('};\n')                          

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
                    mystr = re.sub(r"\bqbit\b","char* ",mystr)
                    fout.write(mystr)

                else:
                    m = re.match(pattern_cbit_decl,b)
                    if(m):
                        numElem = int(m.group('array_size'))
                        var = m.group('qbit_var')


                        subStr = "char* "+m.group('qbit_var')+'['+m.group('array_size')+'] = {'
                        fout.write(subStr)
                        
                        for i in range(numElem-1):
                            tmp = '"'+var+str(i)+'",'
                            setCbitDecl.append(var+str(i))
                            fout.write(tmp)

                        tmp = '"'+var+str(numElem-1)+'"'
                        setCbitDecl.append(var+str(numElem-1))
                        fout.write(tmp)
                        fout.write('};\n')
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

    #write qbit and cbit declarations to file
    fdecl = open("fdecl.out",'w')
    for q in setQbitDecl:
        myStr = 'qubit '+q+'\n'
        fdecl.write(myStr)
    for q in setCbitDecl:
        myStr = 'cbit '+q+'\n'
        fdecl.write(myStr)
    fdecl.close()
        

parser = argparse.ArgumentParser(description='Convert QASM code into flattened QASM code')
parser.add_argument("input")
args = parser.parse_args()

process_qasm(args.input)
