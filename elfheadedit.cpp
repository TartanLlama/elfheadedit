#include <iostream>
#include <fstream>

#include "cxxopts.hpp"
#include "optional.hpp"
#include "elf.h"

template <std::size_t N>
void edit_header(std::ifstream& is, std::ostream& os,
                 nonstd::optional<std::string> machine,
                 nonstd::optional<std::string> flags) {
    using header_type = std::conditional_t<N == 32, Elf32_Ehdr, Elf64_Ehdr>;
    header_type header{};
    auto header_ptr = reinterpret_cast<char*>(&header);

    
    is.read(header_ptr, sizeof(header));
    
    if (machine) {
        header.e_machine = std::stoi(machine.value(), 0, 16);
    }
    if (flags) {
        header.e_flags = std::stoi(flags.value(), 0, 16);        
    }

    std::copy(header_ptr, header_ptr + sizeof(header),
              std::ostream_iterator<char>(os));
    os << is.rdbuf();
}
    

int main(int argc, char** argv) {
    cxxopts::Options options {"elfheadedit", "Edit elf headers"};
    options.add_options()
        ("m,machine", "Machine in hex", cxxopts::value<std::string>())
        ("f,flags", "Flags in hex", cxxopts::value<std::string>())
        ("i,input", "Input file", cxxopts::value<std::string>())
        ("o,output", "Output file", cxxopts::value<std::string>())
        ("h,help", "Print help")        
        ;

    options.parse(argc, argv);

    if (options.count("help")) {
        std::cerr << options.help({""}) << '\n';
        return -1;
    }

    if (!options.count("input")) {
        std::cerr << "ERROR: Must pass input file" << "\n\n";
        std::cerr << options.help({""}) << '\n';
        return -1;
    }

    auto in_file_name = options["input"].as<std::string>();
    std::ifstream in_file {in_file_name};
    if (!in_file) {
        std::cerr << "Failed to open input file " << in_file_name << ".\n";
        return -1;
    }

    if (!options.count("output")) {
        std::cerr << "ERROR: Must pass output file" << "\n\n";
        std::cerr << options.help({""}) << '\n';
        return -1;
    }

    auto out_file_name = options["output"].as<std::string>();    
    std::ofstream out_file {out_file_name};
    if (!in_file) {
        std::cerr << "Failed to open output file " << out_file_name << ".\n";
        return -1;
    }

    nonstd::optional<std::string> machine{};
    if (options.count("machine")) {
        machine = options["machine"].as<std::string>();
    }

    nonstd::optional<std::string> flags{};
    if (options.count("flags")) {
        flags = options["flags"].as<std::string>();
    }

    
    char ident[EI_NIDENT];
    in_file.read(ident, EI_NIDENT);
    in_file.seekg(0);
    if (ident[4] == ELFCLASS32) {
        edit_header<32>(in_file, out_file, machine, flags);
    }
    else if (ident[4] == ELFCLASS64) {
        edit_header<64>(in_file, out_file, machine, flags);
    }
}
