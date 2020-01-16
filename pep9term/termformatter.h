#ifndef TERMFORMATTER_H
#define TERMFORMATTER_H
#include "CLI11.hpp"
#include <QDebug>
class TermFormatter : public CLI::Formatter {
public:
    TermFormatter(std::map<std::string,std::map<std::string,std::string>> &param_names): CLI::Formatter(),
        param_names(param_names)
    {

    }
    TermFormatter(const TermFormatter &) = default;
    TermFormatter(TermFormatter &&) = default;

    /// This displays the usage line
    std::string make_name(const CLI::App *app, std::string name) const;
    /// This displays the usage line
    std::string make_usage(const CLI::App *app, std::string name) const override;
    /// This puts everything together
    std::string make_help(const CLI::App *, std::string, CLI::AppFormatMode) const override;
    std::string make_options_names(const CLI::Option *opt, std::string group_name) const;
    std::string find_param_name(const CLI::Option *opt, std::string group_name) const;
    std::string make_option_opts(const CLI::Option *opt) const override;
    std::string make_subcommands(const CLI::App *app, CLI::AppFormatMode mode) const;
private:
    std::map<std::string,std::map<std::string,std::string>> &param_names;
    mutable std::string subcommand_name;
};

std::string TermFormatter::make_name(const CLI::App *app, std::string name) const
{
    std::stringstream out;

    out << "Name:" << std::endl;
    out << name ;
    return out.str();
}

std::string TermFormatter::make_usage(const CLI::App *app, std::string name) const {
    std::stringstream out;

    if(name == "pep9term") name.clear();
    auto subcom = app->get_subcommands(std::function<bool(const CLI::App *)>([](const CLI::App *){return true;}));
    auto options = app->get_options(std::function<bool(const CLI::Option *)>([](const CLI::Option *){return true;}));
    if(!options.empty()) {
        out << "pep9term ";
        if(!name.empty()) {
            out << name << " ";
        }
        for (auto option : options) {
            if(!name.empty() && option->get_name(false, false) == "--help-all") continue;
            out << make_options_names(option, name);
            out << " ";
        }
        out << std::endl;
    }
    for(const auto command : subcom) {
        out << make_usage(command, command->get_name());
    }

    return out.str();
}

std::string TermFormatter::make_help(const CLI::App *app, std::string name, CLI::AppFormatMode mode) const {

    // This immediately forwards to the make_expanded method. This is done this way so that subcommands can
    // have overridden formatters
    if(mode == CLI::AppFormatMode::Sub)
        return make_expanded(app);

    std::stringstream out;
    if((app->get_name().empty()) && (app->get_parent() != nullptr)) {
        if(app->get_group() != "Subcommands") {
            out << app->get_group() << ':';
        }
    }
    out << make_name(app, name);
    out << " -- ";
    out << make_description(app) << std::endl;
    out << get_label("Usage:") << std::endl;
    out << make_usage(app, name) << std::endl;
    out << make_groups(app, mode);
    out << make_subcommands(app, mode) << std::endl;
    out << make_footer(app);

    return out.str();
}

std::string TermFormatter::make_options_names(const CLI::Option *opt, std::string group_name) const
{
    std::stringstream out;
    out << opt->get_name(false, false);
    if(auto str = find_param_name(opt, group_name);
            !str.empty()) {
        out << " " << str;
    }

    if(opt->get_required()) {
        return out.str();
    }
    else {
        return "[" + out.str()+"]";
    }
}

std::string TermFormatter::find_param_name(const CLI::Option *opt, std::string group_name) const
{
    std::vector<std::string> option_names;
    for(auto fname : opt->get_fnames()) {
        option_names.push_back(fname);
    }
    for(auto lname : opt->get_lnames()) {
        option_names.push_back(lname);
    }
    for(auto sname : opt->get_snames()) {
        option_names.push_back(sname);
    }

    if(auto map_it = param_names.find(group_name);
            map_it != param_names.end()) {
        auto map = map_it->second;
        for(auto name : option_names) {
            if(auto tag_it = map.find(name);
                    tag_it != map.end()) {
                auto tag = tag_it->second;
                return tag;
            }
        }
    }
    return "";
}

std::string TermFormatter::make_option_opts(const CLI::Option *opt) const {
    return " " + find_param_name(opt, subcommand_name);
}

std::string TermFormatter::make_subcommands(const CLI::App *app, CLI::AppFormatMode mode) const {
    std::stringstream out;
    subcommand_name = app->get_name();
    return CLI::Formatter::make_subcommands(app, mode);
}

#endif // TERMFORMATTER_H
