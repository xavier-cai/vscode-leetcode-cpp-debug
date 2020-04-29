#ifndef LEETCODE_TYPES
#define LEETCODE_TYPES

#include <list>
#include <queue>

#ifndef NULL
#define NULL 0
#endif

namespace lc {

namespace mem {

class PtrObject {
public:
    virtual ~PtrObject() {};

protected:
    PtrObject();
}; // class PtrObject

class MemoryCleaner {
public:
    static void Add(PtrObject* obj);
    static void Clean();
    MemoryCleaner(MemoryCleaner&) = delete;
    MemoryCleaner& operator=(const MemoryCleaner&) = delete;

private:
    MemoryCleaner();
    std::list<PtrObject*> objs_;

    static MemoryCleaner instance_;
}; // class MemoryCleaner

MemoryCleaner MemoryCleaner::instance_;

MemoryCleaner::MemoryCleaner() {}

void MemoryCleaner::Add(PtrObject* obj) {
    instance_.objs_.push_back(obj);
}

void MemoryCleaner::Clean() {
    for (auto& obj : instance_.objs_) {
        delete obj;
    }
    instance_.objs_.clear();
}

PtrObject::PtrObject() {
    MemoryCleaner::Add(this);
}

} // namespace mem



class ListNode : public mem::PtrObject {
public:
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL) {}
};

struct TreeNode : public mem::PtrObject {
public:
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

} // namespace lc

#endif