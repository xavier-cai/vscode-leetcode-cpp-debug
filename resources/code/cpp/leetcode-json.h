#ifndef LEETCODE_JSON_H
#define LEETCODE_JSON_H

#include <string>
#include <exception>
#include <sstream>
#include <ostream>
#include <vector>
#include <map>
#include <cstring>
#include <functional>

#ifndef NULL
#define NULL 0
#endif

namespace lc {

namespace clone {

class Interface {
public:
    virtual ~Interface() {}
    virtual Interface* DoClone() const = 0;

protected:
    virtual void DoCopyTo(Interface* clone) const {}
}; // class Interface

template<typename _THIS, typename _BASE = Interface>
class Template : public _BASE {
public:
    virtual Interface* DoClone() const override {
        if (dynamic_cast<const Interface*>(this) == NULL) return NULL;
        if (dynamic_cast<const _THIS*>(this) == NULL) return NULL;
        Interface* clone = dynamic_cast<Interface*>(new _THIS(*dynamic_cast<const _THIS*>(this)));
        this->DoCopyTo(clone);
        return clone;
    }

    _THIS* Clone() const {
        return dynamic_cast<_THIS*>(this->DoClone());
    }

protected:
    virtual void DoCopy(_THIS* clnoe) const {}
    virtual void DoCopyTo(Interface* clone) const override {
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



class Object : public clone::Interface {
public:
    enum Type {
        JSON_NULL = 1,
        BOOLEAN = 2,
        NUMBER = 4,
        STRING = 8,
        ARRAY = 16,
        OBJECT = 32, //DICT
    };

    virtual ~Object() {}

    Type GetType() const;
    JsonMask* GetMask() const;
    void SetMask(JsonMask* mask);
    int GetPosistion() const;
    void SetPosition(int pos);
    int Parse(const std::string& raw, int start = 0);

    friend std::ostream& operator << (std::ostream& os, const Object& obj);

protected:
    Object();
    void SetType(Type type);
    //return length
    virtual int DoParse(const std::string& raw, int start) = 0;
    virtual void Print(std::ostream& os) const = 0;

    //return length
    int SkipSpaces(const std::string& raw, int start, bool error = true);

private:
    Type type_;
    JsonMask* mask_;
    int pos_;
}; // class Object



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

    Object* GetObject() const;
    template <typename _T>
    _T* GetObject() const { return dynamic_cast<_T*>(this->obj_); }
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
Object::Object() : mask_(NULL), pos_(-1) {}

Object::Type Object::GetType() const {
    return this->type_;
}

void Object::SetType(Object::Type type) {
    this->type_ = type;
}

JsonMask* Object::GetMask() const {
    return this->mask_;
}

void Object::SetMask(JsonMask* mask) {
    this->mask_ = mask;
}

int Object::GetPosistion() const {
    return this->pos_;
}

void Object::SetPosition(int pos) {
    this->pos_ = pos;
}

int Object::Parse(const std::string& raw, int start) {
    return this->DoParse(raw, start);
}

int Object::SkipSpaces(const std::string& raw, int start, bool error) {
    int idx = start, n = raw.size();
    while (idx < n && raw[idx] == ' ') ++idx;
    if (error && idx >= n) throw JsonException(*this, raw, idx);
    return idx - start;
}

std::ostream& operator << (std::ostream& os, const Object& obj) {
    obj.Print(os);
    return os;
}

std::ostream& operator << (std::ostream& os, const Object::Type& type) {
    switch (type)
    {
    case Object::JSON_NULL: os << "json::null"; break;
    case Object::NUMBER: os << "json::number"; break;
    case Object::STRING: os << "json::string"; break;
    case Object::ARRAY: os << "json::array"; break;
    case Object::OBJECT: os << "json::dict"; break;
    default: throw JsonException(Json(), std::string("Unkonwn json type.") + std::to_string((int)type));
    }
    return os;
}



// implements after JsonException
Json::Json() : obj_(NULL) {}

Json::Json(const std::string& raw, int start, JsonMask* mask, bool all) : obj_(NULL) { this->Parse(raw, start, mask, all); }

Json::Json(const std::string& raw, int start, int& len, JsonMask* mask, bool all) : obj_(NULL) { len = this->Parse(raw, start, mask, all); }

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



class ObjectNull : public clone::Template<ObjectNull, Object> {
public:
    ObjectNull();

protected:
    virtual int DoParse(const std::string& raw, int start) override;
    virtual void Print(std::ostream& os) const override;

private:
    static const std::string text_;
}; // class ObjectNull

const std::string ObjectNull::text_("null");

ObjectNull::ObjectNull() { this->Object::SetType(Object::JSON_NULL); }

int ObjectNull::DoParse(const std::string& raw, int start) {
    int n = raw.size(), m = this->text_.size();
    int idx = start + this->SkipSpaces(raw, start);
    for (int cmp = 0; cmp < m; ++cmp, ++idx) {
        if (idx >= n || raw[idx] != this->text_[cmp]) {
            throw JsonException(*this, raw, idx);
        }
    }
    return m;
}

void ObjectNull::Print(std::ostream& os) const {
    os << this->text_;
}



class ObjectBoolean : public clone::Template<ObjectBoolean, Object> {
public:
    ObjectBoolean();
    ObjectBoolean(bool value);
    bool GetValue() const;

protected:
    virtual int DoParse(const std::string& raw, int start) override;
    virtual void Print(std::ostream& os) const override;

private:
    bool value_;

    static const std::string true_;
    static const std::string false_;
}; // class ObjectNull

const std::string ObjectBoolean::true_("true");
const std::string ObjectBoolean::false_("false");

ObjectBoolean::ObjectBoolean() : value_(false) { this->Object::SetType(Object::BOOLEAN); }

ObjectBoolean::ObjectBoolean(bool value) : value_(value) { this->Object::SetType(Object::BOOLEAN); }

bool ObjectBoolean::GetValue() const {
    return this->value_;
}

int ObjectBoolean::DoParse(const std::string& raw, int start) {
    int idx = start + this->SkipSpaces(raw, start);
    int n = raw.size(), mt = this->true_.size(), mf = this->false_.size();
    auto check = [&](const std::string& str) -> bool {
        int m = str.size();
        if (idx + m > n) return false;
        return std::strncmp(str.c_str(), raw.c_str() + idx, m) == 0;
    };
    if (check(this->true_)) {
        this->value_ = true;
        return mt;
    }
    if (check(this->false_)) {
        this->value_ = false;
        return mf;
    }
    throw JsonException(*this, raw, idx);
    return 0;
}

void ObjectBoolean::Print(std::ostream& os) const {
    os << (this->value_ ? this->true_ : this->false_);
}



//TODO: scientific notation not supported yet
class ObjectNumber : public clone::Template<ObjectNumber, Object> {
public:
    ObjectNumber();
    ObjectNumber(int v);
    ObjectNumber(double v);
    int GetInteger() const;
    double GetNumber() const;
    bool IsInteger() const;
    void SetValue(int v);
    void SetValue(double v);

protected:
    virtual int DoParse(const std::string& raw, int start) override;
    virtual void Print(std::ostream& os) const override;

private:
    int integer_;
    double number_;
    bool isInteger_;
}; // class ObjectNumber

ObjectNumber::ObjectNumber() :
    integer_(0),
    number_(0.0),
    isInteger_(true)
{
    this->Object::SetType(Object::NUMBER);
}

ObjectNumber::ObjectNumber(int v)
{
    this->Object::SetType(Object::NUMBER);
    this->SetValue(v);
}

ObjectNumber::ObjectNumber(double v)
{
    this->Object::SetType(Object::NUMBER);
    this->SetValue(v);
}

int ObjectNumber::GetInteger() const {
    return this->integer_;
}

double ObjectNumber::GetNumber() const {
    return this->number_;
}

bool ObjectNumber::IsInteger() const {
    return this->isInteger_;
}

void ObjectNumber::SetValue(int v) {
    this->number_ = this->integer_ = v;
    this->isInteger_ = true;
}

void ObjectNumber::SetValue(double v) {
    this->integer_ = this->number_ = v;
    this->isInteger_ = false;
}

int ObjectNumber::DoParse(const std::string& raw, int start) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    std::stringstream ss1, ss2;
    int from = idx;
    while (idx < n && (
        (raw[idx] >= '0' && raw[idx] <= '9')
        || raw[idx] == '-'
        || raw[idx] == '.'
    )) {
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

void ObjectNumber::Print(std::ostream& os) const {
    if (this->isInteger_) os << this->integer_;
    else os << this->number_;
}



//TODO: escape not supported yet
class ObjectString : public clone::Template<ObjectString, Object> {
public:
    ObjectString();
    ObjectString(const std::string& s);
    ObjectString(std::string&& s);
    const std::string& GetString() const;
    void SetString(const std::string& s);

protected:
    virtual int DoParse(const std::string& raw, int start) override;
    virtual void Print(std::ostream& os) const override;

private:
    std::string string_;
}; // class ObjectString

ObjectString::ObjectString() { this->Object::SetType(Object::STRING); }

ObjectString::ObjectString(const std::string& s) : string_(s) { this->Object::SetType(Object::STRING); }

ObjectString::ObjectString(std::string&& s) { 
    this->Object::SetType(Object::STRING);
    this->string_.swap(s);
}

const std::string& ObjectString::GetString() const {
    return this->string_;
}

void ObjectString::SetString(const std::string& s) {
    this->string_ = s;
}

int ObjectString::DoParse(const std::string& raw, int start) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '\"') throw JsonException(*this, raw, idx);
    std::stringstream ss;
    for (++idx; idx < n && raw[idx] != '\"'; ++idx) ss << raw[idx];
    if (idx >= n) throw JsonException(*this, raw, idx);
    this->string_ = ss.str();
    return ++idx - start;
}

void ObjectString::Print(std::ostream& os) const {
    os << '\"' << this->string_ << '\"';
}



class ObjectArray : public clone::Template<ObjectArray, Object> {
public:
    typedef std::vector<Json> Array;
    ObjectArray();
    Array& GetArray();
    const Array& GetArray() const;

protected:
    virtual int DoParse(const std::string& raw, int start) override;
    virtual void Print(std::ostream& os) const override;

private:
    Array array_;
}; // class ObjectString

ObjectArray::ObjectArray() { this->Object::SetType(Object::ARRAY); }

ObjectArray::Array& ObjectArray::GetArray() {
    return this->array_;
}

const ObjectArray::Array& ObjectArray::GetArray() const {
    return this->array_;
}

int ObjectArray::DoParse(const std::string& raw, int start) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '[') throw JsonException(*this, raw, idx);
    idx += this->SkipSpaces(raw, idx + 1) + 1;
    if (raw[idx] == ']') return ++idx - start;
    for (; idx < n; ++idx) {
        int len;
        this->array_.emplace_back(raw, idx, len, this->GetMask(), false);
        idx += len;
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] != ',' && raw[idx] != ']') throw JsonException(*this, raw, idx);
        if (raw[idx] == ']') return ++idx - start;
    }
    throw JsonException(*this, raw, idx);
    return idx - start;
}

