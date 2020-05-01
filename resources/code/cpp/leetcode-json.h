#ifndef LEETCODE_JSON_H
#define LEETCODE_JSON_H

#include <string>
#include <exception>
#include <sstream>
#include <ostream>
#include <vector>
#include <map>
#include <list>
#include <cstring>
#include <typeinfo>
#include <functional>
#include <cassert>

#ifdef __GNUC__
#include <cxxabi.h>
#include <cstdlib>
#endif

#ifndef NULL
#define NULL 0
#endif

namespace lc {

namespace clone {

class Base {
public:
    virtual ~Base() {}
    virtual Base* DoClone() const = 0;

protected:
    virtual void DoCopyTo(Base* clone) const {}
}; // class Base

template<typename _THIS, typename _BASE = Base>
class Template : public _BASE {
public:
    Base* DoClone() const override {
        if (dynamic_cast<const Base*>(this) == NULL) return NULL;
        if (dynamic_cast<const _THIS*>(this) == NULL) return NULL;
        const _THIS* me = dynamic_cast<const _THIS*>(this);
        _THIS* clone = new _THIS(*me);
        this->DoCopyTo(dynamic_cast<Base*>(clone));
        return dynamic_cast<Base*>(clone);
    }

    _THIS* Clone() const {
        return dynamic_cast<_THIS*>(DoClone());
    }

protected:
    virtual void DoCopy(_THIS* clnoe) const {}
    virtual void DoCopyTo(Base* clone) const override {
        _BASE::DoCopyTo(clone);
        this->DoCopy(dynamic_cast<_THIS*>(clone));
    }
}; // class Template

} // namespace clon

namespace json {

struct JsonMask {
public:
    JsonMask(int mask);
    ~JsonMask();
    JsonMask* Next(int mask);
    JsonMask* Set(int mask);
    int Mask() const;
    JsonMask* Next() const;

private:
    int mask_;
    JsonMask* next_;
}; // struct JsonMask

JsonMask::JsonMask(int mask) : mask_(mask), next_(NULL) {}

JsonMask::~JsonMask() {
    if (this->next_ != NULL) delete this->next_;
}

JsonMask* JsonMask::Next(int mask) {
    if (this->next_ == NULL) this->next_ = new JsonMask(mask);
    else this->next_->Set(mask);
    return this->next_;
}

JsonMask* JsonMask::Set(int mask) {
    this->mask_ = mask;
    return this;
}

int JsonMask::Mask() const {
    return this->mask_;
}

JsonMask* JsonMask::Next() const {
    return this->next_;
}



class Object : public clone::Base {
public:
    virtual ~Object() {}

    template<typename _T>
    bool Is() const {
        return dynamic_cast<const _T*>(this) != NULL;
    }
    const std::string& GetName() const;
    int GetPosistion() const;
    void SetPosition(int pos);
    int Parse(const std::string& raw, int start = 0, JsonMask* mask = NULL);

    friend std::ostream& operator << (std::ostream& os, const Object& obj);

protected:
    Object();
    virtual const std::string& Name() const = 0;
    //return length
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) = 0;
    virtual void Print(std::ostream& os) const = 0;

    //return length
    int SkipSpaces(const std::string& raw, int start, bool error = true);
    static int SkipSpacesWithoutError(const std::string& raw, int start);

private:
    int pos_;
}; // class Object



template <typename _T>
struct _is_object : public std::is_base_of<json::Object, _T> {};

template <typename _T>
std::string _get_name() {
    std::string name(typeid(_T).name());
#ifdef __GNUC__
    char* real = abi::__cxa_demangle(name.c_str(), NULL, NULL, NULL);
    name = std::string(real);
    free(real);
#endif
    return name;
}



class ObjectManager {
public:
    typedef std::function<Object*()> Constructor;
    typedef std::function<bool(const std::string&, int)> Checker;
    struct Api {
        int mask;
        Constructor constructor;
        Checker checker;
    }; // struct ObjectApi

    template <typename _T>
    static void RegistObject(Checker checker, int id) {
        GetInstance().apis_[typeid(_T).hash_code()] = {
            1 << id,
            []() { return new _T(); },
            checker
        };
    }

