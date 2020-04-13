#ifndef LEETCODE_DEFINITION
#define LEETCODE_DEFINITION

#include <list>

#ifndef NULL
#define NULL 0
#endif

namespace lc {
    
struct ListNode {
public:
    int val;
    ListNode *next;
    ListNode(int x) : val(x), next(NULL) { all_.push_back(this); }
private:
    static std::list<ListNode*> all_;
    friend class MemoryCleaner;
};
std::list<ListNode*> ListNode::all_;

struct TreeNode {
public:
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) { all_.push_back(this); }
private:
    static std::list<TreeNode*> all_;
    friend class MemoryCleaner;
};
std::list<TreeNode*> TreeNode::all_;

} // namespace lc

// for user solution
#define INPUT std::cin // input[istream, string], defalut to cin
#define OUTPUT std::cout // output[ostream, string], defalut to cout
//#define INTERACTION

#include <bits/stdc++.h>
using namespace std;
using namespace lc;

#endif // LEETCODE_DEFINITION