void ObjectArray::Print(std::ostream& os) const {
    os << '[';
    for (int i = 0, n = this->array_.size(); i < n; ++i) {
        if (i != 0) os << ',';
        os << this->array_[i];
    }
    os << ']';
}



class ObjectDict : public clone::Template<ObjectDict, Object> {
public:
    ObjectDict();
    template <typename _T>
    bool Have(const _T& key) {
        return this->keys_.find(ToString(key)) != this->keys_.end();
    }
    template <typename _T>
    bool Remove(const _T& key) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find == this->keys_.end()) return false;
        this->keys_.erase(k);
        this->vals_.erase(k);
        return true;
    }
    template <typename _T>
    bool Add(_T&& key, Json&& v) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find != this->keys_.end()) return false;
        this->vals_.emplace(k, v);
        this->keys_.emplace(k, std::move(ObjectString(k)));
        return true;
    }
    bool Add(Json&& key, Json&& v) {
        auto k = ToString(key);
        auto find = this->keys_.find(k);
        if (find != this->keys_.end()) return false;
        this->vals_.emplace(k, v);
        this->keys_.emplace(k, key);
        return true;
    }
    template <typename _T>
    Json& operator [] (const _T& key) {
        auto find = this->vals_.find(ToString(key));
        if (find == this->vals_.end()) throw JsonException(*this, "Key not find.");
        return find->second;
    }
    template <typename _T>
    const Json& operator [] (const _T& key) const {
        auto find = this->vals_.find(ToString(key));
        if (find == this->vals_.end()) throw JsonException(*this, "Key not find.");
        return find->second;
    }
    void ForEach(std::function<void(const Json&, Json&)> cb) {
        for (auto ite = this->keys_.begin(); ite != this->keys_.end(); ++ite) {
            cb(ite->second, this->vals_.at(ite->first));
        }
    }
    void ForEach(std::function<void(const Json&, const Json&)> cb) const {
        for (auto ite = this->keys_.begin(); ite != this->keys_.end(); ++ite) {
            cb(ite->second, this->vals_.at(ite->first));
        }
    }

