// g++ -std=c++17 -o dir_inspector dir_inspector.cpp
// g++ -std=c++17 -o dir_inspector dir_inspector.cpp -lstdc++fs
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

void del_r(const fs::path& path) {
  try {
    if (fs::exists(path)) {
      fs::remove_all(path);
      cout << "Deleted folder: " << path.string() << endl;
    }
  } catch (const fs::filesystem_error& e) {
    cerr << "Error deleting folder " << path.string() << ": " << e.what()
         << endl;
  }
}

void del_f(const fs::path& path) {
  try {
    if (fs::exists(path)) {
      fs::remove(path);
      cout << "Deleted file: " << path.string() << endl;
    }
  } catch (const fs::filesystem_error& e) {
    cerr << "Error deleting file " << path.string() << ": " << e.what() << endl;
  }
}

void insp_dir(const fs::path& dirPath) {
  try {
    if (fs::exists(dirPath) && fs::is_directory(dirPath)) {
      for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
        if (fs::is_directory(entry)) {
          if (entry.path().filename() == "nbproject" ||
              entry.path().filename() == "build" ||
              entry.path().filename() == "dist") {
            del_r(entry.path());
          }
        } else if (fs::is_regular_file(entry)) {
          if (entry.path().extension() == ".inc" ||
              entry.path().extension() == ".Rhistory" ||
              entry.path().extension() == ".exe" ||
              entry.path().filename() == "Makefile") {
            del_f(entry.path());
          }
        }
      }
    } else {
      cerr << "Provided path is not a directory or does not exists: "
           << dirPath.string() << endl;
    }
  } catch (const fs::filesystem_error& e) {
    cerr << "Error inspecting directory: " << e.what() << endl;
  }
}

int main() {
  string path;
  cout << "Enter directory to inspect: ";
  cin >> path;
  insp_dir(fs::path(path));
  return 0;
}
