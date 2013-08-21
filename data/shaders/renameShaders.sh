#!/bin/bash

for FILE in *VertexShader.glsl 
do
	NEWFILE=${FILE/(.*)VertexShader.glsl/$1.vert}
	echo "mv $FILE $NEWFILE"
done
