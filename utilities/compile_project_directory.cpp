#include <iostream>
#include <cstdlib>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

int comp_proj(const string& proj_dir, const string& out_name) {
  string compiler = "E:\\mingw64\\bin\\g++.exe";
  string command;

  command = compiler + " -o " + out_name;

  for(const auto& entry : fs::directory_iterator(proj_dir)) {
    if(entry.path().extension() == ".cpp") {
      command += " " + entry.path().string();
    }
  }

  command += " -std=c++17";

  cout << "Compiling with command: " << command << endl;

  int result = system(command.c_str());

  if(result==0) {
    cout << "Compilation succeeded!\n";
  } else {
    cerr << "Compilation failed with code: " << result << endl;
  }

  return result;
}

int main() {
  string projPath;
  string outFileName;

  cout << "Enter the path to your C++ project directory: ";
  cin >> projPath;
  cin.ignore();
  cin.ignore();
  cout << "Enter the name for the output executable: ";
  cin >> outFileName;

  return comp_proj(projPath, outFileName);
}
