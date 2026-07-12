#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <unicode/uchar.h>
#include <unicode/umachine.h>
#include <unicode/unistr.h>
#include <unicode/urename.h>
#include <unicode/utf16.h>
#include <unicode/utf8.h>
#include <unordered_map>
#include <vector>

#define EXPECT_EQ(a, b)                                                        \
  if ((a) != (b)) {                                                            \
    cerr << "EXPECT_EQ failed: " << #a << " != " << #b << "\n";                \
  }

using namespace std;

// 使输入的string以*号mask
string maskString(const string &input) {
  icu::UnicodeString text = icu::UnicodeString::fromUTF8(input).trim();
  icu::UnicodeString result;

  for (int32_t i = 0; i < text.length();) {
    UChar32 c = text.char32At(i);

    // 保留空格和换行
    if (c == U' ') {
      result.append(c);
    } else {
      result.append((UChar32)'*');
    }

    i += U16_LENGTH(c);
  }

  string output;
  result.toUTF8String(output);

  return output;
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
  Songs songs(argc, argv);
  // 注册命令
  using Command = function<void(const string)>;
  unordered_map<string, Command> commands;
  commands["ans"] = [songs](const auto arg) {
    int i;
    try {
      i = stoi(arg);
    } catch (const exception &e) {
      cout << "不是一个数字" << endl;
      return 1;
    }

    if (i == songs.numberOfSongs) {
      songs.songsShadowed[--i] = songs.songs.at(--i);
    }
  };

  // 检查参数够不够
  if (argc == 1) {
    cerr << "error: Need more arguments\n";
    return 1;
  }

  // init songs

  while (true) {
    cout << "已经开启了的字母: " << songs.openedChars << endl;

    songs.printSongsShadowed();
    // 手动换行
    cout << endl;

    vector<string> args(2);
    cout << "请输入选项: ";
    cin >> args[0] >> args[1];
    string cmd = args[0];
    string arg = args[1];

    auto it = commands.find(cmd);

    if (it == commands.end()) {
      cout << "未知的命令" << endl;
    } else {
      it->second(arg);
    }
    // icu::UnicodeString uline = icu::UnicodeString::fromUTF8(line);
    // if (uline.length() == 1) {
    //   uline.toUTF8String(songs.openedChars);
    //   songs.openedChars.append(line);
    // }

    cout << endl;
  }

  return 0;
}
