#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
using namespace std;

int comp_proj(const string& proj_files, const string& out_name) {
  string compiler = "E:\\mingw64\\bin\\g++.exe";
  string command;
  string outputName = out_name;

  string output_folder = "output";
  fs::create_directories(output_folder);

  if (out_name.empty()) {
    outputName += "output";
  }

  string output_path = output_folder + "/" + outputName;
  
  command = compiler;

  istringstream ss(proj_files);
  string file;

  while (getline(ss, file, ' ')) {
    command += " " + file;
  }

  command += " -o " + output_path;

  command += " -std=c++17 -Werror -Wextra -Wconversion -Wshadow -Wpedantic";

  cout << "Compiling with command: " << command << endl;

  int result = system(command.c_str());

  if (result == 0) {
    cout << "Compilation succeeded!\n";
  } else {
    cerr << "Compilation failed with code: " << result << endl;
  }

  return result;
}

int main() {
  string projFiles;
  string outFileName;

  cout << "Enter the C++ files to compile: ";
  getline(cin, projFiles);

  cout << "Enter the name for the output executable: ";
  getline(cin, outFileName);

  projFiles.erase(remove(projFiles.begin(), projFiles.end(), '\r'),
                  projFiles.end());

  return comp_proj(projFiles, outFileName);
}
