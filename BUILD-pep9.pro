include (gitversion.pri)

TEMPLATE = subdirs
SUBDIRS = \
            PepCommon \
            Pep9DEF_lib \
            Pep9ASM_lib \
            Pep9ASM_app \   # relative paths
            Pep9CPU_lib \
            Pep9CPU_app\
            Pep9Micro_lib \
            Pep9Micro_app\
            Pep9Term_app\

# Submodule build scripts
PepCommon.file = "pep-core/BUILD-common-libs.pro"

Pep9DEF_lib.file = "pep9/pep9def-lib/pep9def-lib.pro"
Pep9DEF_lib.depends += PepCommon

Pep9ASM_lib.file = "pep9/pep9asm-lib/pep9asm-lib.pro"
Pep9ASM_lib.depends += PepCommon Pep9DEF_lib

Pep9CPU_lib.file = "pep9/pep9cpu-lib/pep9cpu-lib.pro"
Pep9CPU_lib.depends += PepCommon Pep9DEF_lib

Pep9Micro_lib.file = "pep9/pep9micro-lib/pep9micro-lib.pro"
Pep9Micro_lib.depends += PepCommon Pep9CPU_lib Pep9ASM_lib Pep9DEF_lib
# Main application libraries
Pep9ASM_app.file = "pep9/pep9asm-app/pep9asm-app.pro"
Pep9ASM_app.depends += PepCommon Pep9ASM_lib Pep9DEF_lib

Pep9CPU_app.file = "pep9/pep9cpu-app/pep9cpu-app.pro"
Pep9CPU_app.depends += PepCommon Pep9CPU_lib Pep9DEF_lib

Pep9Micro_app.file = "pep9/pep9micro-app/pep9micro-app.pro"
Pep9Micro_app.depends += PepCommon Pep9Micro_lib Pep9DEF_lib

Pep9Term_app.file = "pep9/pep9term-app/pep9term-app.pro"
Pep9Term_app.depends += PepCommon Pep9CPU_lib Pep9ASM_lib Pep9Micro_lib Pep9DEF_lib
