param(
    [Parameter(Mandatory = $true)]$PathToVideo,
    [Parameter(Mandatory = $true)]$PathToOutputPngs
)

$VideoBasename = (Split-Path $PathToVideo -Leaf).split('.')[0]
ffmpeg -i $PathToVideo -vf "select='eq(pict_type\,I)*not(mod(n\,4))',showinfo" -vsync vfr "$PathToOutputPngs\$($VideoBaseName)_%04d.png"