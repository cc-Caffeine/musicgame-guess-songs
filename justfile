build:
  g++ -std=c++23 -Wall -Wextra -o build/app src/app.cpp -licuuc

run: build
  ./build/app data/songs.txt


