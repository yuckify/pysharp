import sys
import clang.cindex
import argparse
import os.path

# download clang: http://releases.llvm.org/download.html
# install clang: pip install clang-5
# test command: python cppsharp.py -I . -I other_dir -I lib -f Test.hpp
# test dir:     /c/Users/yucki/workspace/cppsharp/cppsharp/test

def printTree(node, indent = 0):
	print(indent*'	' + '\'' + str(node.spelling) + '\' ' + str(node.kind))
	
	for c in node.get_children():
		printTree(c, indent + 1)

def isFunction(node):
	if (node.is_converting_constructor() or 
		node.is_copy_constructor() or 
		node.is_default_constructor() or 
		node.is_move_constructor() or
		node.is_static_method() or
		node.is_virtual_method()):
		return True
	else:
		return False

def __hasNodeKind(node, kind):
	if node.kind is kind:
		return True
	
	for c in node.get_children():
		if __hasNodeKind(node):
			return True
	
	return False

def hasConstructor(node):
	assert(node.kind is clang.cindex.CursorKind.CLASS_DECL or 
			node.kind is clang.cindex.CursorKind.STRUCT_DECL)
	
	return __hasNodeKind(node, clang.cindex.CursorKind.CONSTRUCTOR)

def hasDestructor(node):
	assert(node.kind is clang.cindex.CursorKind.CLASS_DECL or 
			node.kind is clang.cindex.CursorKind.STRUCT_DECL)
	
	return __hasNodeKind(node, clang.cindex.CursorKind.DESTRUCTOR)

def functionCount(node):
	if isFunction(node):
		return 1
	
	count = 0
	for c in node.get_children():
		count += functionCount(c)
	return count

def hasInteger(node):
	if node.kind is clang.cindex.CursorKind.INTEGER_LITERAL:
		return True
	
	for c in node.get_children():
		if hasInteger(c):
			return True
	return False

def hasExport(node):
	if node.kind is clang.cindex.CursorKind.ANNOTATE_ATTR and 'export' in str(node.spelling):
		return True
	
	for c in node.get_children():
		if hasExport(c):
			return True
	return False

def isExport(node):
	for c in node.get_children():
		if c.kind is clang.cindex.CursorKind.ANNOTATE_ATTR and 'export' in str(c.spelling):
			return True
	
	return False

def printRefNamespace(node, data):
	if node is None:
		return ''
	elif node.kind is clang.cindex.CursorKind.TRANSLATION_UNIT:
		return ''
	else:
		res = printRefNamespace(node.semantic_parent, data)
		if res is not '':
			return res + '::' + node.spelling + '::'
	
	return node.spelling

# node is the node of the class we are currently processing
def printDeclNamespaceOpen(node, data):
	if node is None:
		return ''
	elif node.kind is clang.cindex.CursorKind.TRANSLATION_UNIT:
		return ''
	else:
		res = printDeclNamespaceOpen(node.semantic_parent, data)
		if res is not '':
			return res + 'namespace ' + node.spelling + ' {'
	
	return 'namespace ' + node.spelling + ' { '

def printDeclNamespaceClose(node, data):
	if node is None:
		return ''
	elif node.kind is clang.cindex.CursorKind.TRANSLATION_UNIT:
		return ''
	else:
		res = printDeclNamespaceClose(node.semantic_parent, data)
		if res is not '':
			return res + '} '
	
	return '} '

def printExports(node, indent = 0):
#	if node.kind is clang.cindex.CursorKind.CLASS_DECL:
	if 'RetTest' in str(node.spelling) and node.kind is clang.cindex.CursorKind.CLASS_DECL:
		print('HAS EXPORTS ' + str(hasExport(node)))
		print('EXPORT CLASS ' + node.spelling + ' ' + str(node.kind))
		print('referenced ' + printRefNamespace(node.referenced.semantic_parent, ''))
		print('OPEN ' + printDeclNamespaceOpen(node.referenced.semantic_parent, ''))
		print('CLOSE ' + printDeclNamespaceClose(node.referenced.semantic_parent, ''))
		printTree(node, indent)
		for t in node.get_tokens():
			print('SPELLING ' + str(t.spelling))
			print('LOCATION ' + str(t.location))
			print('CURSOR KIND ' + str(t.cursor.kind))
			print('KIND ' + str(t.kind))
			print("\n")
	
	for c in node.get_children():
		printExports(c, indent + 1)

def printAccess(node, data):
	if node.access_specifier is clang.cindex.AccessSpecifier.PUBLIC:
		sys.stdout.write('public ')

def isPrivate(node):
	if node.access_specifier is clang.cindex.AccessSpecifier.PRIVATE:
		return True
	return False

def isPublic(node):
	if node.access_specifier is clang.cindex.AccessSpecifier.PUBLIC:
		return True
	return False

