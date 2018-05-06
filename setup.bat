mkdir build
cd build

conan install -g cmake_multi -s build_type=Release ..
conan install -g cmake_multi -s build_type=Debug ..
cmake -G "Visual Studio 15 2017 Win64" ..

cd ..