#ifndef LEETCODE_CONVERT
#define LEETCODE_CONVERT

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <cassert>

#include "leetcode-json.h"

namespace lc {

namespace conv {

struct ConvertException : public std::exception {
public:
    ConvertException(const json::Json& obj, const std::string& info = "");
    const json::Json& GetJson() const;
    const char* what() const throw ();

private:
    json::Json obj_;
    std::string info_;
}; // struct ConvertException

ConvertException::ConvertException(const json::Json& obj, const std::string& info) :
    obj_(obj),
    info_("Convert error. ")
{
    this->info_ += info;
}

const json::Json& ConvertException::GetJson() const {
    return this->obj_;
}

const char* ConvertException::what() const throw () {
    return this->info_.c_str();
}



//common template
template <typename _T, bool object = json::_is_object<_T>::value>
struct Convert {
    static void FromJson(_T& v, const json::Json& js) {
        throw ConvertException(js, std::string("Conversion from JSON to ") + json::_get_name<_T>() + " not implemented.");
    }

    static json::Json ToJson(const _T& v) {
        throw ConvertException(json::Json(), std::string("Conversion from ") + json::_get_name<_T>() + " to JSON not implemented.");
    }
};

//object specialization
template <typename _T>
struct Convert<_T, true> {
#define ASSERT_HINT "Hey, this is not a json::Object, right?"
    static void FromJson(_T& v, const json::Json& js) {
        static_assert(json::_is_object<_T>::value, ASSERT_HINT);
        v = *js.GetObject<_T>();
    }

    static json::Json ToJson(const _T& v) {
        static_assert(json::_is_object<_T>::value, ASSERT_HINT);
        return v;
    }
#undef ASSERT_HINT
};

//reference type specialization
template <typename _T>
struct Convert<_T&, true> {
    static void FromJson(_T& v, const json::Json& js) {
        Convert<_T>::FromJson(v, js);
    }

    static void FromJson(_T& v, json::Json&& js) {
        Convert<_T>::FromJson(v, std::move(js));
    }

    static json::Json ToJson(const _T& v) {
        return Convert<_T>::ToJson(v);
    }

    static json::Json ToJson(_T&& v) {
        return Convert<_T>::ToJson(std::move(v));
    }
};

template <typename _T>
struct Convert<_T&, false> {
    static void FromJson(_T& v, const json::Json& js) {
        Convert<_T>::FromJson(v, js);
    }

    static void FromJson(_T& v, json::Json&& js) {
        Convert<_T>::FromJson(v, std::move(js));
    }

    static json::Json ToJson(const _T& v) {
        return Convert<_T>::ToJson(v);
    }

    static json::Json ToJson(_T&& v) {
        return Convert<_T>::ToJson(std::move(v));
    }
};

template <>
struct Convert<json::Json> {
    static void FromJson(json::Json& v, const json::Json& js) {
        v = js;
    }

    static void FromJson(json::Json& v, json::Json&& js) {
        v.Swap(js);
    }

    static json::Json ToJson(const json::Json& v) {
        return v;
    }

    static json::Json ToJson(json::Json&& v) {
        return std::move(v);
    }
};



#define QUICK_THROW(js, type) throw ConvertException(js, std::string("Convert from JSON to ") + json::_get_name<type>() + " failed.")

template <>
struct Convert<bool> {
    static void FromJson(bool& v, const json::Json& js) {
        auto obj = js.GetObject<json::JBoolean>();
        if (obj == NULL) QUICK_THROW(js, bool);
        v = obj->GetValue();
    }
    static json::Json ToJson(const bool& v) {
        return json::JBoolean(v);
    }
};


template <>
struct Convert<int> {
    static void FromJson(int& v, const json::Json& js) {
        auto obj = js.GetObject<json::JNumber>();
        if (obj == NULL || !obj->IsInteger()) QUICK_THROW(js, int);
        v = obj->GetInteger();
    }
    static json::Json ToJson(const int& v) {
        return json::JNumber((long long)v);
    }
};


template <>
struct Convert<long long> {
    static void FromJson(long long& v, const json::Json& js) {
        auto obj = js.GetObject<json::JNumber>();
        if (obj == NULL || !obj->IsInteger()) QUICK_THROW(js, long long);
        v = obj->GetInteger();
    }
    static json::Json ToJson(const long long& v) {
        return json::JNumber(v);
    }
};


template <>
struct Convert<double> {
    static void FromJson(double& v, const json::Json& js) {
        auto obj = js.GetObject<json::JNumber>();
        if (obj == NULL) QUICK_THROW(js, double); 
        v = obj->GetNumber();
    }

    static json::Json ToJson(const double& v) {
        return json::JNumber(v);
    }
};



template <>
struct Convert<std::string> {
    static void FromJson(std::string& v, const json::Json& js) {
        auto obj = js.GetObject<json::JString>();
        if (obj == NULL) QUICK_THROW(js, std::string);
        v = obj->GetString();
    }

    static void FromJson(std::string& v, json::Json&& js) {
        auto obj = js.GetObject<json::JString>();
        if (obj == NULL) QUICK_THROW(js, std::string);
        v.swap(obj->GetString());
    }
    
    static json::Json ToJson(const std::string& v) {
        return json::Json::Create<json::JString>(v);
    }

