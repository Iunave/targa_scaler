#!/bin/bash

workdir="sse_test/TestImages"
downscaler="../../build/downscale"
downscaled_dir="downscaled"

algorithms=("nearest" "average" "lanczos")
images=("ImageTestRGB.tga" "ImageTestRGBA.tga")

make -j all

[ -e "sse_test/TestImages/${downscaled_dir}" ] || mkdir sse_test/TestImages/${downscaled_dir}

for image in "${images[@]}"
do
  for algorithm in "${algorithms[@]}"
  do
    echo "testing $image - $algorithm"
    image_name="${image%.*}"
    (cd $workdir && exec $downscaler $image ${downscaled_dir}/${image_name}_${algorithm}.tga $algorithm)
  done
done

