PepStyles
=================

Provides two cross-platform stylesheets in Qt with the same look-and-feel for the Pep/9 suite of applications.


Theme generation
================
To modify the colors used in generating the light & dark themes, navigate to template/colorsDark.txt or template/colorsLight.text
After reading through the comments to find the colors to be changed, execute generate_styles.py, and light.qss & dark.qss will automatically be regenerated.

C++ Installation
================

PepStyles is meant to be used a submodule to the Pep/9 suite of applications.

To add and initialize PepStyles as a submodule, run:
```git
git submodule add https://github.com/Matthew-McRaven/PepStyles
git submodule update
```

After downloading, add PepStyles.pepstyles.qrc to the applications resources.
```qmake
RESOURCES = PepStyles/pepstyles.qrc
```

To load the stylesheet in C++, load the file using QFile and read the data. For example, to load the dark style, run:

```cpp

#include <QApplication>
#include <QFile>
#include <QTextStream>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // set stylesheet
    QFile file(":/dark.qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    app.setStyleSheet(stream.readAll());
    return app.exec();
}
```

To remove PepStyles as a submodule, one must do the following:
- Delete the relevant section from the .gitmodules file.
- Stage the .gitmodules changes git add .gitmodules
- Delete the relevant section from .git/config.
- Run git rm --cached path_to_submodule (no trailing slash).
- Run rm -rf .git/modules/path_to_submodule (no trailing slash).
- Commit git commit -m "Removed submodule <name>"
- Delete the now untracked submodule files rm -rf path_to_submodule

License
=======

MIT, see [license](/LICENSE.md).

Acknowledgements
================

PepStyles is a fork [BreezeStyleSheets](https://github.com/Alexhuszagh/BreezeStyleSheets).
BreezeStyleSheets is a fork of [QDarkStyleSheet](https://github.com/ColinDuquesnoy/QDarkStyleSheet).

Contact
=======

Email: matthew.mcraven@pepperdine.edu
