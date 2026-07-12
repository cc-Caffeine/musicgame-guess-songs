#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
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
using icu::UnicodeString;

// 使输入的字符串以*号mask
icu::UnicodeString maskString(const icu::UnicodeString &input) {
  icu::UnicodeString text = input;
  text.trim();
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

  return result;
}

void replaceCodePoint(UnicodeString &target, int32_t targetIndex,
                      const UnicodeString &source, int32_t sourceIndex) {
  // 找目标 code point 的 UTF-16 位置
  int32_t targetPos = target.moveIndex32(0, targetIndex);

  // 得到目标 code point
  UChar32 oldCp = target.char32At(targetPos);

  // 目标占用的 UTF-16 长度
  int32_t oldLength = U16_LENGTH(oldCp);

  // 找源 code point
  int32_t sourcePos = source.moveIndex32(0, sourceIndex);

  UChar32 newCp = source.char32At(sourcePos);

  // 替换
  target.replace(targetPos, oldLength, UnicodeString(newCp));
}

struct Songs {
  icu::UnicodeString openedChars;
  vector<icu::UnicodeString> songs;
  vector<icu::UnicodeString> songsShadowed;
  int numberOfSongs = 0;

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
        icu::UnicodeString song = icu::UnicodeString::fromUTF8(line);
        songs.emplace_back(song);
        songsShadowed.emplace_back(maskString(song));
      }

      file.close();
    }

    numberOfSongs = static_cast<int>(songs.size());
  }

  // 格式化输出SongsShadowed
  void printSongsShadowed() {
    int i = 1;
    for (const auto &s : songsShadowed) {
      string output;
      s.toUTF8String(output);
      cout << i << ": ";
      cout << output << endl;
      ++i;
    }
  }
};

int main(int argc, char *argv[]) {
  Songs songs(argc, argv);

  // 注册命令
  using Command = function<int(const string)>;
  unordered_map<string, Command> commands;
  // ans 命令加上一个数字: 完全打开一首歌
  commands["ans"] = [&songs](const auto arg) {
    int i;
    try {
      i = stoi(arg) - 1;
    } catch (const exception &e) {
      cout << "不是一个数字" << endl;
      return 1;
    }

    if (i < 0 || i >= songs.numberOfSongs) {
      cout << "数字过大" << endl;
      return 1;
    }

    songs.songsShadowed.at(i) = songs.songs.at(i);
    return 0;
  };
  // open 命令加上一个字符: 开字母
  commands["open"] = [&songs](const auto arg) {
    icu::UnicodeString u = icu::UnicodeString::fromUTF8(arg);
    // 得到输入中的第一个point
    UChar32 c = u.char32At(0);

    for (auto s : songs.songs) {
      int indexOfSongs = 0;
      for (auto cp : s) {
        int indexOfC = 0;
        if (c == cp) {
          replaceCodePoint(songs.songsShadowed.at(indexOfSongs), indexOfC,
                           songs.songs.at(indexOfSongs), indexOfC);
        }
        indexOfC++;
      }
      indexOfSongs++;
    }

    // todo!
    return 0;
  };

  // 检查参数够不够
  if (argc == 1) {
    cerr << "error: Need more arguments\n";
    return 1;
  }

  // init songs

  while (true) {
    string openedChars;
    songs.openedChars.toUTF8String(openedChars);
    cout << "已经开启了的字母: " << openedChars << endl;

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

    cout << endl;
  }

  return 0;
}
