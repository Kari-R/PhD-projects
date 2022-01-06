#include <Config.hpp>

void Config::output() const {
    std::vector<std::string> begins;
    size_t max_width = 0;
    for(size_t i = 0; i < args.size(); i++) {
        begins.push_back("");
        for(const std::string& name: args[i].names) {
            if(begins.back() != "") begins.back() += ", ";
            begins.back() += name;
        }
        if(!args[i].is_bool)
            begins.back() += " <value>";
        max_width = std::max(max_width, begins.back().length());
    }

    for(size_t i = 0; i < args.size(); i++)
        print("  %% (%)\n", justify(begins[i], max_width + 2),
            args[i].description, args[i].str);
}

void Config::read(int argc, char** argv) {
    auto add = [&](size_t i) {
        if(string != "")
            string += "_";
        for(char c: std::string(argv[i]))
            if(c != ' ')
                string += c;
    };

    for(size_t i = 1; i < (size_t)argc; i++) {
        std::string name = argv[i];
        if(name != "" && name[0] != '-') {
            if(input_fname != "") {
                print("Input filename given multiple times.\n");
                std::exit(-1);
            }
            input_fname = name;
            continue;
        }
        auto it = names.find(name);
        if(it == names.end()) {
            print("Unrecognized argument '%'\n", name);
            std::exit(-1);
        }
        add(i);
        if((*this)[it->second].is_bool)
            set(it->second, (*this)[it->second] == 0? "1": "0");
        else {
            ensure(i + 1 < (size_t)argc);
            set(it->second, argv[i + 1]);
            add(i + 1);
            i++;
        }
    }
}