TEMPLATE = subdirs
SUBDIRS = \
            CommonCore \
            AsmCore \   # relative paths
            CPUCore \
            TermCore \

# Submodule build scripts
CommonCore.file = "common/pep-core-common.pro"

# Main application libraries
AsmCore.file = "asm/pep-core-asm.pro"
AsmCore.depends += CommonCore

CPUCore.file = "cpu/pep-core-cpu.pro"
CPUCore.depends += CommonCore

TermCore.file = "term/pep-core-term.pro"
TermCore.depends += CommonCore CPUCore AsmCore