def compileEnum(node, data):
	print()

def exportEnum(node, data):
	print('PRINT ' + node.spelling + ' ' + str(node.kind))
	for t in node.get_tokens():
			print('TOKEN ' + t.spelling + ' ' + str(t.kind))
	for c in node.get_children():
		exportEnum(c, data)

def exportAll(node, data):
	if node.kind is clang.cindex.CursorKind.CLASS_DECL and isPublic(node):
		printAccess(node, data)
		print('method count ' + str(functionCount(node)))
		print('class ' + node.spelling + ' : cppsharp.Object {')
	elif node.kind is clang.cindex.CursorKind.ENUM_DECL and isPublic(node):
		printAccess(node, data)
		print('enum ' + node.spelling + ' {')
	elif node.kind is clang.cindex.CursorKind.ENUM_CONSTANT_DECL and isPublic(node):
		sys.stdout.write(node.spelling + ' = ')
		if not hasInteger(node):
			print(str(data['last_int']) + ',')
			data['last_int'] = data['last_int'] + 1
	elif ((node.kind is clang.cindex.CursorKind.INTEGER_LITERAL and 
			data['last'].kind is clang.cindex.CursorKind.ENUM_CONSTANT_DECL) and isPublic(data['last'])):
		for t in node.get_tokens():
			print(t.spelling + ',')
			data['last_int'] = int(t.spelling) + 1
	elif node.kind is clang.cindex.CursorKind.FUNCTION_DECL:
		print('FUNCTION ' + node.spelling)
	elif node.kind is clang.cindex.CursorKind.CXX_METHOD:
		print('CXXMETHOD ' + node.spelling)
		exportEnum(node, data)
		for t in node.get_tokens():
			print(t.spelling + ' ' + str(t.kind))
		quit(0)
	elif node.kind is clang.cindex.CursorKind.CONSTRUCTOR:
		print('CONSTRUCTOR ' + node.spelling)
	elif node.kind is clang.cindex.CursorKind.DESTRUCTOR:
		print('DESTRUCTOR ' + node.spelling)
	elif node.is_default_constructor():
		print('DEFAULT CONT ' + node.spelling)
		
	
	for c in node.get_children():
		data['last'] = node
		exportAll(c, data)
	
	if ((node.kind is clang.cindex.CursorKind.NAMESPACE or
		node.kind is clang.cindex.CursorKind.CLASS_DECL or
		node.kind is clang.cindex.CursorKind.ENUM_DECL)  and isPublic(node)):
		print('}')

def compile(node, data):
	data['last'] = node
	
	if isExport(node):
		exportAll(node, data)
	elif hasExport(node):
		if node.kind is clang.cindex.CursorKind.NAMESPACE:
			print('namespace ' + node.spelling + ' {')
		elif node.kind is clang.cindex.CursorKind.CLASS_DECL:
			printAccess(node, data)
			print('class ' + node.spelling + ' : cppsharp.Object {')
		elif node.kind is clang.cindex.CursorKind.ENUM_DECL:
			printAccess(node, data)
			print('enum ' + node.spelling)
		
#		print('NODE ' + str(node.kind))
		for c in node.get_children():
			compile(c, data)
		
		if (node.kind is clang.cindex.CursorKind.NAMESPACE or
			node.kind is clang.cindex.CursorKind.CLASS_DECL or
			node.kind is clang.cindex.CursorKind.ENUM_DECL):
			print('}')

def startCompile(node, data):
	print('using System;')
	print('using System.Runtime.CompilerServices;')
	print ('using System.Reflection;')
	print('')
	
	compile(node, data)
	

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument('-f', '--file', type=str, help='input file to generate interface')
	parser.add_argument('-I', action='append', help='include path')
	parser.add_argument('-v', '--verbose', type=bool, nargs='?', const=True, default=False, help='print debug statements')
	args = parser.parse_args()
	
	if not args.file:
		print('Error: no file specified')
		exit(1)
	
	if not os.path.isfile(args.file):
		print('Error: file \'' + args.file + '\' does not exist')
		exit(1)
	
	clang_args = []
	for i in args.I:
		clang_args.append('-I' + i)
#	clang_args = ['-I.', '-Ilib', '-Iother_dir']
	
	cl = 'clang'
	for i in clang_args:
		cl = cl +' ' + i
	cl = cl +' ' + args.file
	if args.verbose:
		print(cl)
	
	data = {}
	data['header'] = 'test'
	data['source'] = 'test'
	data['init'] = 'test'
	data['last'] = ()
	data['last_int'] = 0
	
	index = clang.cindex.Index.create()
	tu = index.parse(args.file, args=clang_args)
#	printTree(tu.cursor)
#	printExports(tu.cursor)
	startCompile(tu.cursor, data)
	
	

if __name__ == '__main__':
	main()