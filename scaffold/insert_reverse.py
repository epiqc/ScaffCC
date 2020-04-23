import sys
import re
import mmap

if __name__ == "__main__":
  if len(sys.argv) < 2:
    print("Must provide a filename as an argument")
    exit(1)
  file_name = sys.argv[1]

  existing_reverse = set()
  needed_reverse = set()
  module_signatures = {}
  call_signatures = {}
  with open(file_name, 'r+') as f:
    data = mmap.mmap(f.fileno(), 0)
    decls_re = re.compile(b"^[ \t\f\v]*scaff_module\s+(?P<name>\w+)\s*(?P<args>\([^\)]*\))\s*({|;)?", re.MULTILINE)
    reverse_re = re.compile(b"^(?!scaff_module)\s*_reverse_(?P<rev_gate>\w+)\(([^\)]*)\)", re.MULTILINE)
    decl_matches = decls_re.findall(data)
    reverse_matches = reverse_re.findall(data)
    for match in decl_matches:
      if match[0].startswith(b"_reverse_"):
        existing_reverse.add(match[0][len("_reverse_"):])
      module_signatures[match[0]] = match[1]
    for match in reverse_matches:
      needed_reverse.add(match[0])
      call_signatures[match[0]] = match[1]

  #print(existing_reverse)
  #print(needed_reverse)
  #print(module_signatures)
  split_name = file_name.split(".")
  new_file_name = ".".join(split_name[:-3] + [split_name[-2]+"_reverse_insert"] + split_name[-1:])
  new_file_obj = open(new_file_name, "w+")
  for module in needed_reverse:
    new_name = str(module)[2:-1]
    if module in module_signatures:
      signature = str(module_signatures[module])[2:-1]
    else:
      # we assume all qubits for builtins
      arguments = str(call_signatures[module])[2:-1].split(",")
      signature = "({})".format(",".join(["qbit" for i in range(len(arguments))]))
    full_sig = "scaff_module _reverse_{}{};".format(new_name, signature)
    new_file_obj.write(full_sig + "\n")
  new_file_obj.close() 