    template <typename _T>
    static const Api& GetApi() {
        auto find = GetInstance().apis_.find(typeid(_T).hash_code());
        if (find == GetInstance().apis_.end()) throw std::string("Unregisted type [") + _get_name<_T>() + "].";
        return find->second;
    }

    static void Until(std::function<bool(const Api&)> cb) {
        for (auto& ite : GetInstance().apis_) {
            if (cb(ite.second)) break;
        }
    }

private:
    std::map<std::size_t, Api> apis_;

    ObjectManager() {}
    static ObjectManager& GetInstance();
}; // class JsonManager

ObjectManager& ObjectManager::GetInstance() {
    static ObjectManager instance;
    return instance;
}

#define REGIST_JSON_OBJECT(type, checker, id) \
struct _##type##_ObjectRegister { \
    _##type##_ObjectRegister() { \
        static_assert(std::is_base_of<Object, type>::value, "Only json::Object can be registed."); \
        json::ObjectManager::RegistObject<type>(checker, id); \
    } \
} _##type##_ObjectRegister_trigger_instance



class Json {
public:
    Json();
    Json(const std::string& raw, int start = 0, JsonMask* mask = NULL, bool all = true);
    Json(const std::string& raw, int start, int& len, JsonMask* mask = NULL, bool all = true);
    Json(const Object& obj);
    Json(const Json& js);
    Json(Json&& js);
    Json& operator = (const Json& js); 
    ~Json();

    template <typename _T, typename... _ARGS>
    static Json Create(_ARGS&&... args) {
        static_assert(_is_object<_T>::value, "Type need be Object.");
        Json js;
        js.obj_ = new _T(args...);
        return js;
    }

    template <typename _T>
    bool Is() const {
        return this->obj_ != NULL && this->obj_->Is<_T>();
    }
    Object* GetObject() const;
    template <typename _T>
    _T* GetObject() const { return dynamic_cast<_T*>(this->obj_); }
    template <typename _T>
    static int GetMask() {
        return ObjectManager::GetApi<_T>().mask;
    }
    void Swap(Json& js);

    int Parse(const std::string& raw, int start = 0, JsonMask* mask = NULL, bool all = true);

    friend std::ostream& operator << (std::ostream& os, const Json& js);

private:
    Object* obj_;
}; // class Json

struct JsonException : public std::exception {
public:
    JsonException(const Json& obj, const std::string& raw, int pos, const std::string& info = "");
    JsonException(const Json& obj, const std::string& info);
    const Json& GetJson() const;
    const std::string& GetRaw() const;
    int GetPosition() const;
    const char* what() const throw ();

private:
    std::string info_;
    Json obj_;
    std::string raw_;
    int pos_;
};

JsonException::JsonException(const Json& obj, const std::string& raw, int pos, const std::string& info) :
    info_("JSON parse error. "),
    obj_(obj),
    raw_(raw),
    pos_(pos)
{
    this->info_ += info;
}

JsonException::JsonException(const Json& obj, const std::string& info) :
    info_("JSON parse error. "),
    obj_(obj),
    raw_(""),
    pos_(-1)
{
    this->info_ += info;
}

const Json& JsonException::GetJson() const {
    return this->obj_;
}

const std::string& JsonException::GetRaw() const {
    return this->raw_;
}

int JsonException::GetPosition() const {
    return this->pos_;
}

const char* JsonException::what() const throw() {
    return this->info_.c_str();
}


// implements after JsonException
Object::Object() : pos_(-1) {}

const std::string& Object::GetName() const {
    return this->Name();
}

int Object::GetPosistion() const {
    return this->pos_;
}

void Object::SetPosition(int pos) {
    this->pos_ = pos;
}

int Object::Parse(const std::string& raw, int start, JsonMask* mask) {
    return this->DoParse(raw, start, mask);
}

int Object::SkipSpaces(const std::string& raw, int start, bool error) {
    int idx = start, n = raw.size();
    while (idx < n && raw[idx] == ' ') ++idx;
    if (error && idx >= n) throw JsonException(*this, raw, idx);
    return idx - start;
}

