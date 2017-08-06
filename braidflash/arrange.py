# Pranav Gokhale
# Usage: python arrange.py input_trace_file.tr [width height]
# (if width and height are not specified, default to sidelength
#  of smallest square big enough to contain all nodes)
# Input file: .tr file
# Output:
#    - placement of nodes on the rectangle

import os, sys, math, re
from os.path import basename
from os.path import splitext

module_num_nodes = 0


def main():
  trace_to_graph(sys.argv[1])
  mapping_dic = {}
  nodes_dic, tl_dic, br_dic = parser()  # tl = top left coordinate, br = bottom right coordinate
  benchname = splitext(basename(sys.argv[1]))[0] + '.p.' + sys.argv[2] + '.yx.' + sys.argv[3] + '.drop.' + sys.argv[4]  
  for module_name, nodes in nodes_dic.iteritems():
    locations = get_locations(nodes, tl=tl_dic[module_name], br=br_dic[module_name])
    os.system('rm -f '+benchname+'.delete_*')  # partition function creates many files prefixed with delete_
    node_map = {v: k for k, v in locations.items()}
    mapping = ''
    for y in range(br_dic[module_name].y + 1):
      for x in range(br_dic[module_name].x + 1):
        if node_map.get(Point(x, y)):
          mapping += str(node_map.get(Point(x, y)).index) + ' '
        else:
          mapping += ' '
      mapping += '\n'
    mapping_dic[module_name] = mapping
  replace(mapping_dic, sys.argv[1])
  os.system('rm -f '+benchname+'.delete_*')


def trace_to_graph(infile):
  """Converts .tr file to .graph file."""
  assert '.tr' in infile
  outfile = infile.replace('.tr', '.graph')

  infile = open(infile)
  outfile = open(outfile, 'w')

  for line in infile:
    if line.startswith('module:'):
      tokens = line.split(' ')
      module_name = tokens[1]
      outfile.write('module: %s' % module_name)
      continue
    elif line.startswith('num_nodes:'):
      tokens = line.split(' ')
      num_nodes = int(tokens[1])
      outfile.write('%s\n' % num_nodes)
      continue
    elif not line.startswith('ID:'):  # skip the header lines
      continue
    elif 'DST:' not in line:          # skip single qubit gates
      continue

    tokens = line.split(' ')
    assert tokens[4].startswith('SRC:')
    assert tokens[6].startswith('DST:')
    src = int(tokens[5])
    dst = int(tokens[7])
    assert src < num_nodes
    assert dst < num_nodes
    outfile.write('%s %s\n' % (src, dst))

def parser():
  """Parses nodes from .graph input file. Returns nodes, br, tl.

    Example .graph file:
    5
    0 1
    1 2
    2 3
    3 4

    creates
    nodes = [
    Node(0, [0, 1, 0, 0, 0])
    Node(1, [1, 0, 1, 0, 0])
    Node(2, [0, 1, 0, 1, 0])
    Node(3, [0, 0, 1, 0, 1])
    Node(4, [0, 0, 0, 1, 0])
    """
  global module_num_nodes
  nodes_dic = {}
  tl = {}
  br = {}
  
  def _square_size(module_num_nodes):
    """Length of the smallest integer sized square with area of at least module_num_nodes.

      (size - 1) * (size - 1) < module_num_nodes <= size * size"""
    return int(math.ceil(math.sqrt(module_num_nodes)))

  def _add_edge(src, dst):
    """Add an undirected edge."""
    module_nodes[src].weights[dst] += 1
    module_nodes[dst].weights[src] += 1

  f = open(sys.argv[1].replace('.tr', '.graph'))
  line = f.readline()

  module_name = ''
  while line != '':
    # head of a module
    if line.startswith('module:'):
      if module_name != '':        # save previous module's info
        nodes_dic[module_name] = module_nodes
        tl[module_name] = Point(0, 0)
        br[module_name] = Point(module_width - 1, module_height - 1)
    
      tokens = line.split(' ')
      module_name = tokens[1]
      module_num_nodes = int(f.readline())
      module_nodes = []

      if len(sys.argv) == 4:        # if width and height are specified from command line
        module_width = int(sys.argv[5])
        module_height = int(sys.argv[6])
        assert module_width * module_height >= module_num_nodes
      else:
        square_size = _square_size(module_num_nodes)
        module_width, module_height = square_size, square_size

      for i in range(module_num_nodes):
        n = Node(i, module_num_nodes * [0])
        module_nodes.append(n)

    # body of a module
    else:
      tokens = line.split(' ')      # expected format of each line is "src dst"
      assert len(tokens) == 2
      src, dst = int(tokens[0]), int(tokens[1])
      _add_edge(src, dst)

    line = f.readline()  

  nodes_dic[module_name] = module_nodes # save last module's info
  tl[module_name] = Point(0, 0)
  br[module_name] = Point(module_width - 1, module_height - 1)

  return nodes_dic, tl, br

