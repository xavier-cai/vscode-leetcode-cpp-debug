#include <algorithm>

#include "leetcode-entry.h"

#ifndef INPUT
#define INPUT std::cin
#endif

#ifndef OUTPUT
#define OUTPUT std::cout
#endif

std::string print_error(const std::string& raw, int line, int pos, const std::string& info);

int main() {
    try {
        lc::io::SI in(INPUT);
        lc::io::MO out(OUTPUT);
        try {
            lc::Entry::Run(in, out);
            std::cout << "Program compelete." << std::endl;
        }
        catch (lc::json::JsonException& e) { throw print_error(in.GetRaw(), in.GetLineCount(), e.GetPosition(), e.what()); }
        catch (lc::conv::ConvertException& e) {
            if (e.GetJson().GetObject() == NULL) throw std::string(e.what());
            throw print_error(in.GetRaw(), in.GetLineCount(), e.GetJson().GetObject()->GetPosistion(), e.what());
        }
        catch (lc::EntryException& e) {
            throw print_error(e.GetRaw(), e.GetLine(), e.GetPosition(), e.what());
        }
        catch (std::string& e) { throw e; }
        catch (std::exception& e) { throw std::string("Unhandled error. ") + e.what(); }
        catch (...) { throw std::string("Unhandled error."); }
    }
    catch (std::string& e) {
        std::cerr << "\nError: " << e << std::endl;
        // pause here in terminal
        std::cout << "Press Any Key to Continue..." << std::endl;
        std::cin.sync(); // clear
        std::cin.get(); // pause
    }
    return 0;
}

std::string print_error(const std::string& raw, int line, int pos, const std::string& info) {
    const int lmost = 15, rmost = 15, padding = 6;
    std::stringstream ss;
    ss << info;
    if (pos < 0 || line <= 0) return ss.str();
    
    int n = raw.size();
    int l = std::max(0, pos - lmost);
    if (l <= padding) l = 0;
    int r = std::min(n, pos + rmost);
    if (r + padding >= n) r = n;

    ss << " @position=" << line << ':' << pos << '\n';
    int count = ss.str().size();
    ss << "raw string : ";
    if (l > 0) ss << "(..." << l << ")";
    count = ss.str().size() - count + pos - l;
    ss << raw.substr(l, r - l);
    if (r < n) ss << "(" << n - r << "...)";
    ss << "\n";
    while (count-- > 0) ss << ' ';
    ss << "^";

    return ss.str();
}