# Extra Notepad++ Languages
Additional programming language support for Notepad++

The initial goal is to add full support for the following languages, including when they are within a file that is primarily another (e.g. HTML in `html.erb`, and then `<script>` and `<style>` in HTML).

   * HTML
   * Javascript
   * CSS
   * SCSS (http://sass-lang.com/)
   * ERB (Embedded Ruby, https://en.wikipedia.org/wiki/ERuby)
   * Haml (Ruby - "Beautiful, DRY, well-indented, clear markup: templating haiku.", http://haml.info/)
   * Slim (Ruby - "A Fast, Lightweight Template Engine for Ruby", http://slim-lang.com/)
   * Markdown

For example, this Haml example, includes Haml, Markdown and Ruby

```haml
%p
  :markdown
    # Greetings

    Hello, *#{user.name}*
```

# Building
To build the plugin with MSVC 2015, you need to additionally clone https://github.com/notepad-plus-plus/notepad-plus-plus.git
into a `notepad-plus-plus` subdirectory.

To then build only the plugin DLL (`npp-languages.dll`) you can just build that project.

To build notepad++ as well (for example to get a debug build) apply `notepad-plus-plus.diff` to fix up some issues with the
Notepad++ project files (as of 2016/10/14 with MSVC 2015 Update 3).
The 3 projects should then build for debug, release and for both x86 and x64, and give you a runnable Notepad++ including
the plugin in the target directory (e.g. `<clone-dir>/x64/Debug/notepad++.exe`).
