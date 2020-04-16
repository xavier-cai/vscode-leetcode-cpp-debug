#ifndef LEETCODE_IO
#define LEETCODE_IO

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <utility>
#include <tuple>
#include <queue>
#include <functional>

#include "leetcode-definition.h"
#include "leetcode-util.h"

namespace lc {
    
struct Null { } null;
    
class MultiOS {
public:
    template <typename... _ARGS>
    MultiOS(_ARGS&& ...args) {
        Init(args...);
        for (auto& os : oss_) os->precision(6);
    }
    
    ~MultiOS();

    template <typename _T>
    MultiOS& operator << (const _T& v) {
        for (auto& os : oss_) *os << v;
        return *this;
    }
    MultiOS& operator << (std::ostream& (*v)(std::ostream&));

private:
    //Noncopyable
    MultiOS(const MultiOS&);  
    const MultiOS& operator = (const MultiOS&); 
    
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
    
    std::vector<std::ostream*> oss_;
    std::vector<std::ofstream*> ofs_;

}; //class MultiOS

MultiOS::~MultiOS() {
    for (auto& os : oss_) os->flush();
    for (auto& of : ofs_) of->close();
}

void MultiOS::InitOS(std::ostream& os) {
    oss_.push_back(&os);
}

void MultiOS::InitOS(const std::string& fname) {
    ofs_.push_back(new std::ofstream(fname));
    oss_.push_back(ofs_.back());
}

MultiOS& MultiOS::operator << (std::ostream& (*v)(std::ostream&)) {
    for (auto os : oss_) *os << v;
    return *this;
}


//io: single-input multi-output
class SIMO {
public:
    template <typename _IARGS, typename... _OARGS>
    SIMO(_IARGS&& iargs, _OARGS&& ...oargs) :
        is_(NULL),
        ifs_(NULL),
        os_(new MultiOS(oargs...)),
        fromFile_(false),
        line_(1),
        offset_(0),
        level_(0)
    {
        InitIS(iargs);
    }
    
    ~SIMO();
    
    bool ReachEOF() const;
    std::pair<int, std::streampos> Position() const; // line & pos
    bool CheckChar(char ch);
    SIMO& RunInline(std::function<void(SIMO&)> cb);
    
    //default input
    template <typename _T>
    SIMO& operator << (const _T& v) {
        *os_ << v;
        return *this;
    }
    SIMO& operator << (std::ostream& (*v)(std::ostream&)) {
        *os_ << v;
        return *this;
    }
    SIMO& operator << (const bool& v) {
        *os_ << (v ? "true" : "false");
        return *this;
    }
    SIMO& operator << (const std::string& s) {
        *os_ << '"' << s << '"';
        return *this;
    }
    SIMO& operator << (const Null& v) {
        *os_ << "null";
        return *this;
    }
    template <typename _T>
    SIMO& operator << (const std::vector<_T>& v) {
        *os_ << '[';
        for (int i = 0, n = v.size(); i < n; ++i) {
            if (i != 0) *os_ << ',';
            *this << v[i];
        }
        *os_ << ']';
        return *this;
    }
    SIMO& operator << (TreeNode* const &node) {
        std::queue<TreeNode*> Q;
        Q.push(node);
        *os_ << '[';
        bool isFirst = true;
        auto outputComma = [this, &isFirst] {
            if (isFirst) isFirst = false;
            else *os_ << ',';
        };
        int nullCount = 0;
        while (!Q.empty()) {
            TreeNode* cur = Q.front();
            Q.pop();
            if (cur == NULL) ++nullCount;
            else {
                for (; nullCount > 0; --nullCount) { outputComma(); *this << null; } 
                outputComma();
                *os_ << cur->val;
                Q.push(cur->left);
                Q.push(cur->right);
            }
        }
        *os_ << ']';
        return *this;
    }
    
