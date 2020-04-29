#ifndef LEETCODE_ENTRY
#define LEETCODE_ENTRY

#include <vector>
#include <string>
#include <list>
#include <exception>

#include "leetcode-handler.h"

namespace lc {

struct EntryException : public std::exception {
public:
    EntryException(const std::string& raw, int line, int pos, const std::string& info);
    const std::string& GetRaw() const;
    int GetLine() const;
    int GetPosition() const;
    const char* what() const throw ();

private:
    std::string raw_;
    int line_;
    int pos_;
    std::string info_;
}; // struct EntryException

EntryException::EntryException(const std::string& raw, int line, int pos, const std::string& info) :
    raw_(raw),
    line_(line),
    pos_(pos),
    info_("Entry error. ")
{
    this->info_ += info;
}

const std::string& EntryException::GetRaw() const {
    return this->raw_;
}

int EntryException::GetLine() const {
    return this->line_ ;
}

int EntryException::GetPosition() const {
    return this->pos_ ;
}

const char* EntryException::what() const throw () {
    return this->info_.c_str();
}




class Entry {
public:
    static void Run(io::SI& in, io::MO& out);
    
private:
    static void RunAlgorithm(io::SI& in, io::MO& out);
    static void RunSystemDesign(io::SI& in, io::MO& out);
};

void Entry::Run(io::SI& in, io::MO& out) {
    while (!in.Eof()) {
        try {
#ifdef INTERACTION
        in.Input(INTERACTION);
#endif
#ifdef SYSTEM_DESIGN 
        RunSystemDesign(in, out);
#else
        RunAlgorithm(in, out);
#endif
        }
        catch (io::EofException) {}

        try {
            mem::MemoryCleaner::Clean();
        }
        catch (...) {
            throw std::string("Please only use pointer of [TreeNode] & [ListNode] and do not use [delete].");
        }
    }
}

void Entry::RunAlgorithm(io::SI& in, io::MO& out) {
    auto dummy = json::ObjectNull();
    Handler handler(dummy);
    handler.Handle(in, out);
}

void Entry::RunSystemDesign(io::SI& in, io::MO& out) {
    json::Json fobj(in.GetLine());
    int fline = in.GetLineCount();
    std::string fraw = in.GetRaw();
    auto quick_throw = [&fline, &fraw, &fobj](int idx, const std::string& info) {
        throw EntryException(fraw, fline, fobj.GetObject<json::ObjectArray>()->GetArray()[idx].GetObject()->GetPosistion(), info);
    };

    //get functions
    std::vector<std::string> functions;
    conv::FromJson(functions, fobj);
    int n = functions.size();

    //get args
    json::Json obj(in.GetLine());
    auto args = obj.GetObject<json::ObjectArray>();
    if (args == NULL) throw conv::ConvertException(obj, "Input format error.");
    if (n != args->GetArray().size()) throw EntryException(in.GetRaw(), in.GetLineCount(), args->GetPosistion(), "Number of functions and arguments not matched.");

    //call functions
    json::ObjectArray ret;
    if (n > 0) {
        if (functions.front() != Handler::GetClassName()) {
            quick_throw(0, "The first function need be the constructor.");
        }
        Handler handler(args->GetArray().front());
        ret.GetArray().emplace_back(json::ObjectNull());
        for (int i = 1; i < n; ++i) {
            try {
                ret.GetArray().emplace_back(handler.Handle(args->GetArray()[i], functions[i]));
            }
            catch (std::string& e) {
                quick_throw(i, e);
            }
        }
    }
    out << ret << std::endl;
}

} // namespace lc

#endif // LEETCODE_ENTRY