int Object::SkipSpacesWithoutError(const std::string& raw, int start) {
    int idx = start, n = raw.size();
    while (idx < n && raw[idx] == ' ') ++idx;
    return idx - start;
}

std::ostream& operator << (std::ostream& os, const Object& obj) {
    obj.Print(os);
    return os;
}



// implements after JsonException
Json::Json() : obj_(NULL) {}

Json::Json(const std::string& raw, int start, JsonMask* mask, bool all) : 
    obj_(NULL)
{
    this->Parse(raw, start, mask, all);
}

Json::Json(const std::string& raw, int start, int& len, JsonMask* mask, bool all) :
    obj_(NULL)
{
    len = this->Parse(raw, start, mask, all);
}

Json::Json(const Object& obj) : obj_(dynamic_cast<Object*>(obj.DoClone())) {}

Json::Json(const Json& js) : obj_(NULL) { *this = js; }

Json::Json(Json&& js) : obj_(NULL) { this->Swap(js); }

Json& Json::operator = (const Json& js) {
    if (this->obj_ != NULL) delete this->obj_;
    if (js.GetObject() != NULL) this->obj_ = dynamic_cast<Object*>(js.GetObject()->DoClone());
    else this->obj_ = NULL;
    return *this;
}

Json::~Json() {
    if (this->obj_ != NULL) delete this->obj_;
}

Object* Json::GetObject() const {
    return this->obj_;
}

void Json::Swap(Json& js) {
    std::swap(this->obj_, js.obj_);
}

std::ostream& operator << (std::ostream& os, const Json& js) {
    os << *js.obj_;
    return os;
}



class JNull : public clone::Template<JNull, Object> {
public:
    JNull();

    static bool QuickCheck(const std::string& raw, int start);

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    static const std::string text_;
}; // class JNull

REGIST_JSON_OBJECT(JNull, JNull::QuickCheck, 0);

const std::string JNull::text_("null");

JNull::JNull() {}

bool JNull::QuickCheck(const std::string& raw, int start) {
    int idx = start + SkipSpacesWithoutError(raw, start);
    int n = raw.size(), m = text_.size();
    if (idx + m > n) return false;
    return std::strncmp(text_.c_str(), raw.c_str() + idx, m) == 0;
}

const std::string& JNull::Name() const {
    static std::string name("Null");
    return name;
}

int JNull::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + SkipSpaces(raw, start);
    if (!QuickCheck(raw, idx)) {
        throw JsonException(*this, raw, idx);
    }
    return this->text_.size();
}

void JNull::Print(std::ostream& os) const {
    os << this->text_;
}



class JBoolean : public clone::Template<JBoolean, Object> {
public:
    JBoolean();
    JBoolean(bool value);
    bool GetValue() const;
    //<0: not matched, 0: false, >0: true
    static int QuickCheck(const std::string& raw, int start);

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    bool value_;

    static const std::string true_;
    static const std::string false_;
}; // class JNull

#define OBJECT_BOOLEAN_CHECKER [](const std::string& raw, int start) { return JBoolean::QuickCheck(raw, start) >= 0; }
REGIST_JSON_OBJECT(JBoolean, OBJECT_BOOLEAN_CHECKER, 1);
#undef OBJECT_BOOLEAN_CHECKER

const std::string JBoolean::true_("true");
const std::string JBoolean::false_("false");

int JBoolean::QuickCheck(const std::string& raw, int start) {
    int idx = start + SkipSpacesWithoutError(raw, start);
    int n = raw.size(), mt = true_.size(), mf = false_.size();
    auto check = [&](const std::string& str) -> bool {
        int m = str.size();
        if (idx + m > n) return false;
        return std::strncmp(str.c_str(), raw.c_str() + idx, m) == 0;
    };
    if (check(true_)) return 1;
    if (check(false_)) return 0;
    return -1;
}

JBoolean::JBoolean() : value_(false) {}

JBoolean::JBoolean(bool value) : value_(value) {}

bool JBoolean::GetValue() const {
    return this->value_;
}

const std::string& JBoolean::Name() const {
    static std::string name("Boolean");
    return name;
}

