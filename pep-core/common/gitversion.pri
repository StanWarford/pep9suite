# Ask git for the current commit, and hope that we don't get any error messages.
GIT_SHA = $$system(git --git-dir $$PWD/.git --work-tree $$PWD describe --always)
DEFINES += GIT_SHA=\\\"$$GIT_SHA\\\"
