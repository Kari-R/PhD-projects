#include <Config.hpp>
#include <util.hpp>

Config::Arg::Arg(const std::string& name, const std::string& defval, const std::string& descr):
    name(name), strval(defval), descr(descr) {

    type = deduce_type(defval);
    if(defval == "false")
        numval = 0;
    else if(defval == "true")
        numval = 1;
    else
        std::stringstream(defval) >> numval;
}

Config::Arg& Config::Arg::flip() {
    if(strval == "false") { numval = 1; strval = "true"; } else
    if(strval == "true")  { numval = 0; strval = "false"; }
    return *this;
}

Config::Arg& Config::Arg::operator=(const std::string& value) {
    static const char* types[] { "string", "boolean", "integer", "float" };

    value_t ded = deduce_type(value);
    if(ded != type && !(ded == Integer && type == Float)) {
        print("Error: Argument '--%' needs to be % %.\n",
            name, type == Integer? "an": "a", types[(size_t)type]);
        std::exit(-1);
    }

    strval = value;
    std::stringstream(strval) >> numval;
    return *this;
}

Config::Arg::value_t Config::Arg::deduce_type(const std::string& value) {
    if(value == "true" || value == "false")
        return Boolean;

    if(!value.length())
        return String;

    value_t t = Integer;
    size_t i = 0;
    if(value[0] == '-')
        i++;
    for(; i < value.length(); i++)
        if(value[i] == '.') {
            if(t == Float)
                return String;
            t = Float;
        } else if(value[i] < '0' || value[i] > '9')
            return String;
    return t;
}

void Config::read(size_t argc, char** argv) {
    std::vector<std::string> vec;
    vec.reserve(argc);
    for(size_t i = 0; i < argc; i++)
        vec.push_back(argv[i]);
    read(vec);
}

void Config::read(const std::string& str) {
    read(split("./build " + str, ' '));
}

void Config::read(const std::vector<std::string>& vec) {
    print("Arguments:");
    for(const std::string& s: vec)
        print(" %", s);
    print("\n");
    
    for(size_t i = 1; i < vec.size(); i++) {
        if(vec[i].length() < 3 || vec[i].substr(0, 2) != "--") {
            unnamed_args.push_back(vec[i]);
            continue;
        }
        size_t j = 0;
        for(; j < named_args.size(); j++)
            if("--" + named_args[j].name == vec[i])
                break;
        if(j == named_args.size()) {
            print("Error: Invalid argument '%'.\n", vec[i]);
            std::exit(-1);
        }
        if(named_args[j].type == Arg::Boolean)
            named_args[j].flip();
        else if(i + 1 == vec.size()) {
            print("Error: Argument '--%' requires a value.\n", named_args[j].name);
            std::exit(-1);
        } else {
            i++;
            named_args[j] = vec[i];
        }
    }
}

void Config::add(const std::string& name, const std::string& defval, const std::string& descr) {
    named_args.push_back(Arg(name, defval, descr));
}

void Config::put_spaces(size_t count) {
    for(size_t i = 0; i < count; i++)
        print(" ");
}

void Config::output() {
    static std::string types[] { "string", "boolean", "integer", "float" };

    size_t max_left = 0;
    std::vector<std::string> left;
    for(const Arg& arg: named_args) {
        left.push_back("--" + arg.name);
        if(arg.type != Arg::Boolean)
            left.back() += " <" + types[(size_t)arg.type] + ">";
        if(max_left < left.back().length())
            max_left = left.back().length();
    }
    max_left += 2;

    print("Flags (defaults in parentheses):\n");
    for(size_t i = 0; i < named_args.size(); i++) {
        print("%", left[i]);
        put_spaces(max_left - left[i].length());

        auto vec = split(named_args[i].descr, '\n');
        for(size_t j = 0; j < vec.size(); j++) {
            if(vec[j] == "") continue;
            if(j > 0)
                put_spaces(max_left);
            print("%", vec[j]);
            if(!j && named_args[i].type != Arg::Boolean)
                print(" (%)\n", named_args[i].strval);
            else
                print("\n");
        }
    }
}

void Config::set(const std::string& name, const std::string& value) {
    for(Arg& arg: named_args)
        if(arg.name == name) {
            arg = value;
            return;
        }
    print("Error: Undefined argument '%'\n", name);
    std::exit(-1);
}
