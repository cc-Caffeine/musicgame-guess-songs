build:
  g++ -std=c++23 -o app app.cpp -licuuc -Wall 

run: build
  ./app


