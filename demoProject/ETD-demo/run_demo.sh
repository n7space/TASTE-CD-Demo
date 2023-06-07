pushd work/binaries
./demo & sleep 1
./ScienceGUI&
./MCGUI
popd
pkill -INT demo