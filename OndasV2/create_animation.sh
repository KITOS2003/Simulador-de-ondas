Fail()
{
    echo "FATAL($0): $1"
    exit 1
}

rm -rf output || Fail "No se pudo borrar el output anterior"

mkdir output || Fail "No se pudo crear la carpeta nueva"

./wave_sim 

ffmpeg -r 30 -f image2 -s 1280x720 -i output/frame_%05d.png -vcodec libx264 -crf 15 -pix_fmt yuv420p wave_sim.mp4 ||
    Fail "Error in ffmpeg."
