from qiskit import *
import os
import glob
import sys
import json
from qiskit.transpiler import PassManager, transpile
from qiskit.transpiler.passes import ResourceEstimation

# Add config directory to filepath
test_path = os.path.abspath(sys.argv[0])
base_path = "/".join(test_path.split("/")[:-1])
base_path = os.path.abspath(base_path)
config_path = "{}/nisq_benchmarks/config".format(base_path)
sys.path.append(config_path)
import Config_IBMQ_experience
from qiskit.transpiler.passes import resource_estimation

import argparse

using_config_account = False
try:
  provider = IBMQ.load_account()
except:
  print("No account found, using token in config")
  IBMQ_token = Config_IBMQ_experience.API_token #token needs to be added to file in config directory
  provider = IMBQ.enable_account(token=IBMQ_token)
  #IBMQ_URL = Config_IBMQ_experience.API_URL
  using_config_account = True
backend_sim = provider.get_backend('ibmq_qasm_simulator')

parser = argparse.ArgumentParser(description='Estimate Resources for OpenQASM File')
parser.add_argument('-f', action="store", dest="file_name")
print(parser.parse_args().file_name)

filename = parser.parse_args().file_name

if not filename:
  print("OpenQASM filename required")
  exit()
qc = QuantumCircuit.from_qasm_file(filename)

passmanager = PassManager()
passmanager.append(ResourceEstimation())
resources = qiskit.compiler.transpile(qc, backend_sim, pass_manager=passmanager)
print("Size: " + str(passmanager.property_set['size']) + "; Depth: " + str(passmanager.property_set['depth']) + "; Width: " + str(passmanager.property_set['width']) + "; Count Ops: " + str(passmanager.property_set['count_ops']))

if using_config_account:
  IBMQ.disable_accounts(token=IBMQ_token)
