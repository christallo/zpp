from data import MappedAst, Node, Symbol
from lex import lex
from parse import parse
from utils import error, fixpath, getabspath, var_is_comptime

import utils

def cache_mapast(path, ast):
  from gen import Generator

  if path not in utils.cache:
    generator = Generator(None, None, None)
    m, import_nodes = mapast_except_imports(ast, generator)

    imports = {
      get_full_import_path(path, import_node.path.value): ([
        (id.name.value, id.alias.value, id.pos) for id in import_node.ids
      ] if isinstance(import_node.ids, list) else import_node.ids) for import_node in import_nodes
    }

    generator.maps, generator.imports, generator.path = [m], imports, path
    utils.cache[path] = generator
    mapast_imports(import_nodes, path)
  
  return utils.cache[path]

def mapast_except_imports(ast_to_map, generator):
  m = MappedAst()
  import_nodes = []

  for glob in ast_to_map:
    match glob.kind:
      case 'fn_node':
        m.declare_symbol(
          glob.name.value,
          Symbol('fn_sym', node=glob, generator=generator),
          glob.pos
        )
      
      case 'type_decl_node':
        m.declare_symbol(
          glob.name.value,
          Symbol('type_sym' if len(glob.generics) == 0 else 'generic_type_sym', node=glob, generator=generator),
          glob.pos
        )
      
      case 'var_decl_node':
        is_comptime = var_is_comptime(glob.name.value)
        m.declare_symbol(
          glob.name.value,
          Symbol('global_var_sym', is_comptime=is_comptime, node=glob, generator=generator),
          glob.pos
        )
      
      case 'import_node':
        import_nodes.append(glob)
      
      case _:
        raise NotImplementedError()
  
  return m, import_nodes

def mapast_imports(import_nodes, srcpath):
  for glob in import_nodes:
    path = get_full_import_path(srcpath, glob.path.value)

    if path in utils.cache:
      continue

    try:
      with open(path, 'r') as f:
        src = f.read()
    except OSError:
      error(f'file not found (`{path}`)', glob.path.pos)
    
    toks = lex(src, path)
    ast = parse(toks)
    _ = cache_mapast(path, ast)

def get_full_import_path(srcpath, import_path):
  return fixpath('/'.join(srcpath.split('/')[:-1]) + '/' + import_path)
