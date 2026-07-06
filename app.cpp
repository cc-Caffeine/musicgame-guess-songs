#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#define EXPECT_EQ(a, b)                                                        \
  if ((a) != (b)) {                                                            \
    cerr << "EXPECT_EQ failed: " << #a << " != " << #b << "\n";                \
  }

using namespace std;

// 使输入的string以*号mask
string maskString(const string s) {
  string out = s;
  for (char &c : out) {
    if (c != ' ') {
      c = '*';
    }
  }

  return out;
}

struct Songs {
  string openedChars;
  vector<string> songs;
  vector<string> songsShadowed;

  Songs(int argc, char **argv) {
    songs.reserve(argc);

    // init songs
    for (int i = 1; i < argc; i++) {
      songs.emplace_back(argv[i]);
    }

    // init songsShad
    for (auto s : songs) {
      songsShadowed.emplace_back(maskString(s));
    }
  }
};

int main(int argc, char *argv[]) {

  // 检查参数够不够
  if (argc == 1) {
    cout << "error: Need more arguments\n";
    return 1;
  }

  // init songs
  Songs songs(argc, argv);

  return 0;
}
