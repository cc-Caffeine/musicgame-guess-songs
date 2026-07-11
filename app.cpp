#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <utf8cpp/utf8.h>
#include <vector>

#define EXPECT_EQ(a, b)                                                        \
  if ((a) != (b)) {                                                            \
    cerr << "EXPECT_EQ failed: " << #a << " != " << #b << "\n";                \
  }

using namespace std;

// 使输入的string以*号mask
std::string maskString(const std::string &input) {
  std::string result;

  auto it = input.begin();
  auto end = input.end();

  while (it != end) {
    uint32_t codepoint = utf8::next(it, end);

    // 保留空格
    if (codepoint == U' ' || codepoint == '\n') {
      result += ' ';
    } else {
      result += '*';
    }
  }

  return result;
}

struct Songs {
  string openedChars;
  vector<string> songs;
  vector<string> songsShadowed;
  int numberOfSongs;

  // init
  Songs(int argc, char **argv) {

    // init songs, 遍历所有给出的文件
    for (int i = 1; i < argc; i++) {
      ifstream file(argv[i]);

      if (!file.is_open()) {
        cerr << "无法打开文件" << endl;
        exit(1);
      }

      string line;

      while (getline(file, line)) {
        songs.emplace_back(line + "\n");
      }

      file.close();
    }

    // init songsShadowed
    for (auto s : songs) {
      songsShadowed.emplace_back(maskString(s));
    }
  }

  // 格式化输出SongsShadowed
  void printSongsShadowed() {
    int i = 1;
    for (auto s : songsShadowed) {
      cout << i << ": ";
      cout << s << endl;
      ++i;
    }
  }
};

int main(int argc, char *argv[]) {
  // 检查参数够不够
  if (argc == 1) {
    cerr << "error: Need more arguments\n";
    return 1;
  }

  // init songs
  Songs songs(argc, argv);

  while (true) {
    cout << "已经开启了的字母: " << endl;

    songs.printSongsShadowed();
    cout << endl;

    string line;
    cout << "请输入选项: ";
    cin >> line;
    string lineFirstChar = line.begin();

    cout << endl;
  }

  return 0;
}
