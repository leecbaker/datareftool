rm -r build
mkdir build
cd build
set BOOST_ROOT=C:\local\boost_1_58_0_vc12\
set BOOST_LIBRARYDIR=C:\local\boost_1_58_0_vc12\stage\lib
cmake -G "Visual Studio 12 Win64" -DMSVC_RUNTIME=dynamic -DBoost_DEBUG=ON ..
msbuild src/plugin.vcxproj /p:Configuration=Release /flp:logfile=plugin_build.log;verbosity=normal
cd ..
cp "C:\Users\lee\Documents\projects\datareftool\bin\datareftool\Release\win.xpl" "C:\Program Files (x86)\Steam\steamapps\common\X-Plane 10\Resources\plugins\DataRefTool\win.xpl"