int JBoolean::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + this->SkipSpaces(raw, start);
    int ret = this->QuickCheck(raw, idx);
    if (ret < 0) throw JsonException(*this, raw, idx);
    if (ret > 0) {
        this->value_ = true;
        return this->true_.size();
    }
    this->value_ = false;
    return this->false_.size();
}

void JBoolean::Print(std::ostream& os) const {
    os << (this->value_ ? this->true_ : this->false_);
}



class JNumber : public clone::Template<JNumber, Object> {
public:
    JNumber();
    JNumber(int v);
    JNumber(double v);
    int GetInteger() const;
    double GetNumber() const;
    bool IsInteger() const;
    void SetValue(int v);
    void SetValue(double v);

    static bool IsNumberCharacter(char c);

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    int integer_;
    double number_;
    bool isInteger_;
}; // class JNumber

#define OBJECT_NUMBER_CHECKER [](const std::string& raw, int start) { return JNumber::IsNumberCharacter(raw[start]); }
REGIST_JSON_OBJECT(JNumber, OBJECT_NUMBER_CHECKER, 2);
#undef OBJECT_NUMBER_CHECKER

bool JNumber::IsNumberCharacter(char c) {
    return (c >= '0' && c <= '9')
        || c == '-'
        || c == '.';
}

JNumber::JNumber() :
    integer_(0),
    number_(0.0),
    isInteger_(true)
{}

JNumber::JNumber(int v) {
    this->SetValue(v);
}

JNumber::JNumber(double v) {
    this->SetValue(v);
}

int JNumber::GetInteger() const {
    return this->integer_;
}

double JNumber::GetNumber() const {
    return this->number_;
}

bool JNumber::IsInteger() const {
    return this->isInteger_;
}

void JNumber::SetValue(int v) {
    this->number_ = this->integer_ = v;
    this->isInteger_ = true;
}

void JNumber::SetValue(double v) {
    this->integer_ = this->number_ = v;
    this->isInteger_ = false;
}

const std::string& JNumber::Name() const {
    static std::string name("Number");
    return name;
}

int JNumber::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    std::stringstream ss1, ss2;
    int from = idx;
    while (idx < n && this->IsNumberCharacter(raw[idx])) {
        ss1 << raw[idx];
        ss2 << raw[idx];
        ++idx;
    }
    if (idx == from) throw JsonException(*this, raw, idx);
    std::string post1, post2;
    ss1 >> this->integer_ >> post1;
    ss2 >> this->number_ >> post2;
    if (post2.size() > 0) throw JsonException(*this, raw, idx - post2.size());
    this->isInteger_ = post1.size() <= 0;
    return idx - start;
}

void JNumber::Print(std::ostream& os) const {
    if (this->isInteger_) os << this->integer_;
    else os << this->number_;
}



//TODO: escape not supported yet
class JString : public clone::Template<JString, Object> {
public:
    JString();
    JString(const std::string& s);
    JString(std::string&& s);
    const std::string& GetString() const;
    std::string& GetString();
    void SetString(const std::string& s);

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    std::string string_;
}; // class JString

#define OBJECT_STRING_CHECKER [](const std::string& raw, int start) { return raw[start] == '\"'; }
REGIST_JSON_OBJECT(JString, OBJECT_STRING_CHECKER, 3);
#undef OBJECT_STRING_CHECKER

JString::JString() {}

JString::JString(const std::string& s) : string_(s) {}

JString::JString(std::string&& s) {
    this->string_.swap(s);
}

const std::string& JString::GetString() const {
    return this->string_;
}

std::string& JString::GetString() {
    return this->string_;
}

void JString::SetString(const std::string& s) {
    this->string_ = s;
}

const std::string& JString::Name() const {
    static std::string name("String");
    return name;
}

int JString::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '\"') throw JsonException(*this, raw, idx);
    std::stringstream ss;
    for (++idx; idx < n && raw[idx] != '\"'; ++idx) ss << raw[idx];
    if (idx >= n) throw JsonException(*this, raw, idx);
    this->string_ = ss.str();
    return ++idx - start;
}

