#!/bin/bash
# This script generates the figures for lesson 17

make_gif()
{
for f in tmp/frames_*.tif; do convert $f ${f/tif/gif}; done
gifsicle --delay $2 --colors 256 --loop tmp/frames*.gif > tmp/$1
convert -layers Optimize tmp/$1 $1
rm tmp/frames_*if
}

make -C ../.. bin/HalideTraceViz

rm -rf tmp
mkdir -p tmp

# Grab a trace
HL_TRACE=3 HL_TRACE_FILE=$(pwd)/tmp/trace.bin make -C ../.. tutorial_lesson_17_predicated_rdom
ls tmp/trace.bin

# Circular rdom
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 236 236 -t 1 -d 10000 -h 4 -f circle 0 20 -1 0 30 2 10 10 1 0 0 1 | avconv -f rawvideo -pix_fmt bgr32 -s 236x236 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_17_rdom_circular.gif 4

# Triangular rdom
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 236 236 -t 1 -d 10000 -h 16 -f triangle 0 32 -1 0 20 2 10 10 1 0 0 1 | avconv -f rawvideo -pix_fmt bgr32 -s 236x236 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_17_rdom_triangular.gif 1

# Rdom with calls in predicate
cat tmp/trace.bin | ../../bin/HalideTraceViz -s 400 236 -t 1 -d 10000 -h 4 -f call_f -1 13 -1 0 32 1 20 48 1 0 0 1 -f call_g -1 17 -1 0 32 1 228 48 1 0 0 1 -l call_f f 32 40 1 -l call_g g 240 40 1 |  avconv -f rawvideo -pix_fmt bgr32 -s 400x236 -i /dev/stdin tmp/frames_%04d.tif

make_gif lesson_17_rdom_calls_in_predicate.gif 10

rm -rf tmp
