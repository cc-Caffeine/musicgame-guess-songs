#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <ostream>
#include <sstream>
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

vector<string> split(const string &line) {
  vector<string> result;
  string word;
  stringstream ss(line);

  while (ss >> word) {
    result.push_back(word);
  }

  return result;
}

// 使输入的字符串以*号mask
icu::UnicodeString maskString(const icu::UnicodeString &input) {
  // 不要 trim；songsShadowed 必须和 songs 保持相同的字符顺序。
  const icu::UnicodeString &text = input;
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
    const icu::UnicodeString input = icu::UnicodeString::fromUTF8(arg);
    if (input.isEmpty()) {
      cout << "请输入一个字符" << endl;
      return 1;
    }

    // 按 Unicode code point 读取，而不是按 UTF-16 code unit 读取。
    const UChar32 target = input.char32At(0);
    const UChar32 foldedTarget = u_foldCase(target, U_FOLD_CASE_DEFAULT);

    // 记录已经开启的字符，避免 a 和 A 被重复添加。
    bool alreadyOpened = false;
    for (int32_t pos = 0; pos < songs.openedChars.length();) {
      const UChar32 openedChar = songs.openedChars.char32At(pos);
      if (u_foldCase(openedChar, U_FOLD_CASE_DEFAULT) == foldedTarget) {
        alreadyOpened = true;
        break;
      }
      pos += U16_LENGTH(openedChar);
    }

    if (!alreadyOpened) {
      songs.openedChars.append(target);
    }

    for (size_t songIndex = 0; songIndex < songs.songs.size(); ++songIndex) {
      const UnicodeString &song = songs.songs.at(songIndex);
      UnicodeString &shadowed = songs.songsShadowed.at(songIndex);

      int32_t songPos = 0;
      int32_t shadowedPos = 0;

      while (songPos < song.length() && shadowedPos < shadowed.length()) {
        const UChar32 songChar = song.char32At(songPos);
        const UChar32 shadowedChar = shadowed.char32At(shadowedPos);
        const int32_t shadowedLength = U16_LENGTH(shadowedChar);

        if (u_foldCase(songChar, U_FOLD_CASE_DEFAULT) == foldedTarget) {
          shadowed.replace(shadowedPos, shadowedLength,
                           UnicodeString(songChar));
        }

        songPos += U16_LENGTH(songChar);
        shadowedPos += shadowedLength;
      }
    }

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
    cout << "请输入选项: ";

    string line;
    if (!getline(cin >> ws, line)) {
      cout << endl;
      break; // Ctrl+D 或输入文件结束时退出
    }

    istringstream input(line);

    string cmd;
    string arg;

    if (!(input >> cmd)) {
      cout << "请输入命令" << endl;
      continue;
    }

    if (!(input >> arg)) {
      cout << "缺少参数." << endl;
      cout << endl;
      continue;
    }

    // cout << "请输入选项: ";
    // string line;
    // getline(cin, line);
    // if (line.empty()) {
    //   continue;
    // }
    //
    // vector<string> args = split(line);
    // cin >> args[0] >> args[1];
    // string cmd = args[0];
    // string arg = args[1];

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