void JString::Print(std::ostream& os) const {
    os << '\"' << this->string_ << '\"';
}



class JArray : public clone::Template<JArray, Object> {
public:
    typedef std::vector<Json> Array;
    JArray();
    Array& GetArray();
    const Array& GetArray() const;

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    Array array_;
}; // class JString

#define OBJECT_ARRAY_CHECKER [](const std::string& raw, int start) { return raw[start] == '['; }
REGIST_JSON_OBJECT(JArray, OBJECT_ARRAY_CHECKER, 4);
#undef OBJECT_ARRAY_CHECKER

JArray::JArray() {}

JArray::Array& JArray::GetArray() {
    return this->array_;
}

const JArray::Array& JArray::GetArray() const {
    return this->array_;
}

const std::string& JArray::Name() const {
    static std::string name("Array");
    return name;
}

int JArray::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '[') throw JsonException(*this, raw, idx);
    idx += this->SkipSpaces(raw, idx + 1) + 1;
    if (raw[idx] == ']') return ++idx - start;
    for (; idx < n; ++idx) {
        int len;
        this->array_.emplace_back(raw, idx, len, mask, false);
        idx += len;
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] != ',' && raw[idx] != ']') throw JsonException(*this, raw, idx);
        if (raw[idx] == ']') return ++idx - start;
    }
    throw JsonException(*this, raw, idx);
    return idx - start;
}

void JArray::Print(std::ostream& os) const {
    os << '[';
    for (int i = 0, n = this->array_.size(); i < n; ++i) {
        if (i != 0) os << ',';
        os << this->array_[i];
    }
    os << ']';
}



class JDict : public clone::Template<JDict, Object> {
public:
    JDict();
    JDict(JDict&& dict);
    JDict(const JDict& dict);
    JDict& operator = (const JDict& dict);

    template <typename _T>
    bool Have(const _T& key) {
        return this->keys_.find(ToString(key)) != this->keys_.end();
    }
    template <typename _T>
    bool Remove(const _T& key) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find == this->keys_.end()) return false;
        this->vals_.erase(find->second);
        this->keys_.erase(find);
        return true;
    }
    template <typename _T>
    bool Add(_T&& key, Json&& v = JNull()) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find != this->keys_.end()) return false;
        JString jkey(k);
        this->keys_.emplace(
            std::move(k),
            this->vals_.emplace(this->vals_.end(),
                std::move(jkey),
                std::forward<Json>(v)
            )
        );
        return true;
    }
    bool Add(Json&& key, Json&& v = JNull()) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find != this->keys_.end()) return false;
        this->keys_.emplace(
            std::move(k),
            this->vals_.emplace(this->vals_.end(),
                std::forward<Json>(key),
                std::forward<Json>(v)
            )
        );
        return true;
    }
    template <typename _T>
    Json& operator [] (const _T& key) {
        auto find = this->keys_.find(ToString(key));
        if (find == this->keys_.end()) throw JsonException(*this, "Key not find.");
        return find->second->second;
    }
    template <typename _T>
    const Json& operator [] (const _T& key) const {
        auto find = this->keys_.find(ToString(key));
        if (find == this->keys_.end()) throw JsonException(*this, "Key not find.");
        return find->second->second;
    }
    void Until(std::function<bool(const Json&, Json&)> cb) {
        for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite) {
            if (cb(ite->first, ite->second)) break;
        }
    }
    void Until(std::function<bool(const Json&, const Json&)> cb) const {
        for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite) {
            if (cb(ite->first, ite->second)) break;
        }
    }
    void ForEach(std::function<void(const Json&, Json&)> cb) {
        for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite) {
            cb(ite->first, ite->second);
        }
    }
    void ForEach(std::function<void(const Json&, const Json&)> cb) const {
        for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite) {
            cb(ite->first, ite->second);
        }
    }

protected:
    virtual const std::string& Name() const override;
    virtual int DoParse(const std::string& raw, int start, JsonMask* mask) override;
    virtual void Print(std::ostream& os) const override;

