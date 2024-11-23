#include <filesystem>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>

using namespace std;
using namespace std::filesystem;

bool isSeparator(const char& ch) {
	return ch == '\n' || ch == ' ' || ch == '\t';
}

bool containsSubstring(const std::string& str, const std::string& target) {
	return str.find(target) != std::string::npos;
}

bool startsWithRegex(const std::string& str, const std::regex& pattern) {
	std::smatch match;
	if (std::regex_search(str, match, pattern)) {
		return match.position() == 0;
	}
	return false;
}

void replaceAll(std::string& str, const std::string& old_str, const std::string& new_str) {
	size_t pos = 0;
	while ((pos = str.find(old_str, pos)) != std::string::npos) {
		str.replace(pos, old_str.length(), new_str);
		pos += new_str.length();
	}
}

std::string extractMetaData(const std::string& input) {
	std::regex pattern("---\n((?:.|\n)*?)---\n", std::regex_constants::ECMAScript );
	std::sregex_iterator it(input.begin(), input.end(), pattern);
	std::sregex_iterator end;
	if (it != end) {
		return it->str(1);
	}
	return "";
}

std::vector<std::string> extractVideoUrls(const std::string& input) {
	std::vector<std::string> result;
	std::regex pattern("<\\w+ .*</\\w+>", std::regex_constants::ECMAScript | std::regex::optimize );
	std::sregex_iterator it(input.begin(), input.end(), pattern);
	std::sregex_iterator end;
	while (it != end) {
		result.push_back(it->str());
		++it;
	}
	return result;
}

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <iostream>
#include <regex>
#include <string>
#include <vector>

// 用于从匹配到的<iframe>字符串中提取src的值，并根据条件处理后返回标准化后的<iframe>字符串
std::string processIframeMatch(const std::string& matchStr) {
	// 从匹配到的字符串中提取src的值
	std::regex srcRegex("src=(['\"])([^'\"]+)\\1|src=([^'\"\\s>]+)");
	std::smatch srcMatch;
	if (std::regex_search(matchStr, srcMatch, srcRegex)) {
		std::string srcValue;
		if (!srcMatch[2].str().empty()) {
			// 匹配到带引号的src属性值
			srcValue = srcMatch[2].str();
		}
		else if (!srcMatch[3].str().empty()) {
			// 匹配到不带引号的src属性值
			srcValue = srcMatch[3].str();
		}

		if (srcValue.find("bilibili") != std::string::npos && srcValue.find("&autoplay=false") == std::string::npos) {
			// 如果src值中包含bilibili且不存在&autoplay=false，补充&autoplay=false
			if (srcValue.find('?') != std::string::npos) {
				srcValue += "&autoplay=false";
			}
			else {
				srcValue += "?autoplay=false";
			}
		}

		return "<p style=\"text-align:center\"><iframe width=\"560\" height=\"315\" src=\"" + srcValue + "\" scrolling=\"no\" border=\"0\" frameborder=\"0\" framespacing=\"0\" allowfullscreen=\"true\"></iframe></p>";
	}

	return matchStr;
}

std::string standardizeIframeTags(const std::string& input) {
	// 定义正则表达式，用于匹配各种形式的<iframe>标签所在的整行文本
	std::regex iframeRegex("^.*<iframe\\s+[^>]*>\\s*</iframe>.*$");

	// 用于存储匹配结果的向量
	std::vector<std::string> matches;

	// 进行正则表达式匹配，将所有匹配到的包含<iframe>标签的整行文本存储到matches向量中
	std::sregex_iterator it(input.begin(), input.end(), iframeRegex);
	std::sregex_iterator end;
	for (; it != end; ++it) {
		matches.push_back(it->str());
	}

	// 用处理后的<iframe>字符串替换原字符串中的相应部分
	std::string output = input;
	for (const auto& match : matches) {
		// 检查是否已经是完整的标准化格式，如果不是则进行替换
		if (!std::regex_search(match, std::regex("^<p style=\"text-align:center\"><iframe\\s+[^>]*>\\s*</iframe></p>$"))) {
			std::string replacement = processIframeMatch(match);
			size_t pos = output.find(match);
			if (pos != std::string::npos) {
				output.replace(pos, match.length(), replacement);
			}
		}
	}

	return output;
}

std::string repair(const std::string& content) {
	std::string result;
	result.reserve(content.size());
	bool isLeft = true;
	bool inCode = false;
	bool inTip = false;
	int XingCounter = 0;
	int dddCounter = 0;
	for (int i = 0; i < content.size(); i++) {
		result = result + content[i];
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

	std::string metaData = extractMetaData(result);
	if (metaData.empty()) {
		result = "---\ncomments: true\n---\n" + result;
	}
	else if (!containsSubstring(metaData, "comments: true")) {
		replaceAll(result, metaData, metaData + "comments: true\n");
	}

	result = standardizeIframeTags(result);

	//for (const std::string& videoUrl : extractVideoUrls(result)) {
	//	std::cout << videoUrl <<std::endl;
	//}
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
		else if (it->is_regular_file()) {
			const string& ext = it->path().extension().string();
			if (ext == ".md" || ext == ".MD" || ext == ".mD" || ext == ".Md") {
				printf("%s \n", it->path().string().c_str());
				std::ifstream fin(it->path());
				std::stringstream buffer;
				buffer << fin.rdbuf();
				const std::string& str = buffer.str();
				fin.close();
				const string& result = repair(str);
				std::ofstream fout(it->path());
				fout << result;
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