protected:
    virtual int DoParse(const std::string& raw, int start) override;
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

    std::map<std::string, Json> keys_;
    std::map<std::string, Json> vals_;
}; // class ObjectDict

ObjectDict::ObjectDict() { this->Object::SetType(Object::OBJECT); }

int ObjectDict::DoParse(const std::string& raw, int start) {
    int idx = start + this->SkipSpaces(raw, start), n = raw.size();
    if (raw[idx] != '{') throw JsonException(*this, raw, idx);
    idx += this->SkipSpaces(raw, idx + 1) + 1;
    if (raw[idx] == '}') return ++idx - start;
    for (; idx < n; ++idx) {
        int len;
        Json key(raw, idx, len, this->GetMask(), false);
        idx += len;
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] != ':') throw JsonException(*this, raw, idx);
        ++idx;
        Json val(raw, idx, len, this->GetMask(), false);
        if (!this->Add(std::move(key), std::move(val))) throw JsonException(*this, raw, idx - 2, "Existed key.");
        idx += len;
        idx += this->SkipSpaces(raw, idx);
        if (raw[idx] != ',' && raw[idx] != '}') throw JsonException(*this, raw, idx);
        if (raw[idx] == '}') return ++idx - start;
    }
    throw JsonException(*this, raw, idx);
    return idx - start;
}