private:
    template <typename _T>
    static std::string ToString(const _T& v) {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
    std::string ToString(const std::string& v) {
        return v;
    }

    std::map<std::string, std::list<std::pair<Json, Json>>::iterator> keys_;
    std::list<std::pair<Json, Json>> vals_;
}; // class JDict

#define OBJECT_DICT_CHECKER [](const std::string& raw, int start) { return raw[start] == '{'; }
REGIST_JSON_OBJECT(JDict, OBJECT_DICT_CHECKER, 5);
#undef OBJECT_DICT_CHECKER

JDict::JDict() {}

JDict::JDict(JDict&& dict) : 
    keys_(std::move(dict.keys_)),
    vals_(std::move(dict.vals_))
{
    this->SetPosition(dict.GetPosistion());
}

JDict::JDict(const JDict& dict) {
    *this = dict;
}

JDict& JDict::operator = (const JDict& dict) {
    this->SetPosition(dict.GetPosistion());
    this->vals_ = dict.vals_;
    for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite) {
        this->keys_.emplace(ToString(ite->first), ite);
    }
    return *this;
}

const std::string& JDict::Name() const {
    static std::string name("Object");
    return name;
}

int JDict::DoParse(const std::string& raw, int start, JsonMask* mask) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '{') throw JsonException(*this, raw, idx);
    idx += this->SkipSpaces(raw, idx + 1) + 1;
    if (raw[idx] == '}') return ++idx - start;
    for (; idx < n; ++idx) {
        int len;
        Json key(raw, idx, len, mask, false);
        idx += len;
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] == ':') {
            ++idx;
            Json val(raw, idx, len, mask, false);
            if (!this->Add(std::move(key), std::move(val))) throw JsonException(*this, raw, idx - 2, "Existed key.");
            idx += len;
        }
        else if (raw[idx] == ',' || raw[idx] == '}') {
            if (!this->Add(std::move(key), JNull())) throw JsonException(*this, raw, idx - 2, "Existed key.");
        }
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] != ',' && raw[idx] != '}') throw JsonException(*this, raw, idx);
        if (raw[idx] == '}') return ++idx - start;
    }
    throw JsonException(*this, raw, idx);
    return idx - start;
}

void JDict::Print(std::ostream& os) const {
    os << '{';
    int idx = 0;
    for (auto ite = this->vals_.begin(); ite != this->vals_.end(); ++ite, ++idx) {
        if (idx != 0) os << ',';
        os << ite->first;
        if (!ite->second.Is<JNull>()) {
            os << ':' << ite->second;
        }
    }
    os << '}';
}


// post implemention
int Json::Parse(const std::string& raw, int start, JsonMask* mask, bool all) {
    auto flag = [&mask](int m) -> bool { return mask == NULL || (mask->Mask() & m); };
    if (this->obj_ != NULL) delete this->obj_;
    this->obj_ = NULL;
    int idx = start, n = raw.size();
    while (idx < n && raw[idx] == ' ') ++idx;
    if (idx >= n) throw JsonException(*this, raw, idx);

    ObjectManager::Until([&](const ObjectManager::Api& api) -> bool {
        if (flag(api.mask) && api.checker(raw, idx)) {
            Object* obj = api.constructor();
            try {
                obj->SetPosition(idx);
                int len = obj->Parse(raw, idx, mask ? mask->Next() : NULL);
                if (len <= 0) {
                    throw JsonException(*this, raw, idx,
                        std::string("Invalid JSON parse result with parser = ")
                            + obj->GetName() + "."
                    );
                }
                idx += len;
                this->obj_ = obj;
            }
            catch (JsonException& e) {
                delete obj;
                throw e;
            }
            return true;
        }
        return false;
    });
    if (this->obj_ == NULL) throw JsonException(*this, raw, idx, "Invalid JSON format.");
    if (all) {
        while (idx < n && raw[idx] == ' ') ++idx;
        if (idx < n) throw JsonException(*this, raw, idx, "Invalid JSON format.");
    }
    return idx - start;
}



//quick APIs
template <typename _T, typename... _ARGS>
Json Create(_ARGS&&... args) {
    return Json::Create<_T>(args...);
}

} // namespace json

} // namespace lc

#endif