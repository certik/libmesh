#!/bin/sh
# This script makes a movie using the interactive
# mode of gmv.
# Set window size: -w 639 100 640 480

# Change this line if you are using the Mesa-enabled
# version of gmv.
gmv=gmv;
#gmv=linuxMesa;

for i in *.gmv.*; do
    $gmv -m -a gmv_attributes.attr  -i $i -s $i.rgb;
done

# for i in *.gmv.*.rgb; do
#     echo "Converting rgb to png ...";
#     convert $i $i.png;
# done

# Remove the rgb files.
#rm *.rgb
