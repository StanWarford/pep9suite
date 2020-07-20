#ifndef TERMFORMATTER_H
#define TERMFORMATTER_H
#include "CLI11.hpp"
#include <deque>
#include <memory>

#include <QDebug>
class TermFormatter : public CLI::Formatter {
public:
    TermFormatter(std::map<std::string, std::map<std::string,std::string> > &param_names,
                  std::map<std::string, std::string> &detailed_descriptions): CLI::Formatter(),
        param_names(param_names), detailed_descriptions(detailed_descriptions)
    {

    }

    TermFormatter(const TermFormatter &) = default;
    TermFormatter(TermFormatter &&) = default;

    /// This displays the usage line
    std::string make_name(const CLI::App *app, std::string name) const;
    /// This displays the usage line
    std::string make_heirarchical_usage(const CLI::App *app, std::string name, std::string fully_qualified_name) const;
    /// This puts everything together
    std::string make_help(const CLI::App *, std::string, CLI::AppFormatMode) const override;
    std::string make_options_names(const CLI::Option *opt, std::string group_name) const;
    std::string find_param_name(const CLI::Option *opt, std::string group_name) const;
    std::string make_option_opts(const CLI::Option *opt) const override;
    std::string make_subcommands(const CLI::App *app, CLI::AppFormatMode mode) const override;

private:
    // For each subcommand (key), mantain a list of flag names (sub-key) and the pretty-print name of the value
    // (sub-value). This allows for formatting as requested by Dr. Warford such as (-e error_file),
    // since the default CLI11 framework does not allow custom fields.
    std::map<std::string, std::map<std::string,std::string>> &param_names;
    // For a given subcommand (key) add an additional lengthened description of the subcommand (value).
    // Must be passed to the custom formatter, since the formatter is responsible for "switching" descriptions.
    std::map<std::string, std::string> &detailed_descriptions;
    // Determine if detailed_descriptions or default descriptions should be dumped to the console.
    mutable bool show_detailed_desc={false};
    // Name of the subcommand currently being processed.
    mutable std::string subcommand_name;
    // "Stack" of commands, the the deepest level being towards the back.
    // Necessary to print nested commands, since CLI11 does not mantain pointers to parent commands.
    mutable std::deque<std::string> subcommand_heirarchy;
};

std::string TermFormatter::make_name(const CLI::App *, std::string name) const
{
    std::stringstream out;

    out << "Name:" << std::endl;
    out << name ;
    return out.str();
}

std::string TermFormatter::make_heirarchical_usage(const CLI::App *app, std::string name, std::string fully_qualified_name) const {
    std::stringstream out;

    // Subcommands (CLI::App) do not preserve heirarchy information on their own.
    // Therefore, it would be impossible to print something like "pep9term cpuasm"
    // automatically. So, we need to construct a stack-like structure of visited commands.
    // Enumerate all options and generate usage information for each option.
    auto options = app->get_options(std::function<bool(const CLI::Option *)>([](const CLI::Option *){return true;}));

    // If there are no options, skip straight to subcommands.
    if(!options.empty()) {
        // Appended with the name of the current application.
        out << fully_qualified_name << " ";

        for (auto option : options) {
            // --help-all is redundant at lower levels, so don't generate it
            if(!subcommand_heirarchy.empty() && option->get_name(false, false) == "--help-all") continue;
            out << make_options_names(option, name);
            out << " ";
        }
        out << std::endl;
    }

    // Recursively generate usage help for each subcommand.
    // Must use std::function filter, because by default get_subcommands() filters out all items.
    auto subcom = app->get_subcommands(std::function<bool(const CLI::App *)>([](const CLI::App *){return true;}));

    // Push the current subcommand name onto the visited structure so
    // subcommands can print out parent's names (i.e. pep9term cpuasm)
    subcommand_heirarchy.push_back(name);
    for(const auto command : subcom) {
        std::string cat_name = fully_qualified_name + " " + command->get_name();
        out << make_heirarchical_usage(command, command->get_name(), cat_name);
    }
    // Pop the current subcommand name.
    subcommand_heirarchy.pop_back();

    return out.str();
}

std::string TermFormatter::make_help(const CLI::App *app, std::string name, CLI::AppFormatMode mode) const {

    // Need to store name of current subcommand so it is accessible when looking up option's flags.
    subcommand_name = app->get_name();

    // This immediately forwards to the make_expanded method.
    // This is done this way so that subcommands can
    // have overridden formatters.
    // Also enable or disable muli-line descriptions for the applications.
    if(mode == CLI::AppFormatMode::Sub) {
        show_detailed_desc = true;
        std::string ret_val = make_expanded(app);
        return ret_val;
    } else if(mode == CLI::AppFormatMode::All){
        show_detailed_desc = true;
    } else {
        show_detailed_desc = false;
    }

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
    // Name is full list of subcommands traversed to get to here.
    // We don't differentiate between subcommands with the same names via different paths,
    // since this should never occur within Pep/9.
    out << make_heirarchical_usage(app, app->get_name(), name) << std::endl;

    // If a detailed usage section is available, print it to the console under the "Usage" section.
    if(auto detailed_help = detailed_descriptions.find(app->get_name());
       detailed_help != detailed_descriptions.end()) {
        out << detailed_help->second << std::endl;
    }

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