    template <typename _T>
    SIMO& operator >> (_T& v) { 
        return RunInline([&, this](auto& io) {
            *is_ >> v;
        });
    }
    SIMO& operator >> (void (*cb)()) {
        cb();
        return *this;
    }
    SIMO& operator >> (bool& v) {
        return RunInline([&, this](auto& io) {
            static std::string ref1 = "true", ref2 = "false";
            bool flag = true;
            if (!this->CheckChar(ref1.front())) {
                flag = false;
                util::assert_msg(this->CheckChar(ref2.front()), util::join(inputFormatError_, "[bool]."));
            }
            int len = (flag ? ref1 : ref2).size();
            for (int i = 1; i < len; ++i) {
                util::assert_msg(this->CheckChar((flag ? ref1 : ref2)[i]), util::join(inputFormatError_, "[bool]."));
            }
            v = flag;
        });
    }
    SIMO& operator >> (std::string& s) {
        return RunInline([&, this](auto& io) {
            util::assert_msg(this->CheckChar('\"'), util::join(inputFormatError_, "[string]."));
            std::stringstream ss;
            while (true) {
                char ch = is_->get();
                if (ch == '"') break;
                ss << ch;
            }
            s = ss.str();
        });
    }
    SIMO& operator >> (Null& v) {
        return RunInline([&, this](auto& io) {
            static std::string ref = "null";
            for (int i = 0, n = ref.size(); i < n; ++i) {
                util::assert_msg(this->CheckChar(ref[i]), util::join(inputFormatError_, "[null]."));
            }
        });
    }
    template <typename _T>
    SIMO& operator >> (std::vector<_T>& v) {
        return RunInline([&, this](auto& io) {
            v.resize(0);
            util::assert_msg(this->CheckChar('['), util::join(inputFormatError_, "[vector]."));
            if (this->CheckChar(']')) return; //empty vector
            while (true) {
                v.push_back(_T());
                *this >> v.back();
                if (this->CheckChar(']')) break;
                util::assert_msg(this->CheckChar(','), util::join(inputFormatError_, "[vector]."));
            }
        });
    }
    SIMO& operator >> (TreeNode* &node) {
        return RunInline([&, this](auto& io) {
            auto read = [this]() -> TreeNode* {
                char peek = is_->peek();
                if (peek == ']') return NULL;
                if (peek == 'n') { *this >> null; return NULL; }
                int val;
                *this >> val;
                return new TreeNode(val);
            };
            auto readSeparator = [this]() -> bool { // is ']'
                if (this->CheckChar(']')) return true;
                util::assert_msg(this->CheckChar(','), util::join(inputFormatError_, "[TreeNode]."));
                return false;
            };
            util::assert_msg(this->CheckChar('['), util::join(inputFormatError_, "[TreeNode]."));
            node = read();
            if (readSeparator()) return; // empty tree
            std::queue<TreeNode*> Q;
            if (node != NULL) Q.push(node);
            while (!Q.empty()) {
                TreeNode* cur = Q.front();
                Q.pop();
                cur->left = read();
                if (cur->left != NULL) Q.push(cur->left);
                if (readSeparator()) break;
                cur->right = read();
                if (cur->right != NULL) Q.push(cur->right);
                if (readSeparator()) break;
            }
        });
    }
    template <typename THIS, int I, int N, typename... Args>
    struct TupleHelper {
        static void read(THIS& io, std::tuple<Args...>& v) {
            io >> std::get<I>(v);
            util::assert_msg(io.CheckChar(','), util::join(inputFormatError_, "[tuple]."));
            TupleHelper<THIS, I+1, N-1, Args...>::read(io, v);
        }
        static void print(THIS& io, const std::tuple<Args...>& v) {
            io << std::get<I>(v) << ',';
            TupleHelper<THIS, I+1, N-1, Args...>::print(io, v);
        }
    };
    template <typename THIS, int I, typename... Args>
    struct TupleHelper<THIS, I, 1, Args...> {
        static void read(THIS& io, std::tuple<Args...>& v) {
            io >> std::get<I>(v);
        }
        static void print(THIS& io, const std::tuple<Args...>& v) {
            io << std::get<I>(v);
        }
    };
    template <typename... Args>
    SIMO& operator << (const std::tuple<Args...>& v) {
        *os_ << '[';
        TupleHelper<SIMO, 0, sizeof...(Args), Args...>::print(*this, v);
        *os_ << ']';
        return *this;
    }
    template <typename... Args>
    SIMO& operator >> (std::tuple<Args...>& v) {
        return RunInline([&, this](auto& io) {
            util::assert_msg(this->CheckChar('['), util::join(inputFormatError_, "[tuple]."));
            TupleHelper<SIMO, 0, sizeof...(Args), Args...>::read(*this, v);
            util::assert_msg(this->CheckChar(']'), util::join(inputFormatError_, "[tuple]."));
        });
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
    std::istream* is_;
    std::ifstream* ifs_;
    MultiOS* os_;
    bool fromFile_;
    int line_;
    std::streampos offset_;
    int level_;
    static std::string inputFormatError_;
    
    void InitIS(std::istream& is);
    void InitIS(const std::string& fname);
    
}; // class SIMO

std::string SIMO::inputFormatError_("Input format error: ");

SIMO::~SIMO() {
    if (ifs_ != NULL) {
        ifs_->close();
        delete ifs_;
    }
    delete os_;
}

bool SIMO::ReachEOF() const {
    return is_->eof();
}

SIMO& SIMO::RunInline(std::function<void(SIMO&)> cb) {
    ++level_;
    cb(*this);
    if (--level_ == 0) {
        bool cr = CheckChar('\n');
        bool lf = CheckChar('\r');
        bool new_line = cr || lf;
        util::assert_msg(new_line || CheckChar(EOF), util::join(inputFormatError_, "bad end of line."));
        if (fromFile_ && new_line) {
            ++line_;
            offset_ = is_->tellg();
        }
    }
    return *this;
}

std::pair<int, std::streampos> SIMO::Position() const {
    if (!fromFile_) return {0, 0};
    return {line_, is_->tellg() - offset_};
}

bool SIMO::CheckChar(char exp) {
    char read = is_->peek();
    if (read != exp) return false;
    is_->get();
    return true;
}

void SIMO::InitIS(std::istream& is) {
    std::cout << "Input from [std::cin]" << std::endl;
    is_ = &is;
}

void SIMO::InitIS(const std::string& fname) {
    std::cout << "Input from file \"" << fname << "\"" << std::endl;
    is_ = ifs_ = new std::ifstream(fname);
    if (!ifs_->is_open()) util::assert_msg(false, "Can not open the input file.");
    fromFile_ = true;
    is_->peek(); // initialize
    offset_ = is_->tellg();
}
    
} // namespace lc

#endif // LEETCODE_IO