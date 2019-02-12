# Script to automatically generate multiple themes from a template
# using a file containing a list of textual subsitutions and comments
import os
def makeAbsolute(relPathToScript):
    script_dir = os.path.dirname(os.path.abspath(__file__))
    return os.path.join(script_dir, relPathToScript)

def loadRepList(repStrFile):
    validSubs = []
    with open(makeAbsolute(repStrFile))  as file:
        tempSubs = [x.strip() for x in file.readlines() if x.strip()]
        for sub in tempSubs:
            # If the first two non-whitespace characters are //, then the line is a comment
            if sub[0:2] == "//":
                continue
            split = sub.split(":=")
            # If there is no or multiple  :=, then it can't be a valid subsitution
            if len(split) != 2:
                raise SyntaxError("Error: invalid subs: "+ sub)
            # If the left of a textual substitution has a comment, then it can't be a valid substitution.
            elif "\\" in split[0]:
                raise SyntaxError("Error: comment cannot appear to left of :=: "+ sub)
            else:
                target = split[0].rstrip()
                # In case multiple = occur in a line, assume the programmer was intentional
                # and assume that they are all part of the substitution.
                replacement = split[1].partition("//")[0].strip()
                # Fuse anything after multiple = back together, remove anything after a comment, remove extra whitespace
                validSubs.append([target, replacement] )
    return validSubs

# Given an output file name and a file containing substitutions, apply the substitutions
def doReplace(fileName, repStrFile):
    with open(makeAbsolute("template.css")) as tempFile:
        replacements = loadRepList(repStrFile)
        textStr = "".join(tempFile.readlines())
        for rep in replacements:
            textStr = textStr.replace(rep[0], rep[1])
        with open(makeAbsolute(fileName), "w") as lFile:
            lFile.write(textStr)

doReplace("../light.qss", "./colorsLight.txt")
doReplace("../dark.qss", "./colorsDark.txt")
