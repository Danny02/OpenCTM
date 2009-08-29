#!/bin/bash

# Name of the distribution
distname=OpenCTM-0.7

# Clean the source directories
echo Cleaning up...
make -f Makefile.linux clean
cd tools
make -f Makefile.linux clean
cd jpeg
make -f Makefile.linux clean
cd ../..

# Build all the necessary files
echo Building documentation...
cd doc
doxygen
pdflatex DevelopersManual.tex
pdflatex DevelopersManual.tex
cd ..

# Set up a temporary directory
tmproot=/tmp/openctm-$USER-$$
mkdir $tmproot
tmpdir=$tmproot/$distname
mkdir $tmpdir

# Copy files
echo Copying files to $tmpdir...
cp *.c *.h *.rc *.def Makefile* *.txt $tmpdir/
mkdir $tmpdir/liblzma
cp liblzma/* $tmpdir/liblzma/
mkdir $tmpdir/tools
cp tools/*.cpp tools/*.h tools/*.ico tools/*.vert tools/*.frag tools/*.rc tools/Makefile* $tmpdir/tools/
mkdir $tmpdir/tools/jpeg
cp tools/jpeg/* $tmpdir/tools/jpeg/
mkdir $tmpdir/tools/rply
cp tools/rply/* $tmpdir/tools/rply/
mkdir $tmpdir/tools/glew
cp tools/glew/* $tmpdir/tools/glew/
mkdir $tmpdir/tools/glew/GL
cp tools/glew/GL/* $tmpdir/tools/glew/GL/

mkdir $tmpdir/doc
cp doc/DevelopersManual.pdf $tmpdir/doc/
cp doc/format.txt $tmpdir/doc/
mkdir $tmpdir/doc/APIReference
cp doc/APIReference/* $tmpdir/doc/APIReference/

mkdir $tmpdir/plugins
mkdir $tmpdir/plugins/blender
cp plugins/blender/* $tmpdir/plugins/blender/

mkdir $tmpdir/bindings
mkdir $tmpdir/bindings/delphi
cp bindings/delphi/* $tmpdir/bindings/delphi/

# Create archives
olddir=`pwd`
cd $tmproot
tar -cvjf $distname.tar.bz2 $distname