void ObjectDict::Print(std::ostream& os) const {
    os << '{';
    int idx = 0;
    for (auto ite = this->keys_.begin(); ite != this->keys_.end(); ++ite, ++idx) {
        if (idx != 0) os << ',';
        os << ite->second << ':' << this->vals_.at(ite->first);
    }
    os << '}';
}


// post implemention
int Json::Parse(const std::string& raw, int start, JsonMask* mask, bool all) {
    auto flag = [&mask](Object::Type type) -> bool { return mask == NULL || (mask->Mask() & type); };
    auto number = [](char c) -> bool { return (c >= '0' && c <= '9') || c == '.' || c == '-'; };
    auto check = [&](const std::string& str, int i) -> bool {
        int m = str.size();
        if (i + m > raw.size()) return false;
        return std::strncmp(str.c_str(), raw.c_str() + i, m) == 0;
    };
    if (this->obj_ != NULL) delete this->obj_;
    this->obj_ = NULL;
    int idx = start, n = raw.size();
    while (idx < n && raw[idx] == ' ') ++idx;
    if (idx >= n) throw JsonException(*this, raw, idx);
    if (flag(Object::JSON_NULL) && check("null", idx)) this->obj_ = new ObjectNull();
    else if (flag(Object::BOOLEAN) && (check("true", idx) || check("false", idx))) this->obj_ = new ObjectBoolean();
    else if (flag(Object::NUMBER) && number(raw[idx])) this->obj_ = new ObjectNumber();
    else if (flag(Object::STRING) && raw[idx] == '\"') this->obj_ = new ObjectString();
    else if (flag(Object::ARRAY) && raw[idx] == '[') this->obj_ = new ObjectArray();
    else if (flag(Object::OBJECT) && raw[idx] == '{') this->obj_ = new ObjectDict();
    if (this->obj_ == NULL) throw JsonException(*this, raw, idx, "Invalid JSON format.");
    this->obj_->SetMask(mask ? mask->Next() : NULL);
    this->obj_->SetPosition(idx);
    idx += this->obj_->Parse(raw, idx);
    int len = idx - start;
    if (all) {
        while (idx < n && raw[idx] == ' ') ++idx;
        if (idx < n) throw JsonException(*this, raw, idx, "Invalid JSON format.");
    }
    return len;
}

} // namespace json

} // namespace lc

#endif