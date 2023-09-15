$image_dir = "sse_test\TestImages"
$output_dir = $image_dir + "\" + "downscaled"
$downscaler = "build\windows\downscale.exe"
$algorithms = "nearest", "average", "lanczos"
$images = "ImageTestRGB.tga", "ImageTestRGBA.tga"

if(-Not (Test-Path -Path $downscaler -PathType Leaf))
{
    #$sources = Select-String -Path "source\*.cpp"
    #x86_64-w64-mingw32-g++ -static -g -std=c++20 -O3 -mavx -mavx2 -I source -I third_party $sources -o $downscaler
}

if(-Not (Test-Path -Path $output_dir -PathType Container))
{
    mkdir $output_dir
}

foreach($input_image in $images)
{
    foreach($algorithm in $algorithms)
    {
        Write-Host "downscaling $input_image - $algorithm"

        $downscaled_name = [System.Io.Path]::GetFileNameWithoutExtension($input_image) + "_" + $algorithm + ".tga"

        $input = $input_image
        $output = Join-Path -Path $output_dir -ChildPath $downscaled_name

        Start-Process -FilePath $downscaler -WorkingDirectory $image_dir -ArgumentList $input, $output, $algorithm
    }
}
