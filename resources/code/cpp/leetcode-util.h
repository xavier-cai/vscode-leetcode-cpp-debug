#ifndef LEETCODE_UTIL
#define LEETCODE_UTIL

#include <exception>
#include <string>
#include <sstream>
#include <ostream>

namespace lc {

struct StringException : public std::exception {
public:
    StringException(const char* err) : err_(err) { }
    StringException(const std::string& err) : err_(err) { }
    const char* what () const throw () { return err_.c_str(); }

private:
    std::string err_;

};

} // namespace lc

#include <sstream>

namespace lc {

class util {
public:
    template <typename... _T>
    static std::string join(const _T& ...v) {
        std::stringstream ss;
        join_impl(ss, v...);
        return ss.str();
    }

    static void assert_msg_impl(int line, bool condition, const std::string& msg);

private:
    util() {}
    
    template <typename _T>
    static void join_impl(std::stringstream& ss, const _T& v) {
        ss << v;
    }
    
    template <typename _T, typename... _REST>
    static void join_impl(std::stringstream& ss, const _T& v, const _REST& ...r) {
        ss << v;
        join_impl(ss, r...);
    }

}; // class util

void util::assert_msg_impl(int line, bool condition, const std::string& msg) {
    if (!condition) {
        std::stringstream ss;
        ss << msg << (msg.length() > 0 ? " " : "");
        ss << "Code line: " << line;
        ss.flush();
        throw StringException(ss.str());
    }
}

#define assert_msg(...) assert_msg_impl(__LINE__, __VA_ARGS__)

} // namespace lc

#endif // LEETCODE_UTIL