    static json::Json ToJson(std::string&& v) {
        return json::Json::Create<json::JString>(std::move(v));
    }
};


template <>
struct Convert<char> {
    static void FromJson(char& v, const json::Json& js) {
        auto& str = js.GetObject<json::JString>()->GetString();
        if (str.size() != 1) QUICK_THROW(js, char);
        v = str.front();
    }

    static json::Json ToJson(const char& v) {
        return json::Json::Create<json::JString>(std::string(1, v));
    }
};


template <typename _VAL>
struct Convert<std::vector<_VAL>, false> {
    static void FromJson(std::vector<_VAL>& v, const json::Json& js) {
        auto obj = js.GetObject<json::JArray>();
        if (obj == NULL) QUICK_THROW(js, std::vector<_VAL>);
        int n = obj->GetArray().size();
        v.resize(n);
        for (int i = 0; i < n; ++i) {
            Convert<_VAL>::FromJson(v[i], obj->GetArray()[i]);
        }
    }

    static json::Json ToJson(const std::vector<_VAL>& v) {
        json::Json js = json::Json::Create<json::JArray>();
        auto obj = js.GetObject<json::JArray>();
        for (auto& sub : v) {
            obj->GetArray().emplace_back(Convert<_VAL>::ToJson(sub));
        }
        return js;
    }
};



template <typename _KEY, typename _VAL>
struct Convert<std::map<_KEY, _VAL>, false> {
    static void FromJson(std::map<_KEY, _VAL>& v, const json::Json& js) {
        auto obj = js.GetObject<json::JDict>();
        using type = std::map<_KEY, _VAL>;
        if (obj == NULL) QUICK_THROW(js, type);
        obj->ForEach([&v](const json::Json& key, const json::Json& val) {
            _KEY vark;
            _VAL varv;
            Convert<_KEY>::FromJson(vark, key);
            Convert<_VAL>::FromJson(varv, val);
            v.emplace(std::move(vark), std::move(varv));
        });
    }

    static json::Json ToJson(const std::map<_KEY, _VAL>& v) {
        json::Json js = json::Json::Create<json::JDict>();
        auto obj = js.GetObject<json::JDict>();
        for (auto& sub : v) {
            obj->Add(
                Convert<_KEY>::ToJson(sub.first),
                Convert<_VAL>::ToJson(sub.second)
            );
        }
        return js;
    }
};



template <typename... _ARGS>
struct Convert<std::tuple<_ARGS...>, false> {
    static void FromJson(std::tuple<_ARGS...>& v, const json::Json& js) {
        auto obj = js.GetObject<json::JArray>();
        int n = std::tuple_size<std::tuple<_ARGS...>>::value;
        if (obj == NULL || n != obj->GetArray().size()) QUICK_THROW(js, std::tuple<_ARGS...>);
        Impl<std::tuple<_ARGS...>, sizeof...(_ARGS)>::DoFromJson(v, *obj);
    }

    static json::Json ToJson(const std::tuple<_ARGS...>& v) {
        json::Json js = json::Json::Create<json::JArray>();
        auto obj = js.GetObject<json::JArray>();
        Impl<std::tuple<_ARGS...>, sizeof...(_ARGS)>::DoToJson(v, *obj);
        return js;
    }

// tuple implementations
private:
    template <typename _T, size_t N>
    struct Impl {
        static void DoFromJson(_T& v, json::JArray& obj) {
            Impl<_T, N-1>::DoFromJson(v, obj);
            using SubType = typename std::tuple_element<N-1, _T>::type;
            Convert<SubType>::FromJson(std::get<N-1>(v), obj.GetArray()[N-1]);
        }

        static void DoToJson(const _T& v, json::JArray& obj) {
            Impl<_T, N-1>::DoToJson(v, obj);
            using SubType = typename std::tuple_element<N-1, _T>::type;
            obj.GetArray().emplace_back(Convert<SubType>::ToJson(std::get<N-1>(v)));
        }
    };

    template <typename _T>
    struct Impl<_T, 1> {
        static void DoFromJson(_T& v, json::JArray& obj) {
            using SubType = typename std::tuple_element<0, _T>::type;
            Convert<SubType>::FromJson(std::get<0>(v), obj.GetArray()[0]);
        }

        static void DoToJson(const _T& v, json::JArray& obj) {
            using SubType = typename std::tuple_element<0, _T>::type;
            obj.GetArray().emplace_back(Convert<SubType>::ToJson(std::get<0>(v)));
        }
    };
};

#undef QUICK_THROW



template <typename _T>
void FromJson(_T& v, const json::Json& js) {
    Convert<_T>::FromJson(v, js);
}

template <typename _T>
void FromJson(_T& v, json::Json&& js) {
    Convert<_T>::FromJson(v, std::move(js));
}

template <typename _T>
_T FromJson(const json::Json& js) {
    _T v;
    FromJson(v, js);
    return v;
}

template <typename _T>
_T FromJson(json::Json&& js) {
    _T v;
    FromJson(v, std::move(js));
    return v;
}

template <typename _T>
void FromJson(_T& v, const std::string& raw) {
    Convert<_T>::FromJson(v, json::Json(raw));
}

template <typename _T>
_T FromJson(const std::string& raw) {
    _T v;
    FromJson(v, json::Json(raw));
    return v;
}

template <typename _T>
json::Json ToJson(const _T& v) {
    return Convert<_T>::ToJson(v);
}

template <typename _T>
json::Json ToJson(_T&& v) {
    return Convert<_T>::ToJson(std::move(v));
}

} // namespace conv
    
} // namespace lc

#endif