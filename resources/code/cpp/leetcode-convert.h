#ifndef LEETCODE_CONVERT
#define LEETCODE_CONVERT

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <tuple>

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
template <typename _T>
struct Convert {
    static void FromJson(_T& v, const json::Json& js) {
        throw ConvertException(js, "Conversion from JSON not implemented.");
    }

    static json::Json ToJson(const _T& v) {
        auto ptr = dynamic_cast<const json::Object*>(&v);
        if (ptr != NULL) return *ptr;
        throw ConvertException(json::Json(), "Conversion to JSON not implemented.");
    }
};

//reference type specialization
template <typename _T>
struct Convert<_T&> {
    static void FromJson(_T& v, const json::Json& js) {
        Convert<_T>::FromJson(v, js);
    }

    static json::Json ToJson(const _T& v) {
        return Convert<_T>::ToJson(v);
    }
};

template <>
struct Convert<json::Json> {
    static void FromJson(json::Json& v, const json::Json& js) {
        v = js;
    }

    static json::Json ToJson(const json::Json& v) {
        return v;
    }
};



#define QUICK_THROW(js, type) throw ConvertException(js, std::string("Convert to [") + #type + "] failed.")

template <>
struct Convert<int> {
    static void FromJson(int& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectNumber>();
        if (obj == NULL || !obj->IsInteger()) QUICK_THROW(js, int);
        v = obj->GetInteger();
    }
    static json::Json ToJson(const int& v) {
        return json::ObjectNumber(v);
    }
};



template <>
struct Convert<double> {
    static void FromJson(double& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectNumber>();
        if (obj == NULL) QUICK_THROW(js, double); 
        v = obj->GetNumber();
    }

    static json::Json ToJson(const double& v) {
        return json::ObjectNumber(v);
    }
};



template <>
struct Convert<std::string> {
    static void FromJson(std::string& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectString>();
        if (obj == NULL) QUICK_THROW(js, std::string);
        v = obj->GetString();
    }
    
    static json::Json ToJson(const std::string& v) {
        return json::ObjectString(v);
    }
};



template <typename _VAL>
struct Convert<std::vector<_VAL>> {
    static void FromJson(std::vector<_VAL>& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectArray>();
        if (obj == NULL) QUICK_THROW(js, std::vector);
        int n = obj->GetArray().size();
        v.resize(n);
        for (int i = 0; i < n; ++i) {
            Convert<_VAL>::FromJson(v[i], obj->GetArray()[i]);
        }
    }

    static json::Json ToJson(const std::vector<_VAL>& v) {
        json::ObjectArray js;
        for (auto& sub : v) {
            js.GetArray().emplace_back(Convert<_VAL>::ToJson(sub));
        }
        return js;
    }
};



template <typename _KEY, typename _VAL>
struct Convert<std::map<_KEY, _VAL>> {
    static void FromJson(std::map<_KEY, _VAL>& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectDict>();
        if (obj == NULL) QUICK_THROW(js, std:map);
        obj->ForEach([&v](const json::Json& key, const json::Json& val) {
            _KEY vark;
            _VAL varv;
            Convert<_KEY>::FromJson(vark, key);
            Convert<_VAL>::FromJson(varv, val);
            v.emplace(std::move(vark), std::move(varv));
        });
    }

    static json::Json ToJson(const std::map<_KEY, _VAL>& v) {
        json::ObjectDict js;
        for (auto& sub : v) {
            js.Add(Convert<_KEY>::ToJson(sub.first), Convert<_VAL>::ToJson(sub.second));
        }
        return js;
    }
};



template <typename... _ARGS>
struct Convert<std::tuple<_ARGS...>> {
    static void FromJson(std::tuple<_ARGS...>& v, const json::Json& js) {
        auto obj = js.GetObject<json::ObjectArray>();
        int n = std::tuple_size<std::tuple<_ARGS...>>::value;
        if (obj == NULL || n != obj->GetArray().size()) QUICK_THROW(js, std::tuple);
        Impl<std::tuple<_ARGS...>, sizeof...(_ARGS)>::DoFromJson(v, *obj);
    }

    static json::Json ToJson(const std::tuple<_ARGS...>& v) {
        json::ObjectArray js;
        Impl<std::tuple<_ARGS...>, sizeof...(_ARGS)>::DoToJson(v, js);
        return js;
    }

// tuple implementations
private:
    template <typename _T, size_t N>
    struct Impl {
        static void DoFromJson(_T& v, json::ObjectArray& obj) {
            Impl<_T, N-1>::DoFromJson(v, obj);
            using SubType = typename std::tuple_element<N-1, _T>::type;
            Convert<SubType>::FromJson(std::get<N-1>(v), obj.GetArray()[N-1]);
        }

        static void DoToJson(const _T& v, json::ObjectArray& obj) {
            Impl<_T, N-1>::DoToJson(v, obj);
            using SubType = typename std::tuple_element<N-1, _T>::type;
            obj.GetArray().emplace_back(Convert<SubType>::ToJson(std::get<N-1>(v)));
        }
    };

    template <typename _T>
    struct Impl<_T, 1> {
        static void DoFromJson(_T& v, json::ObjectArray& obj) {
            using SubType = typename std::tuple_element<0, _T>::type;
            Convert<SubType>::FromJson(std::get<0>(v), obj.GetArray()[0]);
        }

        static void DoToJson(const _T& v, json::ObjectArray& obj) {
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
_T FromJson(const json::Json& js) {
    _T v;
    FromJson(v, js);
    return v;
}

template <typename _T>
json::Json ToJson(const _T& v) {
    return Convert<_T>::ToJson(v);
}

} // namespace conv
    
} // namespace lc

#endif