class Node(object):
  def __init__(self, index, weights):
    self.index = index
    self.weights = weights
  
  def __repr__(self):
    return '%s' % (self.index)


class Point(object):
  # direction of coordinate system is as follow:
    # 0,0  1,0  2,0
    # 0,1  1,1  2,1
    # 0,2  1,2  2,2
  def __init__(self, x, y):
    self.x = x
    self.y = y

  def __repr__(self):
    return '(%s, %s)' % (self.x, self.y)

  def __hash__(self):
    return hash((self.x, self.y))

  def __eq__(self, another):
    return self.x == another.x and self.y == another.y


def get_locations(nodes, tl, br):
  """Returns a mapping from each node in nodes to a location in the tl<->br square.

  tl and br are tuples representing (x, y) coordinates of the top left and bottom right.
  """
  
  # Base cases:
  if len(nodes) == 1:  # for singleton, only choice is to place in the single spot in 1x1 square
    return {nodes[0]: tl}
  if len(nodes) == 2:  # for two nodes, arbitrarily chose to place the first node in top left
    return {nodes[0]: tl, nodes[1]: br}

  # Recursive case, need to create and solve subproblems:
  ret = {}

  num_edges = count_num_edges(nodes)
  if num_edges == 0:  # for empty graphs, no need to run METIS, just assign arbitrarily
    i = 0
    for x in range(tl.x, br.x+1): 
      for y in range(tl.y, br.y+1):
        if i < len(nodes):
          ret.update({nodes[i]: Point(x,y)})
          i += 1
    return ret

  filename = splitext(basename(sys.argv[1]))[0] + '.p.' + sys.argv[2] + '.yx.' + sys.argv[3] + '.drop.' + sys.argv[4] + '.' +\
      '_'.join(['delete', str(tl.x), str(tl.y), str(br.x), str(br.y)])  

  # special case for the very first call of get_locations. For example, suppose that there are
  # 97 nodes on a 10x10 grid. Instead of dividing the 97 nodes into 2 equal partitions, we should
  # divide them into a partition of 90 nodes and a partition of 7 nodes. The former should be
  # placed on a 10x9 grid and te latter should be placed on a 1x7 grid.
  if len(nodes) < (br.x - tl.x + 1) * (br.y - tl.y + 1):
    assert tl == Point(0, 0)
    size_tl_nodes = (br.x + 1) * int(len(nodes) / (br.x + 1))
    if size_tl_nodes == len(nodes):
      ret.update(get_locations(nodes, tl=Point(0, 0), br=Point(br.x, len(nodes) / (br.x + 1) - 1)))
      return ret

    nodes_tl, nodes_br = partition(nodes, size_tl_nodes, filename)
    # complicated indexing here. As an example, for the 97 into 10x10 case, we want to send 90 nodes
    # to a rectangle spanned by tl=Point(0, 0) and br=Point(9, 8) and we want to send 7 nodes to a 
    # rectangle spanned by tl=Point(0, 9) and br=Point(6, 9)
    ret.update(get_locations(nodes_tl, tl=Point(0, 0), br=Point(br.x, len(nodes) / (br.x + 1) - 1)))
    ret.update(get_locations(nodes_br, tl=Point(0, len(nodes) / (br.x + 1)), br=Point(len(nodes) % (br.x + 1) - 1, len(nodes) / (br.x + 1))))
    return ret

  if br.x - tl.x > br.y - tl.y:  # if rectangle is wider than tall, split on y axis
    half = tl.x + (br.x - tl.x - 1) / 2
    size_tl_nodes = (half - tl.x + 1) * (br.y - tl.y + 1)
  else:  # split on x axis
    half = tl.y + (br.y - tl.y - 1) / 2
    size_tl_nodes = (br.x - tl.x + 1) * (half - tl.y + 1)

  nodes_tl, nodes_br = partition(nodes, size_tl_nodes, filename)

  if br.x - tl.x > br.y - tl.y:  # if rectangle is wider than tall, split on y axis
    ret.update(get_locations(nodes_tl, tl=tl, br=Point(half, br.y)))
    ret.update(get_locations(nodes_br, tl=Point(half + 1,tl.y), br=br))
  else:  # split on x axis
    ret.update(get_locations(nodes_tl, tl=tl, br=Point(br.x, half)))
    ret.update(get_locations(nodes_br, tl=Point(tl.x, half + 1), br=br))

  return ret


