#!/usr/bin/env python

import os
import sys
import random
import argparse

parser = argparse.ArgumentParser(description="Stabilizer Compiler Driver")

# Driver control arguments
parser.add_argument('-v', action='store_true')
parser.add_argument('-fortran', action='store_true')
parser.add_argument('-gcc', action='store_true')
parser.add_argument('-linkrand', action='store_true')

# Compiler pass-through arguments
parser.add_argument('-c', action='store_true')
parser.add_argument('-o')
parser.add_argument('-O', type=int, default=2)
parser.add_argument('-f', action='append', default=[])
parser.add_argument('-D', action='append', default=[])
parser.add_argument('-L', action='append', default=[])
parser.add_argument('-I', action='append', default=[])
parser.add_argument('-l', action='append', default=[])
parser.add_argument('input', nargs='+')

# Do the parse
args = parser.parse_args()

FORTRAN = args.fortran

opts = []

args.l.append('stdc++')

def compile(i, args, verbose=False):
	if i.endswith('.o') or i.endswith('.bc'):
		return i

	if FORTRAN:
		cmd = 'llvm-gcc -S -fplugin-arg-dragonegg-emit-ir'
		cmd += ' -o '+args.o+'.s'
	elif args.gcc:
		cmd = 'llvm-gcc -S -fplugin-arg-dragonegg-emit-ir'
		cmd += ' -o '+args.o+'.s'
	else:
		cmd = 'clang -c -emit-llvm'
		cmd += ' -o '+args.o+'.bc'

	cmd += ' -O'+str(args.O)

	for I in args.I:
		cmd += ' -I'+I
	
	for f in args.f:
		cmd += ' -f'+f

	for D in args.D:
		cmd += ' -D'+D

	cmd += ' '+i

	if verbose:
		print cmd
	os.system(cmd)

	if FORTRAN or args.gcc:
		cmd = 'llvm-as -o '+args.o+'.bc '+args.o+'.s'
		if verbose:
			print cmd
		os.system(cmd)

	return args.o+'.bc'

def transform(i, args, verbose=False):
	if i.endswith('.o') or i.endswith('.opt.bc'):
		return i

	if len(opts) == 0:
		return i

	cmd = 'opt -o='+args.o+'.opt.bc'

	cmd += ' -load='+STABILIZER_HOME+'/LLVMStabilizer.so'

	for opt in opts:
		cmd += ' -'+opt

	cmd += ' '+i

	if verbose:
		print cmd
	os.system(cmd)

	return args.o+'.opt.bc'

def link(inputs, args, verbose=False):
	if FORTRAN and not args.c:
		cmd = 'gfortran'
	else:
		cmd = 'clang'

	if args.c:
		cmd += ' -c'
	else:
		for L in args.L:
			cmd += ' -L'+L
		
		for l in args.l:
			cmd += ' -l'+l

	cmd += ' -o '+args.o
	
	for f in args.f:
		cmd += ' -f'+f

	if args.linkrand:
		random.shuffle(inputs)
		print 'Random link order:', ' '.join(inputs)

	cmd += ' '+' '.join(inputs)

	if verbose:
		print cmd
	os.system(cmd)

	return args.o

# Build up program arguments
bytecode_files = []
for i in args.input:
	bytecode_files.append(compile(i, args, verbose=args.v))

transformed = []
for b in bytecode_files:
	transformed.append(transform(b, args, verbose=args.v))

link(transformed, args, verbose=args.v)

