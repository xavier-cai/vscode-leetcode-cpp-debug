#ifndef LEETCODE_IO
#define LEETCODE_IO

#include <string>
#include <iostream>
#include <fstream>
#include <exception>
#include <list>
#include <sstream>
#include <functional>
#include <direct.h>

#include "leetcode-convert.h"
#include "leetcode-types.h"

namespace lc {

namespace conv {

template <>
struct Convert<ListNode*> {
    static void FromJson(ListNode* &v, const json::Json& js) {
        auto obj = js.GetObject<json::JArray>();
        if (obj == NULL) throw ConvertException(js, "Convert failed. ListNode*.");
        int n = obj->GetArray().size();
        if (n <= 0) { v = NULL; return; }
        v = new ListNode(conv::FromJson<int>(obj->GetArray().front()));
        ListNode* current = v;
        for (int i = 1; i < n; ++i) {
            current->next = new ListNode(conv::FromJson<int>(obj->GetArray()[i]));
            current = current->next;
        }
    }

    static json::Json ToJson(ListNode* const &v) {
        json::Json js = json::Json::Create<json::JArray>();
        auto obj = js.GetObject<json::JArray>();
        ListNode* current = v;
        while (current != NULL) {
            obj->GetArray().emplace_back(conv::ToJson<int>(current->val));
            current = current->next;
        }
        return js;
    }
};

template <>
struct Convert<TreeNode*> {
    static void FromJson(TreeNode* &v, const json::Json& js) {
        auto obj = js.GetObject<json::JArray>();
        if (obj == NULL) throw ConvertException(js, "Convert failed. TreeNode*.");
        int n = obj->GetArray().size();
        if (n <= 0) { v = NULL; return; }
        auto read = [&obj, &n](int idx) -> TreeNode* {
            if (idx >= n) return NULL;
            auto& js = obj->GetArray()[idx];
            if (js.Is<json::JNull>()) return NULL;
            return new TreeNode(conv::FromJson<int>(js));
        };
        v = read(0);
        std::queue<TreeNode*> Q;
        if (v != NULL) Q.push(v);
        for (int i = 1; i < n && !Q.empty();) {
            TreeNode* current = Q.front(); Q.pop();
            if ((current->left = read(i++)) != NULL) Q.push(current->left);
            if ((current->right = read(i++)) != NULL) Q.push(current->right);
        }
    }

    static json::Json ToJson(TreeNode* const &v) {
        json::Json js = json::Create<json::JArray>();
        auto obj = js.GetObject<json::JArray>();
        std::queue<TreeNode*> Q;
        if (v != NULL) Q.push(v);
        int nullCount = 0;
        while (!Q.empty() && nullCount < Q.size()) {
            TreeNode* current = Q.front(); Q.pop();
            if (current == NULL) {
                obj->GetArray().emplace_back(json::JNull());
                nullCount -= 1;
                continue;
            }
            obj->GetArray().emplace_back(conv::ToJson<int>(current->val));
            Q.push(current->left);
            Q.push(current->right);
            if (current->left == NULL) nullCount += 1;
            if (current->right == NULL) nullCount += 1;
        }
        return js;
    }
};

} // namespace conv

namespace io {

std::string GetCurrentDirectory() {
    char* buffer = getcwd(NULL, 0);
    if (buffer == NULL) return "";
    std::string cwd(buffer);
    free(buffer);
    if (cwd.length() <= 0 || cwd.back() == '/' || cwd.back() == '\\') return cwd;
    if (cwd.find('\\') != std::string::npos) return cwd + '\\';
    return cwd + '/';
}

class SI {
public:
    SI(const std::string& file);
    SI(std::istream& is);
    ~SI();

    bool Eof() const;
    int GetLineCount() const;
    const std::string& GetRaw() const;
    json::Json GetLine();

    template <typename _T>
    SI& operator >> (_T& v) {
        conv::FromJson(v, GetLine());
        return *this;
    }
    SI& operator >> (std::function<void()> cb) {
        cb();
        return *this;
    }
    SI& operator >> (void (*cb)()) {
        cb();
        return *this;
    }
    
    template <typename _T, typename... _REST>
    void Input(_T& v, _REST& ...r) {
        *this >> v;
        Input(r...);
    }
    template <typename _T>
    void Input(_T& v) {
        *this >> v;
    }

private:
    SI(const SI&) = delete;
    const SI& operator = (const SI&) = delete;

    std::istream* is_;
    bool fromFile_;
    int line_;
    std::string raw_;
}; // class SI

SI::SI(const std::string& file) :
    is_(NULL),
    fromFile_(true),
    line_(0)
{
    std::cout << "Input from file \"" << file << "\"" << std::endl;
    auto ifs = new std::ifstream(file);
    if (!ifs->is_open()) {
        delete ifs;
        throw std::string("Can not open the input file: ")  + GetCurrentDirectory() + file;
    }
    this->is_ = ifs;
}

SI::SI(std::istream& is) :
    is_(&is),
    fromFile_(false),
    line_(0)
{
    std::cout << "Input from [std::cin]" << std::endl;
}

SI::~SI() {
    if (this->fromFile_ && this->is_ != NULL) {
        delete dynamic_cast<std::ifstream*>(this->is_);
    }
}

bool SI::Eof() const {
    return this->is_->eof();
}

int SI::GetLineCount() const {
    return this->line_;
}

const std::string& SI::GetRaw() const {
    return this->raw_;
}

json::Json SI::GetLine() {
    this->line_ += 1;
    std::getline(*this->is_, this->raw_);
    return json::Json(this->raw_);
}



class MO {
public:
    template <typename... _ARGS>
    MO(_ARGS&& ...args) {
        this->Init(args...);
    }
    ~MO();

    template <typename _T>
    MO& operator << (const _T& v) {
        auto js = conv::ToJson(v);
        std::stringstream ss;
        ss << js;
        std::string s = ss.str();
        for (auto& os : this->oss_) *os << s;
        return *this;
    }
    MO& operator << (std::ostream& (*v)(std::ostream&));

private:
    MO(const MO&) = delete;  
    const MO& operator = (const MO&) = delete;

    void InitOS(std::ostream& os);
    void InitOS(const std::string& fname);

    template <typename _T, typename... _REST>
    void Init(_T&& arg, _REST&& ...r) {
        InitOS(arg);
        Init(r...);
    }
    
    template <typename _T>
    void Init(_T&& arg) {
        InitOS(arg);
    }
    
    std::list<std::ostream*> oss_;
    std::list<std::ofstream*> ofs_;
}; // class MO

MO::~MO() {
    for (auto& os : this->oss_) os->flush();
    for (auto& of : this->ofs_) of->close(), delete of;
}

void MO::InitOS(std::ostream& os) {
    this->oss_.push_back(&os);
}

void MO::InitOS(const std::string& fname) {
    auto ofs = new std::ofstream(fname);
    if (!ofs->is_open()) {
        delete ofs;
        throw std::string("Can not open the output file: ") + GetCurrentDirectory() + fname;
    }
    this->ofs_.push_back(ofs);
    this->oss_.push_back(ofs);
}

MO& MO::operator << (std::ostream& (*v)(std::ostream&)) {
    for (auto os : this->oss_) *os << v;
    return *this;
}

} // namespace io

} // namespace lc

#endif