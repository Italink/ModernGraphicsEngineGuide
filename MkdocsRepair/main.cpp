#include <filesystem>

#include <iostream>
#include <filesystem>
#include <fstream>

using namespace std;
using namespace std::filesystem;

bool isSeparator(const char& ch) {
	return ch == '\n'|| ch == ' '||ch == '\t';
}

std::string repair(const std::string& str) {
	std::string result;
	result.reserve(str.size());
	bool isLeft = true;
	bool inCode = false;
	bool inTip = false;
	int XingCounter = 0;
	int dddCounter = 0;
	for (int i = 0; i < str.size(); i++) {
		result = result + str[i];
		if (result.back() == '`') {
			dddCounter++;
		}
		else {
			if (dddCounter == 3) {
				inCode = !inCode;
			}
			if (dddCounter == 1) {
				inTip = !inTip;
			}
			dddCounter = 0;
		}

		if (result.back() == '*' && !inCode && !inTip) {
			XingCounter++;
		}
		else {
			if (XingCounter == 2) {
				if (isLeft) {
					if (result.size() > 3 && !isSeparator(result[result.size() - 4])) {
						result.insert(result.size() - 3, " ");
					}
					isLeft = false;
				}
				else {
					if (!isSeparator(result.back())) {
						result.insert(result.size() - 1, " ");
					}
					isLeft = true;
				}
			}
			XingCounter = 0;
		}
	}
	return result;
}

void traverse(directory_iterator trans, string str = " ") {
	while (!trans._At_end()) {
		directory_iterator it = trans;
		if ((*it).is_directory()) {
			try {		//可能访问到系统文件导致权限不够，所以这里需要进行异常处理
				printf("%s \n", it->path().string().c_str());
				traverse(directory_iterator((*it).path()), str + (trans._At_end() ? "    " : "│   "));
			}
			catch (filesystem_error s) {
				continue;
			}
		}
		else if(it->is_regular_file()) {
			const string& ext = it->path().extension().string();
			if (ext == ".md"|| ext == ".MD"|| ext == ".mD" || ext == ".Md") {
				printf("%s \n",it->path().string().c_str());
				std::ifstream fin(it->path());
				std::stringstream buffer;
				buffer << fin.rdbuf();
				const std::string& str = buffer.str();
				fin.close();
				const string& result = repair(str);
				std::ofstream fout(it->path());
				fout<<result;
				fout.close();
			}
		}
		trans++;
	}
}

int main() {
	traverse(directory_iterator("./"));
	return 0;
}