def count_num_edges(nodes):
  num_edges = 0
  for src_node in nodes:
    for dst_node in nodes:
      if src_node.weights[dst_node.index]:
        num_edges += 1
  num_edges /= 2  # don't double count edges
  
  return num_edges


def partition(nodes, size_nodes_tl, filename):
  """Returns nodes_tl, nodes_br."""
  weight_0 = float(size_nodes_tl) / len(nodes)
  num_edges = count_num_edges(nodes)

  f = open(filename, 'w')
  f.write('%s %s 001\n' % (len(nodes), num_edges))
  for src_node in nodes:
    x = 1
    for dst_node in nodes:
      if src_node.weights[dst_node.index]:
        f.write('%s %s ' % (x, src_node.weights[dst_node.index]))
      x += 1
    f.write('\n')
  f.close()

  benchname = splitext(basename(sys.argv[1]))[0] + '.p.' + sys.argv[2] + '.yx.' + sys.argv[3] + '.drop.' + sys.argv[4]  
  f = open(benchname+'.delete_tpwgts', 'w')  # target partition weights file
  f.write('0 = %s' % str(weight_0))
  f.close()

  os.system('gpmetis -ptype=rb -tpwgts='+benchname+'.delete_tpwgts -ufactor=1 -ncuts=500 ' + filename + ' 2 > /dev/null')

  f = open(filename + '.part.2')
  nodes_tl, nodes_br = [], []
  index = 0
  for line in f:
    if line == '0\n':
      nodes_tl.append(nodes[index])
    else:
      nodes_br.append(nodes[index])
    index += 1

  assert len(nodes_tl) + len(nodes_br) == len(nodes), 'nodes went missing somehow :('

  # METIS is not perfect and will not always actually yield size_nodes_tl nodes in nodes_tl
  # in this case, just pop enough nodes from one list to the other, until sizes are appropriate
  # TODO: pick the nodes to transfer between lists more intelligently
  while len(nodes_tl) > size_nodes_tl:
    nodes_br.append(nodes_tl.pop())
  while size_nodes_tl > len(nodes_tl):
    nodes_tl.append(nodes_br.pop())

  return nodes_tl, nodes_br


def replace(mapping_dic, f_tr):
  """Replace the nodes in the .tr file using the mapping and generate .opt.tr file."""
  f_opt_tr = f_tr.replace('.tr', '.opt.tr')
  f_tr = open(f_tr)
  f_opt_tr = open(f_opt_tr, 'w')
  for line in f_tr:
    if line.startswith('module: '):
      tokens = line.split(' ')
      module_name = tokens[1]
      mapping = mapping_dic[module_name].split()
      d = {}
      i = 0
      for item in mapping:
        d[int(item)] = i
        i += 1
    if 'SRC' not in line:
      f_opt_tr.write(line)
      continue
    line =  re.sub(r'SRC: (\d+)', lambda match: 'SRC: '+str(d.get(int(match.group(1)), 'SOMETHING WENT WRONG' + match.group(1))), line)
    line =  re.sub(r'DST: (\d+)', lambda match: 'DST: '+str(d.get(int(match.group(1)), 'SOMETHING WENT WRONG' + match.group(1))), line)
    f_opt_tr.write(line)


if __name__ == "__main__":
  main()
