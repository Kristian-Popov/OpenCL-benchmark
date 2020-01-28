# Code convention

In general project should follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some changes that author of this library seems appropriate.

## Change compared to Google C++ style

### Notes about Abseil Common Libraries

Google code style refers to Abseil Common Libraries in a couple places. It is not used at a moment, so please use alternative solutions if base style refers to macros/functions/functionality from Abseil libraries.

### C++ Version

Currently, code should target C++14, i.e., should not use C++17 or later features. At this moment project is aiming to support Boost 1.65.1 - version supplied with Ubuntu 18.04 (Bionic Beaver). This version of Boost works will with Visual Studio 2015, so it is recommended as well.

### Exceptions

Use C++ exceptions where appropriate. It is the biggest difference comparing to base style. Author of this document believes that good use of exceptions has a lot of benefits, including better support of Boost and C++ standard library, which sometimes rely on exceptions (e.g. std::vector::at()).

## Other notes

[Almost-always-auto initialization style](https://herbsutter.com/2013/08/12/gotw-94-solution-aaa-style-almost-always-auto/) provides some benefits, so please use it when it increases readability.

## Format

CLang format file is included in project root, it should be used to format C++ code automatically. A convenient way is to install CLang Format extension to Visual Studio (change extension settings to rely on format file).
