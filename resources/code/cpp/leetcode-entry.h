#ifndef LEETCODE_ENTRY
#define LEETCODE_ENTRY

#include <vector>
#include <string>
#include <list>

#include "leetcode-handler.h"

namespace lc {
    
class MemoryCleaner {
public:
    static void Clean();
    
private:
    template<typename _T>
    static void Clean(std::list<_T*>& all) {
        for (auto& p : all)
            delete p;
        all.clear();
    }

};

void MemoryCleaner::Clean() {
    Clean(TreeNode::all_);
    Clean(ListNode::all_); 
}

class Entry {
public:
    static void Run(SIMO& io);
    
private:
    static void RunAlgorithm(SIMO& io);
    static void RunSystemDesign(SIMO& io);
};

void Entry::Run(SIMO& io) {
    while (!io.ReachEOF()) {
#ifdef INTERACTION
        io.Input(INTERACTION);
#endif
#ifdef SYSTEM_DESIGN 
        RunSystemDesign(io);
#else
        RunAlgorithm(io);
#endif
        try {
            MemoryCleaner::Clean();
        }
        catch (...) {
            util::assert_msg(false, "Please only use pointer of [TreeNode] & [ListNode] and do not use [delete].");
        }
    }
}

void Entry::RunAlgorithm(SIMO& io) {
    Handler handler(io);
    handler.Handle(io);
}

void Entry::RunSystemDesign(SIMO& io) {
    static std::string inputFormatError = "Input format error: ";
    std::vector<std::string> functions;
    io >> functions;
    auto check = [&io] (char expect) -> void { 
        util::assert_msg(io.CheckChar(expect),
            util::join(inputFormatError, "design problem.",
            "expect=\'", expect, "\'."));
    };
    if (functions.size() == 0) {
        check('[');
        check(']');
        io << "[]" << std::endl;
        return;
    }
    io.RunInline([&](SIMO& simo) {
        check('[');
        io << '[';
        util::assert_msg(functions.front() == Handler::GetClassName(),
            util::join(inputFormatError, "the first function should be constructor."));
        Handler handler(io);
        io << null;
        for (int i = 1, n = functions.size(); i < n; ++i) {
            check(',');
            io << ',';
            handler.Handle(io, functions[i]);
        }
        check(']');
        io << ']' << std::endl;
    });
}

} // namespace lc

#endif // LEETCODE_ENTRY